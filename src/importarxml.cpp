#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include "estoqueproxymodel.h"
#include "importarxml.h"
#include "noeditdelegate.h"
#include "reaisdelegate.h"
#include "singleeditdelegate.h"
#include "ui_importarxml.h"

ImportarXML::ImportarXML(const QStringList &idsCompra, const QDateTime &dataReal, QWidget *parent) : QDialog(parent), dataReal(dataReal), idsCompra(idsCompra), ui(new Ui::ImportarXML) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables(idsCompra);
}

ImportarXML::~ImportarXML() { delete ui; }

void ImportarXML::setupTables(const QStringList &idsCompra) {
  modelEstoque.setTable("estoque");
  modelEstoque.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelEstoque.setHeaderData("status", "Status");
  modelEstoque.setHeaderData("numeroNFe", "NFe");
  modelEstoque.setHeaderData("fornecedor", "Fornecedor");
  modelEstoque.setHeaderData("descricao", "Produto");
  modelEstoque.setHeaderData("lote", "Lote");
  modelEstoque.setHeaderData("local", "Local");
  modelEstoque.setHeaderData("bloco", "Bloco");
  modelEstoque.setHeaderData("quant", "Quant.");
  modelEstoque.setHeaderData("un", "Un.");
  modelEstoque.setHeaderData("caixas", "Cx.");
  modelEstoque.setHeaderData("codBarras", "Cód. Bar.");
  modelEstoque.setHeaderData("codComercial", "Cód. Com.");
  modelEstoque.setHeaderData("valorUnid", "R$ Unid.");
  modelEstoque.setHeaderData("valor", "R$");

  modelEstoque.setFilter("status = 'TEMP'");

  if (not modelEstoque.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + modelEstoque.lastError().text());
  }

  ui->tableEstoque->setModel(new EstoqueProxyModel(&modelEstoque, this));
  ui->tableEstoque->setItemDelegate(new NoEditDelegate(this));
  ui->tableEstoque->setItemDelegateForColumn("codComercial", new SingleEditDelegate(this));
  ui->tableEstoque->setItemDelegateForColumn("lote", new SingleEditDelegate(this));
  ui->tableEstoque->setItemDelegateForColumn("bloco", new SingleEditDelegate(this));
  ui->tableEstoque->setItemDelegateForColumn("quant", new SingleEditDelegate(this));
  ui->tableEstoque->setItemDelegateForColumn("un", new SingleEditDelegate(this));
  ui->tableEstoque->setItemDelegateForColumn("descricao", new SingleEditDelegate(this));
  ui->tableEstoque->setItemDelegateForColumn("valorUnid", new ReaisDelegate(this));
  ui->tableEstoque->setItemDelegateForColumn("valor", new ReaisDelegate(this));
  ui->tableEstoque->hideColumn("recebidoPor");
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

  modelConsumo.setHeaderData("status", "Status");
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

  modelConsumo.setFilter("status = 'TEMP'");

  if (not modelConsumo.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque_has_consumo: " + modelConsumo.lastError().text());
  }

  ui->tableConsumo->setModel(new EstoqueProxyModel(&modelConsumo, this));
  ui->tableConsumo->setItemDelegate(new NoEditDelegate(this));
  ui->tableConsumo->setItemDelegateForColumn("valorUnid", new ReaisDelegate(this));
  ui->tableConsumo->setItemDelegateForColumn("valor", new ReaisDelegate(this));
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

  modelCompra.setFilter("idCompra = " + idsCompra.join(" OR idCompra = "));

  if (not modelCompra.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela pedido_fornecedor_has_produto: " + modelCompra.lastError().text());
  }

  ui->tableCompra->setModel(new EstoqueProxyModel(&modelCompra, this));
  ui->tableCompra->setItemDelegate(new NoEditDelegate(this));
  ui->tableCompra->setItemDelegateForColumn("codComercial", new SingleEditDelegate(this));
  ui->tableCompra->setItemDelegateForColumn("descricao", new SingleEditDelegate(this));
  ui->tableCompra->setItemDelegateForColumn("prcUnitario", new ReaisDelegate(this));
  ui->tableCompra->setItemDelegateForColumn("preco", new ReaisDelegate(this));
  ui->tableCompra->hideColumn("idVendaProduto");
  ui->tableCompra->hideColumn("selecionado");
  ui->tableCompra->hideColumn("statusFinanceiro");
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
  ui->tableCompra->hideColumn("aliquotaSt");
  ui->tableCompra->hideColumn("st");

  ui->tableCompra->resizeColumnsToContents();

  //

  modelNFe.setTable("nfe");
  modelNFe.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelNFe.setFilter("0");

  if (not modelNFe.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela nfe: " + modelNFe.lastError().text());
  }

  modelEstoque_nfe.setTable("estoque_has_nfe");
  modelEstoque_nfe.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelEstoque_nfe.setFilter("0");

  if (not modelEstoque_nfe.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque_has_nfe: " + modelEstoque_nfe.lastError().text());
  }

  modelEstoque_compra.setTable("estoque_has_compra");
  modelEstoque_compra.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelEstoque_compra.setFilter("0");

  if (not modelEstoque_compra.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque_has_compra: " + modelEstoque_compra.lastError().text());
  }
}

