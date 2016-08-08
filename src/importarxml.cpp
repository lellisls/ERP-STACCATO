#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include "estoqueproxymodel.h"
#include "importarxml.h"
#include "noeditdelegate.h"
#include "singleeditdelegate.h"
#include "ui_importarxml.h"
#include "xml.h"

ImportarXML::ImportarXML(const QStringList &idsCompra, const QString &dataReal, QWidget *parent)
    : QDialog(parent), ui(new Ui::ImportarXML), dataReal(dataReal), idsCompra(idsCompra) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables(idsCompra);
}

ImportarXML::~ImportarXML() { delete ui; }

void ImportarXML::setupTables(const QStringList &idsCompra) {
  modelEstoque.setTable("estoque");
  modelEstoque.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelEstoque.setFilter("status = 'TEMP'");

  modelEstoque.setHeaderData("status", "Status");
  modelEstoque.setHeaderData("ordemCompra", "OC");
  modelEstoque.setHeaderData("numeroNFe", "NFe");
  modelEstoque.setHeaderData("local", "Local");
  modelEstoque.setHeaderData("fornecedor", "Fornecedor");
  modelEstoque.setHeaderData("descricao", "Produto");
  modelEstoque.setHeaderData("quant", "Quant.");
  modelEstoque.setHeaderData("un", "Un.");
  modelEstoque.setHeaderData("caixas", "Cx.");
  modelEstoque.setHeaderData("codBarras", "Cód. Bar.");
  modelEstoque.setHeaderData("codComercial", "Cód. Com.");
  modelEstoque.setHeaderData("valorUnid", "R$ Unid.");
  modelEstoque.setHeaderData("valor", "R$");

  if (not modelEstoque.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + modelEstoque.lastError().text());
  }

  ui->tableEstoque->setModel(new EstoqueProxyModel(&modelEstoque, this));
  ui->tableEstoque->setItemDelegate(new NoEditDelegate(this));
  ui->tableEstoque->setItemDelegateForColumn("codComercial", new SingleEditDelegate(this));
  ui->tableEstoque->setItemDelegateForColumn("quant", new SingleEditDelegate(this));
  ui->tableEstoque->setItemDelegateForColumn("un", new SingleEditDelegate(this));
  ui->tableEstoque->setItemDelegateForColumn("descricao", new SingleEditDelegate(this));
  ui->tableEstoque->hideColumn("quantUpd");
  ui->tableEstoque->hideColumn("idNFe");
  ui->tableEstoque->hideColumn("idEstoque");
  ui->tableEstoque->hideColumn("idCompra");
  ui->tableEstoque->hideColumn("idProduto");
  ui->tableEstoque->hideColumn("ncm");
  ui->tableEstoque->hideColumn("cfop");
  ui->tableEstoque->hideColumn("codBarrasTrib");
  ui->tableEstoque->hideColumn("unTrib");
  ui->tableEstoque->hideColumn("quantTrib");
  ui->tableEstoque->hideColumn("valorTrib");
  ui->tableEstoque->hideColumn("desconto");
  ui->tableEstoque->hideColumn("compoeTotal");
  ui->tableEstoque->hideColumn("numeroPedido");
  ui->tableEstoque->hideColumn("itemPedido");
  ui->tableEstoque->hideColumn("tipoICMS");
  ui->tableEstoque->hideColumn("orig");
  ui->tableEstoque->hideColumn("cstICMS");
  ui->tableEstoque->hideColumn("modBC");
  ui->tableEstoque->hideColumn("vBC");
  ui->tableEstoque->hideColumn("pICMS");
  ui->tableEstoque->hideColumn("vICMS");
  ui->tableEstoque->hideColumn("modBCST");
  ui->tableEstoque->hideColumn("pMVAST");
  ui->tableEstoque->hideColumn("vBCST");
  ui->tableEstoque->hideColumn("pICMSST");
  ui->tableEstoque->hideColumn("vICMSST");
  ui->tableEstoque->hideColumn("cEnq");
  ui->tableEstoque->hideColumn("cstIPI");
  ui->tableEstoque->hideColumn("cstPIS");
  ui->tableEstoque->hideColumn("vBCPIS");
  ui->tableEstoque->hideColumn("pPIS");
  ui->tableEstoque->hideColumn("vPIS");
  ui->tableEstoque->hideColumn("cstCOFINS");
  ui->tableEstoque->hideColumn("vBCCOFINS");
  ui->tableEstoque->hideColumn("pCOFINS");
  ui->tableEstoque->hideColumn("vCOFINS");

  //

  modelConsumo.setTable("estoque_has_consumo");
  modelConsumo.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelConsumo.setFilter("status = 'TEMP'");

  modelConsumo.setHeaderData("status", "Status");
  modelConsumo.setHeaderData("ordemCompra", "OC");
  modelConsumo.setHeaderData("numeroNFe", "NFe");
  modelConsumo.setHeaderData("local", "Local");
  modelConsumo.setHeaderData("fornecedor", "Fornecedor");
  modelConsumo.setHeaderData("descricao", "Produto");
  modelConsumo.setHeaderData("quant", "Quant.");
  modelConsumo.setHeaderData("un", "Un.");
  modelConsumo.setHeaderData("caixas", "Cx.");
  modelConsumo.setHeaderData("codBarras", "Cód. Bar.");
  modelConsumo.setHeaderData("codComercial", "Cód. Com.");
  modelConsumo.setHeaderData("valorUnid", "R$ Unid.");
  modelConsumo.setHeaderData("valor", "R$");

  if (not modelConsumo.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque_has_consumo: " + modelConsumo.lastError().text());
  }

  ui->tableConsumo->setModel(new EstoqueProxyModel(&modelConsumo, this));
  ui->tableConsumo->hideColumn("idConsumo");
  ui->tableConsumo->hideColumn("quantUpd");
  ui->tableConsumo->hideColumn("idNFe");
  ui->tableConsumo->hideColumn("idEstoque");
  ui->tableConsumo->hideColumn("idCompra");
  ui->tableConsumo->hideColumn("idVendaProduto");
  ui->tableConsumo->hideColumn("idProduto");
  ui->tableConsumo->hideColumn("ncm");
  ui->tableConsumo->hideColumn("cfop");
  ui->tableConsumo->hideColumn("codBarrasTrib");
  ui->tableConsumo->hideColumn("unTrib");
  ui->tableConsumo->hideColumn("quantTrib");
  ui->tableConsumo->hideColumn("valorTrib");
  ui->tableConsumo->hideColumn("desconto");
  ui->tableConsumo->hideColumn("compoeTotal");
  ui->tableConsumo->hideColumn("numeroPedido");
  ui->tableConsumo->hideColumn("itemPedido");
  ui->tableConsumo->hideColumn("tipoICMS");
  ui->tableConsumo->hideColumn("orig");
  ui->tableConsumo->hideColumn("cstICMS");
  ui->tableConsumo->hideColumn("modBC");
  ui->tableConsumo->hideColumn("vBC");
  ui->tableConsumo->hideColumn("pICMS");
  ui->tableConsumo->hideColumn("vICMS");
  ui->tableConsumo->hideColumn("modBCST");
  ui->tableConsumo->hideColumn("pMVAST");
  ui->tableConsumo->hideColumn("vBCST");
  ui->tableConsumo->hideColumn("pICMSST");
  ui->tableConsumo->hideColumn("vICMSST");
  ui->tableConsumo->hideColumn("cEnq");
  ui->tableConsumo->hideColumn("cstIPI");
  ui->tableConsumo->hideColumn("cstPIS");
  ui->tableConsumo->hideColumn("vBCPIS");
  ui->tableConsumo->hideColumn("pPIS");
  ui->tableConsumo->hideColumn("vPIS");
  ui->tableConsumo->hideColumn("cstCOFINS");
  ui->tableConsumo->hideColumn("vBCCOFINS");
  ui->tableConsumo->hideColumn("pCOFINS");
  ui->tableConsumo->hideColumn("vCOFINS");

  //

  modelCompra.setTable("pedido_fornecedor_has_produto");
  modelCompra.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelCompra.setFilter("idCompra = " + idsCompra.join(" OR idCompra = "));

  modelCompra.setHeaderData("status", "Status");
  modelCompra.setHeaderData("ordemCompra", "OC");
  modelCompra.setHeaderData("idVenda", "Venda");
  modelCompra.setHeaderData("fornecedor", "Fornecedor");
  modelCompra.setHeaderData("descricao", "Produto");
  modelCompra.setHeaderData("colecao", "Coleção");
  modelCompra.setHeaderData("quant", "Quant.");
  modelCompra.setHeaderData("quantConsumida", "Consumido");
  modelCompra.setHeaderData("un", "Un.");
  modelCompra.setHeaderData("un2", "Un. 2");
  modelCompra.setHeaderData("caixas", "Cx.");
  modelCompra.setHeaderData("prcUnitario", "R$ Unid.");
  modelCompra.setHeaderData("preco", "R$");
  modelCompra.setHeaderData("kgcx", "Kg./Cx.");
  modelCompra.setHeaderData("formComercial", "Form. Com.");
  modelCompra.setHeaderData("codComercial", "Cód. Com.");
  modelCompra.setHeaderData("codBarras", "Cód. Bar.");
  modelCompra.setHeaderData("obs", "Obs.");

  if (not modelCompra.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + modelCompra.lastError().text());
  }

  ui->tableCompra->setModel(new EstoqueProxyModel(&modelCompra, this));
  ui->tableCompra->setItemDelegate(new NoEditDelegate(this));
  ui->tableCompra->setItemDelegateForColumn("codComercial", new SingleEditDelegate(this));
  ui->tableCompra->setItemDelegateForColumn("descricao", new SingleEditDelegate(this));
  ui->tableCompra->hideColumn("selecionado");
  ui->tableCompra->hideColumn("quantUpd");
  ui->tableCompra->hideColumn("idCompra");
  ui->tableCompra->hideColumn("idNFe");
  ui->tableCompra->hideColumn("idEstoque");
  ui->tableCompra->hideColumn("idPedido");
  ui->tableCompra->hideColumn("idProduto");
  ui->tableCompra->hideColumn("dataPrevCompra");
  ui->tableCompra->hideColumn("dataRealCompra");
  ui->tableCompra->hideColumn("dataPrevConf");
  ui->tableCompra->hideColumn("dataRealConf");
  ui->tableCompra->hideColumn("dataPrevFat");
  ui->tableCompra->hideColumn("dataRealFat");
  ui->tableCompra->hideColumn("dataPrevColeta");
  ui->tableCompra->hideColumn("dataRealColeta");
  ui->tableCompra->hideColumn("dataPrevReceb");
  ui->tableCompra->hideColumn("dataRealReceb");
  ui->tableCompra->hideColumn("dataPrevEnt");
  ui->tableCompra->hideColumn("dataRealEnt");

  ui->tableCompra->resizeColumnsToContents();
}

