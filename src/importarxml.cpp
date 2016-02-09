#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include "checkboxdelegate.h"
#include "estoqueproxymodel.h"
#include "importarxml.h"
#include "singleeditdelegate.h"
#include "ui_importarxml.h"
#include "xml.h"

ImportarXML::ImportarXML(const QString &idCompra, QWidget *parent) : QDialog(parent), ui(new Ui::ImportarXML) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables(idCompra);

  connect(ui->tableCompra->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &ImportarXML::openPersistente);

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();
}

ImportarXML::~ImportarXML() { delete ui; }

void ImportarXML::setupTables(const QString &idCompra) {
  modelEstoque.setTable("estoque");
  modelEstoque.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelEstoque.setFilter("status = 'TEMP'");

  if (not modelEstoque.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + modelEstoque.lastError().text());
  }

  ui->tableEstoque->setModel(new EstoqueProxyModel(&modelEstoque, this));
  ui->tableEstoque->setItemDelegate(new SingleEditDelegate(modelEstoque.fieldIndex("codComercial"), this));
  ui->tableEstoque->hideColumn("quantUpd");

  modelCompra.setTable("pedido_fornecedor_has_produto");
  modelCompra.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelCompra.setHeaderData("selecionado", "");

  modelCompra.setFilter("idCompra = " + idCompra);

  if (not modelCompra.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + modelCompra.lastError().text());
  }

  ui->tableCompra->setModel(new EstoqueProxyModel(&modelCompra, this));
  ui->tableCompra->setItemDelegateForColumn("selecionado", new CheckBoxDelegate(this));
  ui->tableCompra->hideColumn("quantUpd");

  ui->tableCompra->resizeColumnsToContents();

  openPersistente();

  for (int row = 0; row < modelCompra.rowCount(); ++row) {
    modelCompra.setData(row, "selecionado", true);
  }
}

void ImportarXML::on_pushButtonImportar_clicked() {
  //--------------
  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    modelEstoque.setData(row, "status", modelEstoque.data(row, "quant").toDouble() < 0 ? "PRÉ-CONSUMO" : "ESTOQUE");
  }
  //--------------

  bool ok = true;

  for (int row = 0; row < modelCompra.rowCount(); ++row) {
    if (modelCompra.data(row, "quantUpd") != Green) {
      ok = false;
      break;
    }
  }

  if (not ok) {
    QMessageBox::critical(this, "Erro!", "Nenhum produto de compra foi pareado!");
    return;
  }

  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    int color = modelEstoque.data(row, "quantUpd").toInt();

    if (color != Green and color != DarkGreen) {
      ok = false;
      break;
    }
  }

  if (not ok) {
    QMessageBox::critical(this, "Erro!", "Nem todos os estoques estão ok!");
    return;
  }

  if (not modelEstoque.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados da tabela estoque: " + modelEstoque.lastError().text());
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  QDialog::accept();
  close();
}

void ImportarXML::limparAssociacoes() {
  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    modelEstoque.setData(row, "quantUpd", 0);

    if (modelEstoque.data(row, "quant").toDouble() < 0) modelEstoque.removeRow(row);
  }

  for (int row = 0; row < modelCompra.rowCount(); ++row) {
    modelCompra.setData(row, "quantUpd", 0);
    modelCompra.setData(row, "quantConsumida", 0);
  }
}