bool ImportarXML::importar() {
  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    if (not modelEstoque.setData(row, "status", "EM COLETA")) return false;
  }

  for (int row = 0; row < modelConsumo.rowCount(); ++row) {
    if (not modelConsumo.setData(row, "status", "PRÉ-CONSUMO")) return false;
  }

  for (int row = 0; row < modelCompra.rowCount(); ++row)
    if (not modelCompra.setData(row, "selecionado", false)) return false;

  //--------------

  bool ok = false;

  for (int row = 0; row < modelCompra.rowCount(); ++row) {
    if (modelCompra.data(row, "quantUpd") == Green) {
      ok = true;
      break;
    }
  }

  if (not ok) {
    error = "Nenhuma compra pareada!";
    return false;
  }

  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    const int color = modelEstoque.data(row, "quantUpd").toInt();

    if (color == Red) {
      error = "Nem todos os estoques estão ok!";
      return false;
    }
  }

  if (not modelEstoque.submitAll()) {
    error = "Erro salvando dados da tabela estoque: " + modelEstoque.lastError().text();
    return false;
  }

  if (not modelCompra.submitAll()) {
    error = "Erro salvando dados da tabela compra: " + modelCompra.lastError().text();
    return false;
  }

  if (not modelConsumo.submitAll()) {
    error = "Erro salvando dados do consumo: " + modelConsumo.lastError().text();
    return false;
  }

  if (not modelEstoque_compra.submitAll()) {
    error = "Erro salvando modelEstoque_compra: " + modelEstoque_compra.lastError().text();
    return false;
  }

  if (not modelNFe.submitAll()) {
    error = "Erro salvando modelNFe: " + modelNFe.lastError().text();
    return false;
  }

  if (not modelEstoque_nfe.submitAll()) {
    error = "Erro salvando modelEstoque_nfe: " + modelEstoque_nfe.lastError().text();
    return false;
  }

  //------------------------------

  QSqlQuery query;
  query.prepare("UPDATE venda_has_produto SET status = 'EM COLETA', dataRealFat = :dataRealFat WHERE idVendaProduto = :idVendaProduto");

  for (int row = 0; row < modelCompra.rowCount(); ++row) {
    const int idVendaProduto = modelCompra.data(row, "idVendaProduto").toInt();
    const int color = modelCompra.data(row, "quantUpd").toInt();
    const QString status = modelCompra.data(row, "status").toString();

    if (idVendaProduto == 0) continue;
    if (color == White) continue;
    if (status != "EM FATURAMENTO") continue;

    query.bindValue(":dataRealFat", dataReal);
    query.bindValue(":idVendaProduto", modelCompra.data(row, "idVendaProduto"));

    if (not query.exec()) {
      error = "Erro atualizando status do produto da venda: " + query.lastError().text();
      return false;
    }
  }

  query.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'EM COLETA', dataRealFat = :dataRealFat WHERE idCompra = :idCompra AND quantUpd = 1 AND status = 'EM FATURAMENTO'");

  for (auto const &idCompra : idsCompra) {
    query.bindValue(":dataRealFat", dataReal);
    query.bindValue(":idCompra", idCompra);

    if (not query.exec()) {
      error = "Erro atualizando status da compra: " + query.lastError().text();
      return false;
    }
  }
  //------------------------------

  return true;
}

void ImportarXML::on_pushButtonImportar_clicked() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not importar()) {
    if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  QDialog::accept();
  close();
}

bool ImportarXML::limparAssociacoes() {
  for (int row = 0; row < modelCompra.rowCount(); ++row) {
    const QString status = modelCompra.data(row, "status").toString();
    const int color = modelCompra.data(row, "quantUpd").toInt();

    if (status != "EM FATURAMENTO" and color == Green) continue;

    if (not modelCompra.setData(row, "quantUpd", White)) return false;
    if (not modelCompra.setData(row, "quantConsumida", 0)) return false;
  }

  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    if (not modelEstoque.setData(row, "quantUpd", White)) return false;
  }

  while (modelConsumo.rowCount() > 0) {
    if (not modelConsumo.removeRow(0)) return false;
  }

  return true;
}