void ImportarXML::on_pushButtonImportar_clicked() {
  //--------------
  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    modelEstoque.setData(row, "status", modelEstoque.data(row, "quant").toDouble() < 0 ? "PRÉ-CONSUMO" : "EM COLETA");
  }

  for (int row = 0; row < modelCompra.rowCount(); ++row) {
    modelCompra.setData(row, "selecionado", false);
  }
  //--------------

  bool ok = false;

  for (int row = 0; row < modelCompra.rowCount(); ++row) {
    if (modelCompra.data(row, "quantUpd") == Green) {
      ok = true;
      break;
    }
  }

  if (not ok) {
    QMessageBox::critical(this, "Erro!", "Nenhuma compra pareada!");
    return;
  }

  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    int color = modelEstoque.data(row, "quantUpd").toInt();

    if (color == Red) {
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
    close();
    return;
  }

  if (not modelCompra.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados da tabela compra: " + modelCompra.lastError().text());
    close();
    return;
  }

  // NOTE: todos as linhas em cima devem ser pareadas mas nem todas em baixo precisam ser (como a nota nao pode ser
  // cadastrada duas vezes seus produtos devem ser todos pareados)

  //------------------------------
  QSqlQuery query;
  for (auto idCompra : idsCompra) {
    // TODO: se fornecedor coleta=0 mudar status para EM RECEBIMENTO com prazo ~8 dias

    query.prepare("UPDATE pedido_fornecedor_has_produto SET dataRealFat = :dataRealFat, status = 'EM COLETA' WHERE "
                  "idCompra = :idCompra AND quantUpd = 1");
    query.bindValue(":dataRealFat", dataReal);
    query.bindValue(":idCompra", idCompra);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status da compra: " + query.lastError().text());
      close();
      return;
    }
  }
  //------------------------------

  QDialog::accept();
  close();
}

