#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include "estoqueproxymodel.h"
#include "importarxml.h"
#include "singleeditdelegate.h"
#include "ui_importarxml.h"
#include "xml.h"

ImportarXML::ImportarXML(const QString &fornecedor, QWidget *parent) : QDialog(parent), ui(new Ui::ImportarXML) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables(fornecedor);

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();
}

ImportarXML::~ImportarXML() { delete ui; }

void ImportarXML::setupTables(const QString &fornecedor) {
  modelEstoque.setTable("estoque");
  modelEstoque.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelEstoque.setFilter("status = 'TEMP'");

  if (not modelEstoque.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + modelEstoque.lastError().text());
  }

  //  mapper.setModel(&modelEstoque);
  //  mapper.setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

  ui->tableEstoque->setModel(new EstoqueProxyModel(&modelEstoque, "quantUpd", this));
  ui->tableEstoque->setItemDelegate(new SingleEditDelegate(modelEstoque.fieldIndex("codComercial"), this));
  ui->tableEstoque->hideColumn("quantUpd");

  modelCompra.setTable("pedido_fornecedor_has_produto");
  modelCompra.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelCompra.setFilter("fornecedor = '" + fornecedor + "' AND status = 'EM FATURAMENTO'");

  if (not modelCompra.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + modelCompra.lastError().text());
  }

  ui->tableCompra->setModel(new EstoqueProxyModel(&modelCompra, "quantUpd", this));
  ui->tableCompra->hideColumn("selecionado");
  ui->tableCompra->hideColumn("quantUpd");

  ui->tableCompra->resizeColumnsToContents();
}

QString ImportarXML::getIdCompra() { return m_idCompra; }

void ImportarXML::on_pushButtonImportar_clicked() {
  //--------------
  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    modelEstoque.setData(row, "status", modelEstoque.data(row, "quant").toDouble() < 0 ? "PRÉ-CONSUMO" : "ESTOQUE");
  }
  //--------------

  if (not modelEstoque.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados da tabela estoque: " + modelEstoque.lastError().text());
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  bool ok = true;

  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    ok = modelEstoque.data(row, "quantUpd") != White ? false : true;
  }

  //  for (int row = 0; row < modelCompra.rowCount(); ++row) {
  //    ok = modelCompra.data(row, "quantUpd") == Green;
  //  }

  if (not ok) {
    QMessageBox::critical(this, "Erro!", "Nem todos os produtos estão ok!");
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

  int idNFe = lerXML(file);
  //  idNFe = lerXML(file);
  if (idNFe == 0) return;

  //--------------------
  // TODO: local nao deve ser reinserido, substituindo os antigos (cada um tem o seu!)
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
  //--------------------

  // generate ids for estoque
  if (not modelEstoque.submitAll()) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  //--------------
  limparAssociacoes();
  //--------------

  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    if (modelEstoque.data(row, "quantUpd") == Green) continue;

    //---------
    if (modelEstoque.data(row, "local").toString() == "TEMP") modelEstoque.setData(row, "local", local);
    //---------

    const auto list = modelCompra.match(modelCompra.index(0, modelCompra.fieldIndex("codComercial")), Qt::DisplayRole,
                                        modelEstoque.data(row, "codComercial"), -1);

    if (list.size() == 0) {
      if (not modelEstoque.setData(row, "quantUpd", Red)) {
        QSqlQuery("ROLLBACK").exec();
        return;
      }

      continue;
    }

    double estoqueConsumido = 0;

    for (auto item : list) {
      associarItens(item, row, idNFe, estoqueConsumido);
    }
  }

  m_idCompra = m_idCompra.replace(",", " OR idCompra = ");

  if (not modelEstoque.submitAll() and not modelCompra.submitAll()) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  setarIdCompraNFe(idNFe);

  ui->tableEstoque->resizeColumnsToContents();
}

void ImportarXML::associarItens(QModelIndex &item, int row, int idNFe, double &estoqueConsumido) {
  if (modelEstoque.data(row, "quantUpd") == Green) return;
  if (modelCompra.data(item.row(), "quantUpd") == Green) return;

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
  if (not m_idCompra.contains(idCompra)) m_idCompra += m_idCompra.isEmpty() ? idCompra : "," + idCompra;

  QString idCompraEstoque = modelEstoque.data(row, "idCompra").toString().remove("0");
  if (not idCompraEstoque.contains(idCompra)) idCompraEstoque += idCompraEstoque.isEmpty() ? idCompra : "," + idCompra;
  modelEstoque.setData(row, "idCompra", idCompraEstoque);
  if (modelEstoque.data(row, "idNFe").isNull()) modelEstoque.setData(row, "idNFe", idNFe);

  QString idEstoqueBaixo = modelCompra.data(item.row(), "idEstoque").toString();
  QString idEstoqueCima = modelEstoque.data(row, "idEstoque").toString();
  QString idNFeBaixo = modelCompra.data(item.row(), "idNFe").toString();
  QString idNFeCima = modelEstoque.data(row, "idNFe").toString();

  modelCompra.setData(item.row(), "idEstoque",
                      idEstoqueBaixo.isEmpty() ? idEstoqueCima : idEstoqueBaixo + "," + idEstoqueCima);
  modelCompra.setData(item.row(), "idNFe", idNFeBaixo.isEmpty() ? idNFeCima : idNFeBaixo + "," + idNFeCima);

  // ----------------------------------
  if (modelCompra.data(item.row(), "quantUpd") == Green) {
    criarConsumo(item, modelCompra.data(item.row(), "codComercial").toString(),
                 modelCompra.data(item.row(), "codBarras").toString(), idCompra, row);
  }
}