void ImportarXML::on_pushButtonProcurar_clicked() {
  disconnect(&modelEstoque, &QSqlTableModel::dataChanged, this, &ImportarXML::WrapParear);

  procurar();

  connect(&modelEstoque, &QSqlTableModel::dataChanged, this, &ImportarXML::WrapParear);
}

void ImportarXML::procurar() {
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

  file.close();

  if (not parear()) return;

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

bool ImportarXML::associarItens(const int rowCompra, const int rowEstoque, double &estoqueConsumido) {
  if (modelEstoque.data(rowEstoque, "quantUpd") == Green) return true;
  if (modelCompra.data(rowCompra, "quantUpd") == Green) return true;

  //-------------------------------

  const double quantEstoque = modelEstoque.data(rowEstoque, "quant").toDouble();
  const double quantCompra = modelCompra.data(rowCompra, "quant").toDouble();
  const double quantConsumida = modelCompra.data(rowCompra, "quantConsumida").toDouble();

  const double espaco = quantCompra - quantConsumida;
  const double estoqueDisponivel = quantEstoque - estoqueConsumido;
  const double quantAdicionar = estoqueDisponivel > espaco ? espaco : estoqueDisponivel;
  estoqueConsumido += quantAdicionar;

  if (not modelCompra.setData(rowCompra, "descricao", modelEstoque.data(rowEstoque, "descricao").toString())) {
    return false;
  }

  if (not modelCompra.setData(rowCompra, "quantConsumida", quantConsumida + quantAdicionar)) {
    return false;
  }

  if (not modelEstoque.setData(rowEstoque, "quantUpd", qFuzzyCompare(estoqueConsumido, quantEstoque) ? Green : Yellow)) {
    return false;
  }

  if (not modelCompra.setData(rowCompra, "quantUpd", qFuzzyCompare((quantConsumida + quantAdicionar), quantCompra) ? Green : Yellow)) {
    return false;
  }

  const int idCompra = modelCompra.data(rowCompra, "idCompra").toInt();
  const int idEstoque = modelEstoque.data(rowEstoque, "idEstoque").toInt();

  bool exists = false;

  for (int row = 0; row < modelEstoque_compra.rowCount(); ++row) {
    const int idEstoque_temp = modelEstoque_compra.data(row, "idEstoque").toInt();
    const int idCompra_temp = modelEstoque_compra.data(row, "idCompra").toInt();

    if (idEstoque_temp == idEstoque and idCompra_temp == idCompra) exists = true;
  }

  if (not exists) {
    const int rowEstoque_compra = modelEstoque_compra.rowCount();
    modelEstoque_compra.insertRow(rowEstoque_compra);

    if (not modelEstoque_compra.setData(rowEstoque_compra, "idEstoque", idEstoque)) return false;
    if (not modelEstoque_compra.setData(rowEstoque_compra, "idCompra", idCompra)) return false;
  }

  if (not criarConsumo2(rowCompra, rowEstoque, quantAdicionar)) return false;

  return true;
}

bool ImportarXML::verificaCNPJ(const XML &xml) {
  QSqlQuery queryLoja;

  // TODO: make this not hardcoded but still it shouldnt need the user to set a UserSession flag
  if (not queryLoja.exec("SELECT cnpj FROM loja WHERE descricao = 'CD'") or not queryLoja.first()) {
    QMessageBox::critical(nullptr, "Erro!", "Erro na query CNPJ: " + queryLoja.lastError().text());
    return false;
  }

  if (queryLoja.value("cnpj").toString().remove(".").remove("/").remove("-") != xml.cnpj) {
    QMessageBox msgBox(QMessageBox::Question, "Atenção!", "CNPJ da nota não é do galpão. Continuar?", QMessageBox::Yes | QMessageBox::No, this);
    msgBox.setButtonText(QMessageBox::Yes, "Continuar");
    msgBox.setButtonText(QMessageBox::No, "Voltar");

    return msgBox.exec() == QMessageBox::Yes;
  }

  return true;
}

bool ImportarXML::verificaExiste(XML &xml) {
  QSqlQuery query;
  query.prepare("SELECT idNFe FROM nfe WHERE chaveAcesso = :chaveAcesso");
  query.bindValue(":chaveAcesso", xml.chaveAcesso);

  if (not query.exec()) {
    QMessageBox::critical(nullptr, "Erro!", "Erro verificando se nota já cadastrada: " + query.lastError().text());
    return false;
  }

  if (query.first()) {
    QMessageBox::critical(nullptr, "Erro!", "Nota já cadastrada!");
    return true;
  }

  const auto list = modelNFe.match(modelNFe.index(0, modelNFe.fieldIndex("chaveAcesso")), Qt::DisplayRole, xml.chaveAcesso);

  if (list.size() > 0) {
    QMessageBox::critical(nullptr, "Erro!", "Nota já cadastrada!");
    return true;
  }

  return false;
}

bool ImportarXML::cadastrarNFe(XML &xml) {
  if (not verificaCNPJ(xml) or verificaExiste(xml)) return false;

  QFile file(xml.fileName);

  if (not file.open(QFile::ReadOnly)) {
    QMessageBox::critical(nullptr, "Erro!", "Erro lendo arquivo: " + file.errorString());
    return false;
  }

  QSqlQuery query;

  if (not query.exec("SELECT COALESCE(MAX(idNFe), 1) AS idNFe FROM nfe") or not query.first()) {
    QMessageBox::critical(nullptr, "Erro!", "Erro buscando próximo id: " + query.lastError().text());
    return false;
  }

  xml.idNFe = query.value("idNFe").toInt();

  for (int row = 0; row < modelNFe.rowCount(); ++row) {
    const int id = modelNFe.data(row, "idNFe").toInt();
    if (id > xml.idNFe) xml.idNFe = id;
  }

  xml.idNFe++;

  const int row = modelNFe.rowCount();
  if (not modelNFe.insertRow(row)) return false;

  if (not modelNFe.setData(row, "idNFe", xml.idNFe)) return false;
  if (not modelNFe.setData(row, "tipo", "ENTRADA")) return false;
  if (not modelNFe.setData(row, "cnpjDest", xml.cnpj)) return false;
  if (not modelNFe.setData(row, "chaveAcesso", xml.chaveAcesso)) return false;
  if (not modelNFe.setData(row, "numeroNFe", xml.nNF)) return false;
  if (not modelNFe.setData(row, "xml", file.readAll())) return false;
  if (not modelNFe.setData(row, "transportadora", xml.xNomeTransp)) return false;

  return true;
}

bool ImportarXML::lerXML(QFile &file) {
  XML xml(file.readAll(), file.fileName());

  if (not cadastrarNFe(xml)) return false;
  if (not perguntarLocal(xml)) return false;
  if (not inserirNoSqlModel(xml, xml.model.item(0, 0))) return false;

  return true;
}

bool ImportarXML::perguntarLocal(XML &xml) {
  QSqlQuery query;

  if (not query.exec("SELECT descricao FROM loja WHERE descricao != '' and descricao != 'CD'")) {
    QMessageBox::critical(this, "Erro!", "Erro buscando lojas: " + query.lastError().text());
    return false;
  }

  QStringList lojas{"CD"};

  while (query.next()) lojas << query.value("descricao").toString();

  QInputDialog input;
  input.setInputMode(QInputDialog::TextInput);
  input.setCancelButtonText("Cancelar");
  input.setWindowTitle("Local");
  input.setLabelText("Local do depósito:");
  input.setComboBoxItems(lojas);
  input.setComboBoxEditable(false);

  if (input.exec() != QInputDialog::Accepted) return false;

  xml.local = input.textValue();

  return true;
}

bool ImportarXML::inserirItemSql(XML &xml) {
  const auto list = modelEstoque.match(modelEstoque.index(0, modelEstoque.fieldIndex("codComercial")), Qt::DisplayRole, xml.codProd, -1, Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap));

  for (auto const &item : list) {
    QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Produto é do mesmo lote da linha " + QString::number(item.row() + 1) + "?", QMessageBox::Yes | QMessageBox::No, nullptr);
    msgBox.setButtonText(QMessageBox::Yes, "Sim");
    msgBox.setButtonText(QMessageBox::No, "Não");

    if (msgBox.exec() == QMessageBox::Yes) {
      const int row = item.row();

      const double newQuant = xml.quant + modelEstoque.data(row, "quant").toDouble();

      if (not modelEstoque.setData(row, "quant", newQuant)) return false;

      return true;
    }
  }

  const int row = modelEstoque.rowCount();

  if (not modelEstoque.insertRow(row)) {
    QMessageBox::critical(nullptr, "Erro!", "Erro inserindo linha na tabela: " + modelEstoque.lastError().text());
    return false;
  }

  // remove 'A'
  if (xml.xNome == "CECRISA REVEST. CERAMICOS S.A." and xml.codProd.endsWith("A")) xml.codProd = xml.codProd.left(xml.codProd.size() - 1);

  QSqlQuery query;

  if (not query.exec("SELECT COALESCE(MAX(idEstoque), 1) AS idEstoque FROM estoque") or not query.first()) {
    QMessageBox::critical(nullptr, "Erro!", "Erro buscando próximo id: " + query.lastError().text());
    return false;
  }

  int idEstoque = query.value("idEstoque").toInt();

  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    const int id = modelEstoque.data(row, "idEstoque").toInt();
    if (id > idEstoque) idEstoque = id;
  }

  idEstoque++;

  if (not modelEstoque.setData(row, "idEstoque", idEstoque)) return false;
  if (not modelEstoque.setData(row, "fornecedor", xml.xNome)) return false;
  if (not modelEstoque.setData(row, "local", xml.local)) return false;
  if (not modelEstoque.setData(row, "descricao", xml.descricao)) return false;
  if (not modelEstoque.setData(row, "quant", xml.quant)) return false;
  if (not modelEstoque.setData(row, "un", xml.un)) return false;
  if (not modelEstoque.setData(row, "codBarras", xml.codBarras)) return false;
  if (not modelEstoque.setData(row, "codComercial", xml.codProd)) return false;
  if (not modelEstoque.setData(row, "ncm", xml.ncm)) return false;
  if (not modelEstoque.setData(row, "cfop", xml.cfop)) return false;
  if (not modelEstoque.setData(row, "valorUnid", xml.valorUnid)) return false;
  if (not modelEstoque.setData(row, "valor", xml.valor)) return false;
  if (not modelEstoque.setData(row, "codBarrasTrib", xml.codBarrasTrib)) return false;
  if (not modelEstoque.setData(row, "unTrib", xml.unTrib)) return false;
  if (not modelEstoque.setData(row, "quantTrib", xml.quantTrib)) return false;
  if (not modelEstoque.setData(row, "valorTrib", xml.valorTrib)) return false;
  if (not modelEstoque.setData(row, "desconto", xml.desconto)) return false;
  if (not modelEstoque.setData(row, "compoeTotal", xml.compoeTotal)) return false;
  if (not modelEstoque.setData(row, "numeroPedido", xml.numeroPedido)) return false;
  if (not modelEstoque.setData(row, "itemPedido", xml.itemPedido)) return false;
  if (not modelEstoque.setData(row, "tipoICMS", xml.tipoICMS)) return false;
  if (not modelEstoque.setData(row, "orig", xml.orig)) return false;
  if (not modelEstoque.setData(row, "cstICMS", xml.cstICMS)) return false;
  if (not modelEstoque.setData(row, "modBC", xml.modBC)) return false;
  if (not modelEstoque.setData(row, "vBC", xml.vBC)) return false;
  if (not modelEstoque.setData(row, "pICMS", xml.pICMS)) return false;
  if (not modelEstoque.setData(row, "vICMS", xml.vICMS)) return false;
  if (not modelEstoque.setData(row, "modBCST", xml.modBCST)) return false;
  if (not modelEstoque.setData(row, "pMVAST", xml.pMVAST)) return false;
  if (not modelEstoque.setData(row, "vBCST", xml.vBCST)) return false;
  if (not modelEstoque.setData(row, "pICMSST", xml.pICMSST)) return false;
  if (not modelEstoque.setData(row, "vICMSST", xml.vICMSST)) return false;
  if (not modelEstoque.setData(row, "cEnq", xml.cEnq)) return false;
  if (not modelEstoque.setData(row, "cstIPI", xml.cstIPI)) return false;
  if (not modelEstoque.setData(row, "cstPIS", xml.cstPIS)) return false;
  if (not modelEstoque.setData(row, "vBCPIS", xml.vBCPIS)) return false;
  if (not modelEstoque.setData(row, "pPIS", xml.pPIS)) return false;
  if (not modelEstoque.setData(row, "vPIS", xml.vPIS)) return false;
  if (not modelEstoque.setData(row, "cstCOFINS", xml.cstCOFINS)) return false;
  if (not modelEstoque.setData(row, "vBCCOFINS", xml.vBCCOFINS)) return false;
  if (not modelEstoque.setData(row, "pCOFINS", xml.pCOFINS)) return false;
  if (not modelEstoque.setData(row, "vCOFINS", xml.vCOFINS)) return false;
  if (not modelEstoque.setData(row, "status", "TEMP")) return false;

  const int rowNFe = modelEstoque_nfe.rowCount();
  modelEstoque_nfe.insertRow(rowNFe);

  if (not modelEstoque_nfe.setData(rowNFe, "idEstoque", idEstoque)) return false;
  if (not modelEstoque_nfe.setData(rowNFe, "idNFe", xml.idNFe)) return false;

  return true;
}