void ImportarXML::on_pushButtonProcurar_clicked() {
  const QString filePath = QFileDialog::getOpenFileName(this, "Arquivo XML", QDir::currentPath(), "XML (*.xml)");

  if (filePath.isEmpty()) {
    ui->lineEdit->setText(QString());
    return;
  }

  QFile file(filePath);

  if (not file.open(QFile::ReadOnly)) {
    QMessageBox::critical(this, "Erro!", "Erro lendo arquivo: " + file.errorString());
    return;
  }

  if (not lerXML(file)) return;

  //--------------------
  QSqlQuery query;
  if (not query.exec("SELECT descricao FROM loja WHERE descricao > '' AND descricao != 'CD'")) {
    QMessageBox::critical(this, "Erro!", "Erro buscando lojas: " + query.lastError().text());
    return;
  }

  QStringList lojas;
  lojas << "CD";

  while (query.next()) {
    lojas << query.value("descricao").toString();
  }

  QString local = QInputDialog::getItem(this, "Local", "Local do depósito:", lojas, 0, false);

  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    if (modelEstoque.data(row, "local").toString() == "TEMP") modelEstoque.setData(row, "local", local);
  }
  //--------------------

  // generate ids for estoque
  if (not modelEstoque.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro gerando id estoque: " + modelEstoque.lastError().text());
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  parear();

  if (not modelEstoque.submitAll() and not modelCompra.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando estoque e compra: " + modelEstoque.lastError().text());
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  bool ok = true;

  for (int row = 0; row < modelCompra.rowCount(); ++row) {
    if (modelCompra.data(row, "quantUpd") != Green) {
      ok = false;
      break;
    }
  }

  if (ok) {
    ui->pushButtonProcurar->setDisabled(true);
  }

  ui->tableEstoque->resizeColumnsToContents();
  ui->tableCompra->resizeColumnsToContents();
}

void ImportarXML::associarItens(QModelIndex &item, int row, double &estoqueConsumido) {
  if (modelEstoque.data(row, "quantUpd") == Green) return;
  if (modelCompra.data(item.row(), "quantUpd") == Green) return;
  if (modelCompra.data(item.row(), "selecionado") == false) return;

  //-------------------------------
  if (modelEstoque.data(row, "quant").toDouble() < 0) return;
  //-------------------------------

  double quantEstoque = modelEstoque.data(row, "quant").toDouble();
  double quantCompra = modelCompra.data(item.row(), "quant").toDouble();
  double quantConsumida = modelCompra.data(item.row(), "quantConsumida").toDouble();

  double espaco = quantCompra - quantConsumida;
  double estoqueDisponivel = quantEstoque - estoqueConsumido;
  double quantAdicionar = estoqueDisponivel > espaco ? espaco : estoqueDisponivel;
  estoqueConsumido += quantAdicionar;

  modelCompra.setData(item.row(), "quantConsumida", quantConsumida + quantAdicionar);

  modelEstoque.setData(row, "quantUpd", qFuzzyCompare(estoqueConsumido, quantEstoque) ? Green : Yellow);
  modelCompra.setData(item.row(), "quantUpd",
                      qFuzzyCompare((quantConsumida + quantAdicionar), quantCompra) ? Green : Yellow);

  QString idCompra = modelCompra.data(item.row(), "idCompra").toString();
  QString idCompraEstoque = modelEstoque.data(row, "idCompra").toString().remove("0");
  if (not idCompraEstoque.contains(idCompra)) idCompraEstoque += idCompraEstoque.isEmpty() ? idCompra : "," + idCompra;
  modelEstoque.setData(row, "idCompra", idCompraEstoque);

  QString idEstoqueBaixo = modelCompra.data(item.row(), "idEstoque").toString();
  QString idEstoqueCima = modelEstoque.data(row, "idEstoque").toString();
  if (not idEstoqueBaixo.contains(idEstoqueCima)) {
    idEstoqueBaixo = idEstoqueBaixo.isEmpty() ? idEstoqueCima : idEstoqueBaixo + "," + idEstoqueCima;
  }
  modelCompra.setData(item.row(), "idEstoque", idEstoqueBaixo);

  QStringList idsEstoque = idEstoqueBaixo.split(",");

  QString idNFeBaixo;

  for (auto id : idsEstoque) {
    auto list =
        modelEstoque.match(modelEstoque.index(0, modelEstoque.fieldIndex("idEstoque")), Qt::DisplayRole, id, -1);

    for (auto estoque : list) {
      QString idNFeCima = modelEstoque.data(estoque.row(), "idNFe").toString();
      idNFeBaixo += idNFeBaixo.isEmpty() ? idNFeCima : "," + idNFeCima;
    }

    modelCompra.setData(item.row(), "idNFe", idNFeBaixo);
  }

  //---------------------------- setar idCompra nas nfe's

  QString tempidCompra = modelEstoque.data(row, "idCompra").toString();
  QStringList tempidsNfe = modelEstoque.data(row, "idNFe").toString().split(",");

  for (auto tempnfe : tempidsNfe) {
    QSqlQuery query;
    query.prepare("UPDATE nfe SET idCompra = '" + tempidCompra + "' WHERE idNFe = :idNFe");
    query.bindValue(":idNFe", tempnfe);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando idCompra em NFe: " + query.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }
  }
}

bool ImportarXML::lerXML(QFile &file) {
  XML xml(file.readAll(), file.fileName());
  if (not xml.cadastrarNFe("ENTRADA")) return false;
  xml.mostrarNoSqlModel(modelEstoque);

  return true;
}

void ImportarXML::criarConsumo() {
  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    QString codComercial = modelEstoque.data(row, "codComercial").toString();
    QString idCompra = modelEstoque.data(row, "idCompra").toString().replace(",", " OR idCompra = ");

    QSqlQuery query;
    if (not query.exec("SELECT quant, idVendaProduto FROM venda_has_produto AS v LEFT JOIN produto AS p ON v.idProduto "
                       "= p.idProduto WHERE p.codComercial = '" +
                       codComercial + "' AND idCompra = " + idCompra + " AND status = 'EM FATURAMENTO'")) {
      QMessageBox::critical(this, "Erro!", "Erro buscando em venda_has_produto: " + modelEstoque.lastError().text());
      return;
    }

    while (query.next()) {
      if (query.value("quant") > modelEstoque.data(row, "quant")) continue;

      int newRow = modelEstoque.rowCount();

      modelEstoque.insertRow(newRow);

      for (int column = 0; column < modelEstoque.columnCount(); ++column) {
        if (not modelEstoque.setData(newRow, column, modelEstoque.data(row, column))) return;
      }

      double quant = query.value("quant").toDouble() * -1;

      if (not modelEstoque.setData(newRow, "quant", quant)) return;
      if (not modelEstoque.setData(newRow, "quantUpd", DarkGreen)) return;
      if (not modelEstoque.setData(newRow, "idVendaProduto", query.value("idVendaProduto"))) return;
      if (not modelEstoque.setData(newRow, "idEstoqueConsumido", modelEstoque.data(row, "idEstoque"))) return;
    }
  }
}