void ImportarXML::limparAssociacoes() { // TODO: test setData´s
  for (int row = 0; row < modelCompra.rowCount(); ++row) {
    modelCompra.setData(row, "quantUpd", 0);
    modelCompra.setData(row, "quantConsumida", 0);
  }

  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    modelEstoque.setData(row, "quantUpd", 0);
  }

  for (int row = 0; row < modelConsumo.rowCount(); ++row) {
    QSqlQuery query;
    query.prepare("UPDATE venda_has_produto SET status = 'EM FATURAMENTO' WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":idVendaProduto", modelConsumo.data(row, "idVendaProduto"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro limpando status do produto da venda: " + query.lastError().text());
      close();
    }

    modelConsumo.removeRow(row); // TODO: this need submitAll?
  }
}

void ImportarXML::on_pushButtonProcurar_clicked() {
  const QString filePath = QFileDialog::getOpenFileName(this, "Arquivo XML", QDir::currentPath(), "XML (*.xml)");

  if (filePath.isEmpty()) {
    ui->lineEdit->setText(QString());
    return;
  }

  QFile file(filePath);

  //
  if (not file.open(QFile::ReadOnly)) {
    QMessageBox::critical(this, "Erro!", "Erro lendo arquivo: " + file.errorString());
    return;
  }

  if (not lerXML(file)) {
    close();
    return;
  }

  file.close();

  parear();

  if (not modelEstoque.submitAll() and not modelCompra.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando estoque e compra: " + modelEstoque.lastError().text());
    close();
    return;
  }

  bool ok = true;

  for (int row = 0; row < modelCompra.rowCount(); ++row) {
    if (modelCompra.data(row, "quantUpd") != Green) {
      ok = false;
      break;
    }
  }

  if (ok) ui->pushButtonProcurar->setDisabled(true);

  ui->tableEstoque->resizeColumnsToContents();
  ui->tableConsumo->resizeColumnsToContents();
  ui->tableCompra->resizeColumnsToContents();
}