bool ImportarXML::inserirNoSqlModel(XML &xml, const QStandardItem *item) {
  for (int row = 0; row < item->rowCount(); ++row) {
    for (int col = 0; col < item->columnCount(); ++col) {
      const QStandardItem *child = item->child(row, col);
      const QString text = child->text();

      if (text.mid(0, 10) == "det nItem=") {
        xml.lerValores(child);
        if (not inserirItemSql(xml)) return false;
      }

      if (child->hasChildren()) inserirNoSqlModel(xml, child);
    }
  }

  return true;
}

bool ImportarXML::criarConsumo2(const int rowCompra, const int rowEstoque, const double quantAdicionar) {
  const int idVendaProduto = modelCompra.data(rowCompra, "idVendaProduto").toInt();
  const int idEstoque = modelEstoque.data(rowEstoque, "idEstoque").toInt();
  const QString codCompra = modelCompra.data(rowCompra, "codComercial").toString();
  const QString codEstoque = modelEstoque.data(rowEstoque, "codComercial").toString();

  if (idVendaProduto == 0) return true;
  if (codCompra != codEstoque) return true;

  const int rowConsumo = modelConsumo.rowCount();
  modelConsumo.insertRow(rowConsumo);

  for (int column = 0; column < modelEstoque.columnCount(); ++column) {
    const QString field = modelEstoque.record().fieldName(column);
    const int index = modelConsumo.fieldIndex(field);

    if (index == -1) continue;

    if (modelEstoque.fieldIndex("created") == column) continue;
    if (modelEstoque.fieldIndex("lastUpdated") == column) continue;

    const QVariant value = modelEstoque.data(rowEstoque, column);

    if (not modelConsumo.setData(rowConsumo, index, value)) return false;
  }

  const double quant = quantAdicionar;

  // -------------------------------------

  QSqlQuery query;
  query.prepare("SELECT UPPER(un) AS un, m2cx, pccx FROM produto WHERE idProduto = :idProduto");
  query.bindValue(":idProduto", modelCompra.data(rowCompra, "idProduto"));

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados do produto: " + query.lastError().text());
    return false;
  }

  const QString un = query.value("un").toString();
  const double m2cx = query.value("m2cx").toDouble();
  const double pccx = query.value("pccx").toDouble();

  const double unCaixa = un == "M2" or un == "M²" or un == "ML" ? m2cx : pccx;

  const double caixas = qRound(quant / unCaixa * 100) / 100;

  const double proporcao = quant / modelEstoque.data(rowEstoque, "quant").toDouble();

  const double desconto = modelEstoque.data(rowEstoque, "desconto").toDouble() * proporcao;
  const double valor = modelEstoque.data(rowEstoque, "valor").toDouble() * proporcao;
  const double vBC = modelEstoque.data(rowEstoque, "vBC").toDouble() * proporcao;
  const double vICMS = modelEstoque.data(rowEstoque, "vICMS").toDouble() * proporcao;
  const double vBCST = modelEstoque.data(rowEstoque, "vBCST").toDouble() * proporcao;
  const double vICMSST = modelEstoque.data(rowEstoque, "vICMSST").toDouble() * proporcao;
  const double vBCPIS = modelEstoque.data(rowEstoque, "vBCPIS").toDouble() * proporcao;
  const double vPIS = modelEstoque.data(rowEstoque, "vPIS").toDouble() * proporcao;
  const double vBCCOFINS = modelEstoque.data(rowEstoque, "vBCCOFINS").toDouble() * proporcao;
  const double vCOFINS = modelEstoque.data(rowEstoque, "vCOFINS").toDouble() * proporcao;

  // -------------------------------------

  if (not modelConsumo.setData(rowConsumo, "quant", quant * -1)) return false;
  if (not modelConsumo.setData(rowConsumo, "caixas", caixas)) return false;
  if (not modelConsumo.setData(rowConsumo, "quantUpd", DarkGreen)) return false;
  if (not modelConsumo.setData(rowConsumo, "idVendaProduto", idVendaProduto)) return false;
  if (not modelConsumo.setData(rowConsumo, "idEstoque", idEstoque)) return false;
  if (not modelConsumo.setData(rowConsumo, "desconto", desconto)) return false;
  if (not modelConsumo.setData(rowConsumo, "valor", valor)) return false;
  if (not modelConsumo.setData(rowConsumo, "vBC", vBC)) return false;
  if (not modelConsumo.setData(rowConsumo, "vICMS", vICMS)) return false;
  if (not modelConsumo.setData(rowConsumo, "vBCST", vBCST)) return false;
  if (not modelConsumo.setData(rowConsumo, "vICMSST", vICMSST)) return false;
  if (not modelConsumo.setData(rowConsumo, "vBCPIS", vBCPIS)) return false;
  if (not modelConsumo.setData(rowConsumo, "vPIS", vPIS)) return false;
  if (not modelConsumo.setData(rowConsumo, "vBCCOFINS", vBCCOFINS)) return false;
  if (not modelConsumo.setData(rowConsumo, "vCOFINS", vCOFINS)) return false;

  return true;
}