void ImportarXML::closeEvent(QCloseEvent *event) {
  QSqlQuery("ROLLBACK").exec();

  QDialog::closeEvent(event);
}

void ImportarXML::on_pushButtonCancelar_clicked() { close(); }

void ImportarXML::on_pushButtonReparear_clicked() {
  parear();

  ui->tableEstoque->clearSelection();
  ui->tableCompra->clearSelection();
}

void ImportarXML::openPersistente() {
  for (int row = 0, rowCount = ui->tableCompra->model()->rowCount(); row < rowCount; ++row) {
    ui->tableCompra->openPersistentEditor(row, "selecionado");
  }
}

void ImportarXML::parear() {
  limparAssociacoes();

  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    const auto list = modelCompra.match(modelCompra.index(0, modelCompra.fieldIndex("codComercial")), Qt::DisplayRole,
                                        modelEstoque.data(row, "codComercial"), -1,
                                        Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap));

    if (list.size() == 0) {
      if (not modelEstoque.setData(row, "quantUpd", Red)) {
        QSqlQuery("ROLLBACK").exec();
        return;
      }

      continue;
    }

    double estoqueConsumido = 0;

    //------------------------ procurar quantidades iguais
    for (auto item : list) {
      double quantEstoque = modelEstoque.data(row, "quant").toDouble();
      double quantCompra = modelCompra.data(item.row(), "quant").toDouble();

      if (quantEstoque == quantCompra) associarItens(item, row, estoqueConsumido);
    }
    //------------------------

    for (auto item : list) {
      associarItens(item, row, estoqueConsumido);
    }
  }

  //---------------------------
  bool ok = true;

  for (int row = 0; row < modelCompra.rowCount(); ++row) {
    if (modelCompra.data(row, "quantUpd") != Green) {
      ok = false;
      break;
    }
  }

  if (ok) {
    criarConsumo();
  }

  //---------------------------

  modelEstoque.submitAll();
}

// TODO: corrigir consumo de estoque para consumir parcialmente de forma correta
// NOTE: se filtro por compra, é necessário a coluna 'selecionado' para escolher qual linha parear?
// NOTE: utilizar tabela em arvore (qtreeview) para agrupar consumos com seu estoque (para cada linha do model inserir items na arvore?)