void ImportarXML::associarItens(int rowCompra, int rowEstoque, double &estoqueConsumido) {
  if (modelEstoque.data(rowEstoque, "quantUpd") == Green) return;
  if (modelCompra.data(rowCompra, "quantUpd") == Green) return;

  //-------------------------------
  if (modelEstoque.data(rowEstoque, "quant").toDouble() < 0) return;
  //-------------------------------

  double quantEstoque = modelEstoque.data(rowEstoque, "quant").toDouble();
  double quantCompra = modelCompra.data(rowCompra, "quant").toDouble();
  double quantConsumida = modelCompra.data(rowCompra, "quantConsumida").toDouble();

  double espaco = quantCompra - quantConsumida;
  double estoqueDisponivel = quantEstoque - estoqueConsumido;
  double quantAdicionar = estoqueDisponivel > espaco ? espaco : estoqueDisponivel;
  estoqueConsumido += quantAdicionar;

  modelCompra.setData(rowCompra, "quantConsumida", quantConsumida + quantAdicionar);

  modelEstoque.setData(rowEstoque, "quantUpd", qFuzzyCompare(estoqueConsumido, quantEstoque) ? Green : Yellow);
  modelCompra.setData(rowCompra, "quantUpd",
                      qFuzzyCompare((quantConsumida + quantAdicionar), quantCompra) ? Green : Yellow);

  modelEstoque.setData(rowEstoque, "ordemCompra", modelCompra.data(rowCompra, "ordemCompra"));

  int idCompra = modelCompra.data(rowCompra, "idCompra").toInt();
  int idEstoque = modelEstoque.data(rowEstoque, "idEstoque").toInt();

  QSqlQuery query;
  query.prepare("SELECT idEstoque FROM estoque_has_compra WHERE idEstoque = :idEstoque AND idCompra = :idCompra");
  query.bindValue(":idEstoque", idEstoque);
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando em estoque_has_compra: " + query.lastError().text());
    return;
  }

  if (query.size() == 0) {
    query.prepare("INSERT INTO estoque_has_compra (idEstoque, idCompra) VALUES (:idEstoque, :idCompra)");
    query.bindValue(":idEstoque", idEstoque);
    query.bindValue(":idCompra", idCompra);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando em estoque_has_compra: " + query.lastError().text());
      return;
    }
  }
}