bool ImportarXML::criarConsumo() {
  for (int rowEstoque = 0; rowEstoque < modelEstoque.rowCount(); ++rowEstoque) {
    for (int rowCompra = 0; rowCompra < modelCompra.rowCount(); ++rowCompra) {
      const int idVendaProduto = modelCompra.data(rowCompra, "idVendaProduto").toInt();
      const int idEstoque = modelEstoque.data(rowEstoque, "idEstoque").toInt();
      const QString codCompra = modelCompra.data(rowCompra, "codComercial").toString();
      const QString codEstoque = modelEstoque.data(rowEstoque, "codComercial").toString();

      if (idVendaProduto == 0) continue;
      if (codCompra != codEstoque) continue;

      bool exists = false;

      for (int row = 0; row < modelConsumo.rowCount(); ++row) {
        const int idVendaProdutoTemp = modelConsumo.data(row, "idVendaProduto").toInt();
        const int idEstoqueTemp = modelConsumo.data(row, "idEstoque").toInt();

        if (idVendaProduto == idVendaProdutoTemp and idEstoque == idEstoqueTemp) exists = true;
      }

      if (exists) continue;

      //

      const int rowConsumo = modelConsumo.rowCount();
      modelConsumo.insertRow(rowConsumo);

      for (int column = 0; column < modelEstoque.columnCount(); ++column) {
        const QString field = modelEstoque.record().fieldName(column);
        const int index = modelConsumo.fieldIndex(field);

        if (index == -1) continue;

        if (modelEstoque.fieldIndex("created") == column) continue;
        if (modelEstoque.fieldIndex("lastUpdated") == column) continue;

        const QVariant value = modelEstoque.data(rowEstoque, column);

        if (not modelConsumo.setData(rowConsumo, index, value)) return false;
      }

      const double quant = modelCompra.data(rowCompra, "quant").toDouble();

      // -------------------------------------

      QSqlQuery query;
      query.prepare("SELECT UPPER(un) AS un, m2cx, pccx FROM produto WHERE idProduto = :idProduto");
      query.bindValue(":idProduto", modelCompra.data(rowCompra, "idProduto"));

      if (not query.exec() or not query.first()) {
        QMessageBox::critical(this, "Erro!", "Erro buscando dados do produto: " + query.lastError().text());
        return false;
      }

      const QString un = query.value("un").toString();
      const double m2cx = query.value("m2cx").toDouble();
      const double pccx = query.value("pccx").toDouble();

      const double unCaixa = un == "M2" or un == "M²" or un == "ML" ? m2cx : pccx;

      const double caixas = qRound(quant / unCaixa * 100) / 100;

      const double proporcao = quant / modelEstoque.data(rowEstoque, "quant").toDouble();

      const double desconto = modelEstoque.data(rowEstoque, "desconto").toDouble() * proporcao;
      const double valor = modelEstoque.data(rowEstoque, "valor").toDouble() * proporcao;
      const double vBC = modelEstoque.data(rowEstoque, "vBC").toDouble() * proporcao;
      const double vICMS = modelEstoque.data(rowEstoque, "vICMS").toDouble() * proporcao;
      const double vBCST = modelEstoque.data(rowEstoque, "vBCST").toDouble() * proporcao;
      const double vICMSST = modelEstoque.data(rowEstoque, "vICMSST").toDouble() * proporcao;
      const double vBCPIS = modelEstoque.data(rowEstoque, "vBCPIS").toDouble() * proporcao;
      const double vPIS = modelEstoque.data(rowEstoque, "vPIS").toDouble() * proporcao;
      const double vBCCOFINS = modelEstoque.data(rowEstoque, "vBCCOFINS").toDouble() * proporcao;
      const double vCOFINS = modelEstoque.data(rowEstoque, "vCOFINS").toDouble() * proporcao;

      // -------------------------------------

      if (not modelConsumo.setData(rowConsumo, "quant", quant * -1)) return false;
      if (not modelConsumo.setData(rowConsumo, "caixas", caixas)) return false;
      if (not modelConsumo.setData(rowConsumo, "quantUpd", DarkGreen)) return false;
      if (not modelConsumo.setData(rowConsumo, "idVendaProduto", idVendaProduto)) return false;
      if (not modelConsumo.setData(rowConsumo, "idEstoque", idEstoque)) return false;
      if (not modelConsumo.setData(rowConsumo, "desconto", desconto)) return false;
      if (not modelConsumo.setData(rowConsumo, "valor", valor)) return false;
      if (not modelConsumo.setData(rowConsumo, "vBC", vBC)) return false;
      if (not modelConsumo.setData(rowConsumo, "vICMS", vICMS)) return false;
      if (not modelConsumo.setData(rowConsumo, "vBCST", vBCST)) return false;
      if (not modelConsumo.setData(rowConsumo, "vICMSST", vICMSST)) return false;
      if (not modelConsumo.setData(rowConsumo, "vBCPIS", vBCPIS)) return false;
      if (not modelConsumo.setData(rowConsumo, "vPIS", vPIS)) return false;
      if (not modelConsumo.setData(rowConsumo, "vBCCOFINS", vBCCOFINS)) return false;
      if (not modelConsumo.setData(rowConsumo, "vCOFINS", vCOFINS)) return false;
    }
  }

  return true;
}