void ImportarXML::setarIdCompraNFe(const int &idNFe) {
  auto list = modelEstoque.match(modelEstoque.index(0, modelEstoque.fieldIndex("idNFe")), Qt::DisplayRole, idNFe, -1);

  for (auto item : list) {
    QSqlQuery query;
    query.prepare("SELECT * FROM nfe WHERE idNFe = :idNFe");
    query.bindValue(":idNFe", idNFe);

    if (not query.exec() or not query.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando dados da nfe: " + query.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }

    QString idCompra1 = query.value("idCompra").toString().remove("PLACEHOLDER");
    QString idCompra2 = modelEstoque.data(item.row(), "idCompra").toString();
    QString idCompra3 = (idCompra1.isEmpty() or idCompra1 == idCompra2) ? idCompra2 : idCompra1 + "," + idCompra2;
    idCompra3 = idCompra1.contains(idCompra2) ? idCompra1 : idCompra3;

    //    qDebug() << "idCompra1: " << idCompra1;
    //    qDebug() << "idCompra2: " << idCompra2;
    //    qDebug() << "idCompra3: " << idCompra3;

    query.prepare("UPDATE nfe SET idCompra = :idCompra WHERE idNFe = :idNFe");
    query.bindValue(":idCompra", idCompra3);
    query.bindValue(":idNFe", idNFe);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando idCompra em nfe: " + query.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }
  }
}

int ImportarXML::lerXML(QFile &file) {
  XML xml(file.readAll(), file.fileName());

  int idNFe = xml.cadastrarNFe("ENTRADA");

  if (idNFe == 0) return 0;

  xml.mostrarNoSqlModel(modelEstoque);

  return idNFe;
}

void ImportarXML::criarConsumo(QModelIndex &item, QString codComercial, QString codBarras, QString idCompra, int row) {
  QSqlQuery query;
  query.prepare("SELECT * FROM venda_has_produto AS v LEFT JOIN produto AS p ON v.idProduto = p.idProduto WHERE "
                "(p.codComercial = :codComercial OR p.codBarras = :codBarras) AND idCompra = :idCompra AND status = "
                "'EM FATURAMENTO'");
  query.bindValue(":codComercial", codComercial);
  query.bindValue(":codBarras", codBarras);
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando em venda_has_produto: " + modelEstoque.lastError().text());
    return;
  }

  while (query.next()) {
    if (query.value("quant") > modelCompra.data(item.row(), "quant")) continue;

    int newRow = modelEstoque.rowCount();

    if (not modelEstoque.insertRow(newRow)) {
      QMessageBox::critical(this, "Erro!", "Erro criando nova linha na tabela: " + modelEstoque.lastError().text());
      return;
    }

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

void ImportarXML::closeEvent(QCloseEvent *event) {
  QSqlQuery("ROLLBACK").exec();

  QDialog::closeEvent(event);
}

void ImportarXML::on_pushButtonCancelar_clicked() { close(); }

void ImportarXML::on_pushButtonReparear_clicked() {
  //--------------
  limparAssociacoes();
  //--------------

  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    const auto list = modelCompra.match(modelCompra.index(0, modelCompra.fieldIndex("codComercial")), Qt::DisplayRole,
                                        modelEstoque.data(row, "codComercial"), -1);

    if (list.size() == 0) {
      if (not modelEstoque.setData(row, "quantUpd", Red)) {
        QSqlQuery("ROLLBACK").exec();
        return;
      }

      continue;
    }

    double estoqueConsumido = 0;

    for (auto item : list) {
      associarItens(item, row, idNFe, estoqueConsumido);
    }
  }

  ui->tableEstoque->clearSelection();
  ui->tableCompra->clearSelection();
}

// TODO: quando for consumir verificar se existe quantidades iguais para associar, senao associar normal
// TODO: no final mudar status para diferente de temp
// TODO: erro ao importar 4 notas de white plain
// TODO: adicionar caixas
// TODO: no caso dos white plain com varias notas, e se somasse os estoques na tabela de cima em vez de ter 3 linhas
// iguais??