bool ImportarXML::lerXML(QFile &file) {
  XML xml(file.readAll(), file.fileName());
  if (not xml.cadastrarNFe("ENTRADA")) return false;
  if (not xml.mostrarNoSqlModel(modelEstoque)) return false;

  return true;
}

void ImportarXML::criarConsumo() {
  for (int row = 0; row < modelEstoque.rowCount(); ++row) {

    QString codComercial = modelEstoque.data(row, "codComercial").toString();
    int idEstoque = modelEstoque.data(row, "idEstoque").toInt();

    QSqlQuery queryIdCompra;
    queryIdCompra.prepare("SELECT idCompra FROM estoque_has_compra WHERE idEstoque = :idEstoque");
    queryIdCompra.bindValue(":idEstoque", idEstoque);

    if (not queryIdCompra.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando idCompra: " + queryIdCompra.lastError().text());
      return;
    }

    while (queryIdCompra.next()) {
      QSqlQuery queryProduto;
      queryProduto.prepare("SELECT quant, idVendaProduto FROM venda_has_produto WHERE codComercial = :codComercial AND "
                           "idCompra = :idCompra AND status = 'EM FATURAMENTO'");
      queryProduto.bindValue(":codComercial", codComercial);
      queryProduto.bindValue(":idCompra", queryIdCompra.value("idCompra"));

      if (not queryProduto.exec()) {
        QMessageBox::critical(this, "Erro!", "Erro buscando idVendaProduto: " + queryIdCompra.lastError().text());
        return;
      }

      while (queryProduto.next()) {
        auto list = modelEstoque.match(modelEstoque.index(0, modelEstoque.fieldIndex("idEstoque")), Qt::DisplayRole,
                                       idEstoque, -1);

        double quantTemp = 0;

        for (auto item : list) {
          quantTemp += modelEstoque.data(item.row(), "quant").toDouble();
        }

        if (queryProduto.value("quant") > quantTemp) continue;

        int newRow = modelConsumo.rowCount();
        modelConsumo.insertRow(newRow);

        for (int column = 0; column < modelEstoque.columnCount(); ++column) {
          QString field = modelEstoque.record().fieldName(column);
          int index = modelConsumo.fieldIndex(field);
          QVariant value = modelEstoque.data(row, column);

          if (index != -1) modelConsumo.setData(newRow, index, value);
        }

        const double quant = queryProduto.value("quant").toDouble() * -1;

        if (not modelConsumo.setData(newRow, "quant", quant)) return;
        // TODO: calcular quantidade correta de caixas
        // if (not modelConsumo.setData(newRow, "caixas", )) return;
        if (not modelConsumo.setData(newRow, "quantUpd", DarkGreen)) return;
        if (not modelConsumo.setData(newRow, "idVendaProduto", queryProduto.value("idVendaProduto"))) return;
        if (not modelConsumo.setData(newRow, "idEstoque", modelEstoque.data(row, "idEstoque"))) return;

        QSqlQuery query;
        query.prepare("UPDATE venda_has_produto SET dataRealFat = :dataRealFat, status = 'EM COLETA' WHERE "
                      "idVendaProduto = :idVendaProduto");
        query.bindValue(":dataRealFat", dataReal);
        query.bindValue(":idVendaProduto", queryProduto.value("idVendaProduto"));

        if (not query.exec()) {
          QMessageBox::critical(this, "Erro!",
                                "Erro atualizando status do produto da venda: " + query.lastError().text());
          close();
          return;
        }
      }
    }
  }
}

void ImportarXML::on_pushButtonCancelar_clicked() { close(); }

void ImportarXML::on_pushButtonReparear_clicked() {
  parear();

  ui->tableEstoque->clearSelection();
  ui->tableCompra->clearSelection();
}