void ImportarXML::on_pushButtonCancelar_clicked() { close(); }

void ImportarXML::WrapParear() { // TODO: terminar de refatorar e homologar
  disconnect(&modelEstoque, &QSqlTableModel::dataChanged, this, &ImportarXML::WrapParear);

  parear();

  connect(&modelEstoque, &QSqlTableModel::dataChanged, this, &ImportarXML::WrapParear);
}

bool ImportarXML::parear() {
  if (not limparAssociacoes()) return false;

  for (int rowEstoque = 0, total = modelEstoque.rowCount(); rowEstoque < total; ++rowEstoque) {
    QVector<int> listCompra;

    for (int row = 0; row < modelCompra.rowCount(); ++row) {
      if (modelCompra.data(row, "status").toString() == "EM FATURAMENTO" and modelCompra.data(row, "codComercial").toString() == modelEstoque.data(rowEstoque, "codComercial").toString()) {
        listCompra << row;
      }
    }

    if (listCompra.isEmpty()) {
      if (not modelEstoque.setData(rowEstoque, "quantUpd", Red)) return false;
      continue;
    }

    QSqlQuery query;
    query.prepare("SELECT idProduto, m2cx, pccx, UPPER(un) AS un FROM produto WHERE codComercial = :codComercial");
    query.bindValue(":codComercial", modelEstoque.data(rowEstoque, "codComercial"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro lendo tabela produto: " + query.lastError().text());
      return false;
    }

    if (not query.first()) {
      QMessageBox::critical(this, "Erro!", "Produto não cadastrado: " + modelEstoque.data(rowEstoque, "codComercial").toString());
      return false;
    }

    const QString un = modelEstoque.data(rowEstoque, "un").toString();

    const double quantCaixa = un == "M2" or un == "M²" or un == "ML" ? query.value("m2cx").toDouble() : query.value("pccx").toDouble();

    const double quant = modelEstoque.data(rowEstoque, "quant").toDouble();

    const double caixas = qRound(quant / quantCaixa * 100) / 100;

    //    if (not modelEstoque.setData(rowEstoque, "idProduto", modelCompra.data(listCompra.first().row(),
    //    "idProduto"))) {
    if (not modelEstoque.setData(rowEstoque, "idProduto", modelCompra.data(listCompra.first(), "idProduto"))) {
      return false;
    }

    if (not modelEstoque.setData(rowEstoque, "caixas", caixas)) return false;

    double estoqueConsumido = 0;

    //------------------------ procurar quantidades iguais
    for (auto const &item : listCompra) { // TODO: trocar essa logica por fazer uma busca interna dentro do associarItens()
      const double quantEstoque = modelEstoque.data(rowEstoque, "quant").toDouble();
      //      const double quantCompra = modelCompra.data(item.row(), "quant").toDouble();
      const double quantCompra = modelCompra.data(item, "quant").toDouble();

      if (quantEstoque == quantCompra) {
        //          if (not associarItens(item.row(), rowEstoque, estoqueConsumido)) return false;
        if (not associarItens(item, rowEstoque, estoqueConsumido)) return false;
      }
    }
    //------------------------

    for (auto const &item : listCompra) {
      //        if (not associarItens(item.row(), rowEstoque, estoqueConsumido)) return false;
      if (not associarItens(item, rowEstoque, estoqueConsumido)) return false;
    }
  }

  //  if (not criarConsumo()) return false;

  ui->tableEstoque->clearSelection();
  ui->tableCompra->clearSelection();

  ui->tableCompra->setFocus(); // for updating proxyModel

  return true;
}

void ImportarXML::on_tableEstoque_entered(const QModelIndex &) { ui->tableEstoque->resizeColumnsToContents(); }

void ImportarXML::on_tableCompra_entered(const QModelIndex &) { ui->tableCompra->resizeColumnsToContents(); }

void ImportarXML::on_tableConsumo_entered(const QModelIndex &) { ui->tableConsumo->resizeColumnsToContents(); }

// NOTE: utilizar tabela em arvore (qtreeview) para agrupar consumos com seu estoque (para cada linha do model inserir
// items na arvore?)
// TODO: quando as unidades vierem diferente pedir para usuario converter
// TODO: substituir 'quantConsumida' por separacao de linha na associacao de nota/compra
// TODO: avisar se R$ da nota for diferente do R$ da compra