void ImportarXML::parear() {
  limparAssociacoes();

  for (int rowEstoque = 0; rowEstoque < modelEstoque.rowCount(); ++rowEstoque) {
    const auto list = modelCompra.match(modelCompra.index(0, modelCompra.fieldIndex("codComercial")), Qt::DisplayRole,
                                        modelEstoque.data(rowEstoque, "codComercial"), -1,
                                        Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap));

    if (list.size() == 0) {
      if (not modelEstoque.setData(rowEstoque, "quantUpd", Red)) return;

      continue;
    }

    QSqlQuery query;
    query.prepare("SELECT idProduto FROM produto WHERE codComercial = :codComercial");
    query.bindValue(":codComercial", modelEstoque.data(rowEstoque, "codComercial"));

    if (not query.exec()) {
      QMessageBox::critical(0, "Erro!", "Erro lendo tabela produto: " + query.lastError().text());
      return;
    }

    if (not query.first()) {
      QMessageBox::critical(0, "Erro!", "Não encontrou o produto!");
      return;
    }

    modelEstoque.setData(rowEstoque, "idProduto", query.value("idProduto"));

    double estoqueConsumido = 0;

    //------------------------ procurar quantidades iguais
    for (auto item : list) {
      double quantEstoque = modelEstoque.data(rowEstoque, "quant").toDouble();
      double quantCompra = modelCompra.data(item.row(), "quant").toDouble();

      if (quantEstoque == quantCompra) associarItens(item.row(), rowEstoque, estoqueConsumido);
    }
    //------------------------

    for (auto item : list) {
      associarItens(item.row(), rowEstoque, estoqueConsumido);
    }
  }

  //---------------------------
  criarConsumo();
  //---------------------------

  if (not modelEstoque.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando estoque: " + modelEstoque.lastError().text());
    return;
  }

  ui->tableEstoque->clearSelection();
  ui->tableCompra->clearSelection();
}

void ImportarXML::on_tableEstoque_entered(const QModelIndex &) { ui->tableEstoque->resizeColumnsToContents(); }

void ImportarXML::on_tableCompra_entered(const QModelIndex &) { ui->tableCompra->resizeColumnsToContents(); }

void ImportarXML::on_pushButtonRemover_clicked() {
  auto list = ui->tableEstoque->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhuma linha selecionada!");
    return;
  }

  for (auto item : list) {
    QSqlQuery query;

    query.prepare("DELETE FROM estoque_has_nfe WHERE idEstoque = :idEstoque");
    query.bindValue(":idEstoque", modelEstoque.data(item.row(), "idEstoque"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro removendo de estoque_has_nfe: " + query.lastError().text());
      return;
    }

    query.prepare("DELETE FROM estoque_has_compra WHERE idEstoque = :idEstoque");
    query.bindValue(":idEstoque", modelEstoque.data(item.row(), "idEstoque"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro removendo de estoque_has_compra: " + query.lastError().text());
      return;
    }

    modelEstoque.removeRow(item.row());
  }

  if (not modelEstoque.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro removendo linhas: " + modelEstoque.lastError().text());
    return;
  }

  // TODO: descadastrar nota? (pegar id da nota antes de remover estoque e apos a remocao verificar se nfe ainda possui
  // algum relacao em estoque_has_nfe, senao houver remover nfe
}

void ImportarXML::on_tableConsumo_entered(const QModelIndex &) { ui->tableConsumo->resizeColumnsToContents(); }

// TODO: corrigir consumo de estoque para consumir parcialmente de forma correta (criar varias linhas:
// idEstoque 1 - quant -10 // idEstoque 2 - quant - 20
// NOTE: se filtro por compra, é necessário a coluna 'selecionado' para escolher qual linha parear?
// NOTE: utilizar tabela em arvore (qtreeview) para agrupar consumos com seu estoque (para cada linha do model inserir
// items na arvore?)
// TODO: setar selecionado 0 antes de salvar a tabela de baixo
// NOTE: revisar codigo para verificar se ao pesquisar um produto pelo codComercial+status nao estou alterando outros
// produtos junto
// TODO: nao mudar status para em coleta dos produtos que nao foram importados (para o caso de vir apenas parte da
// compra)
// TODO: se a quantidade nao bater por m2 calcular por pc e vice versa?
// TODO: os que estiverem verdes em baixo nao modificar em importacoes sequentes
// TODO: verificar se estou usando nao apenas o codComercial mas tambem o fornecedor para evitar conflitos de codigos
// repetidos
// TODO: calcular valores proporcionais para o consumo
// TODO: deixar no consumo idNFe vazio para posteriormente guardar a nota de saida?
// TODO: use mysql::savepoints to restore to a point before importing a wrong xml
