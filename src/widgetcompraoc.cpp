#include <QDateTime>
#include <QDesktopServices>
#include <QFile>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSqlError>
#include <QTextStream>
#include <QUrl>

#include "ui_widgetcompraoc.h"
#include "usersession.h"
#include "widgetcompraoc.h"

WidgetCompraOC::WidgetCompraOC(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompraOC) { ui->setupUi(this); }

WidgetCompraOC::~WidgetCompraOC() { delete ui; }

bool WidgetCompraOC::updateTables() {
  if (modelPedido.tableName().isEmpty()) setupTables();

  if (not modelPedido.select()) {
    emit errorSignal("Erro lendo tabela pedidos: " + modelPedido.lastError().text());
    return false;
  }

  if (not modelProduto.select()) {
    emit errorSignal("Erro lendo tabela produtos pedido: " + modelProduto.lastError().text());
    return false;
  }

  return true;
}

void WidgetCompraOC::setupTables() {
  modelPedido.setTable("view_ordemcompra");
  modelPedido.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelPedido.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela pedidos: " + modelPedido.lastError().text());
  }

  ui->tablePedido->setModel(&modelPedido);

  modelProduto.setTable("pedido_fornecedor_has_produto");
  modelProduto.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelProduto.setFilter("0");

  if (not modelProduto.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela produtos pedido: " + modelProduto.lastError().text());
  }

  ui->tableProduto->setModel(&modelProduto);
  ui->tableProduto->hideColumn("idPedido");
  ui->tableProduto->hideColumn("selecionado");
  ui->tableProduto->hideColumn("statusFinanceiro");
  ui->tableProduto->hideColumn("idVendaProduto");
  ui->tableProduto->hideColumn("ordemCompra");
  ui->tableProduto->hideColumn("idCompra");
  ui->tableProduto->hideColumn("idProduto");
  ui->tableProduto->hideColumn("quantUpd");
  ui->tableProduto->hideColumn("quantConsumida");
  ui->tableProduto->hideColumn("aliquotaSt");
  ui->tableProduto->hideColumn("st");
  ui->tableProduto->hideColumn("dataPrevCompra");
  ui->tableProduto->hideColumn("dataRealCompra");
  ui->tableProduto->hideColumn("dataPrevConf");
  ui->tableProduto->hideColumn("dataRealConf");
  ui->tableProduto->hideColumn("dataPrevFat");
  ui->tableProduto->hideColumn("dataRealFat");
  ui->tableProduto->hideColumn("dataPrevColeta");
  ui->tableProduto->hideColumn("dataRealColeta");
  ui->tableProduto->hideColumn("dataPrevReceb");
  ui->tableProduto->hideColumn("dataRealReceb");
  ui->tableProduto->hideColumn("dataPrevEnt");
  ui->tableProduto->hideColumn("dataRealEnt");

  modelNFe.setTable("view_ordemcompra_nfe");
  modelNFe.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelNFe.setHeaderData("numeroNFe", "NFe");

  modelNFe.setFilter("0");

  if (not modelNFe.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela nfe: " + modelNFe.lastError().text());
  }

  ui->tableNFe->setModel(&modelNFe);
  ui->tableNFe->hideColumn("ordemCompra");
}

void WidgetCompraOC::on_tablePedido_clicked(const QModelIndex &index) {
  const QString oc = modelPedido.data(index.row(), "OC").toString();

  modelProduto.setFilter("ordemCompra = " + oc);

  modelNFe.setFilter("ordemCompra = " + oc);
}

void WidgetCompraOC::on_pushButtonDanfe_clicked() {
  if (not imprimirDanfe()) {
    const QString dirEntrada = UserSession::settings("User/pastaEntACBr").toString();
    const QString dirSaida = UserSession::settings("User/pastaSaiACBr").toString();

    QFile fileGerar(dirEntrada + "/gerarDanfe.txt");

    if (fileGerar.exists()) fileGerar.remove();

    QFile fileResposta(dirSaida + "/gerarDanfe-resp.txt");

    if (fileResposta.exists()) fileResposta.remove();
  }
}

bool WidgetCompraOC::imprimirDanfe() {
  // 1. buscar xml pelo idNfeSaida
  // 2. salvar xml em um arquivo na pasta de saida do ACBr
  // 3. enviar comando para ACBr gerar a danfe - NFe.ImprimirDanfePDF("C:\ACBrNFeMonitor\XML\chaveAcesso-nfe.xml")
  // 4. abrir arquivo pdf gerado - "C:\ACBrNFeMonitor\XML\chaveAcesso-nfe.pdf"

  const auto list = ui->tableNFe->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return false;
  }

  const QString dirEntrada = UserSession::settings("User/pastaEntACBr").toString();
  const QString dirSaida = UserSession::settings("User/pastaSaiACBr").toString();
  const QString dirXml = UserSession::settings("User/pastaXmlACBr").toString();

  QSqlQuery query;
  query.prepare("SELECT chaveAcesso, xml FROM nfe WHERE numeroNFe = :numeroNFe");
  query.bindValue(":numeroNFe", modelNFe.data(list.first().row(), "numeroNFe"));

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando chaveAcesso: " + query.lastError().text());
  }

  const QString chaveAcesso = query.value("chaveAcesso").toString();

  QFile fileXml(dirXml + "/" + chaveAcesso + "-nfe.xml");

  if (not fileXml.open(QFile::WriteOnly)) {
    QMessageBox::critical(this, "Erro!", "Não foi possível salvar o XML, favor verificar se as pastas "
                                         "estão corretamente configuradas.");
    return false;
  }

  QTextStream streamXml(&fileXml);

  streamXml << query.value("xml").toString();

  streamXml.flush();
  fileXml.close();

  QFile fileGerar(dirEntrada + "/gerarDanfe.txt");

  if (not fileGerar.open(QFile::WriteOnly)) {
    QMessageBox::critical(this, "Erro!", "Não foi possível enviar o pedido para o ACBr, favor verificar se as pastas "
                                         "estão corretamente configuradas.");
    return false;
  }

  QTextStream streamGerar(&fileGerar);

  streamGerar << "NFe.ImprimirDanfePDF(\"" + dirXml + "/" + chaveAcesso + "-nfe.xml\")" << endl;

  streamGerar.flush();
  fileGerar.close();

  //

  QFile fileResposta(dirSaida + "/gerarDanfe-resp.txt");

  QProgressDialog *progressDialog = new QProgressDialog(this);
  progressDialog->reset();
  progressDialog->setCancelButton(0);
  progressDialog->setLabelText("Esperando ACBr...");
  progressDialog->setWindowTitle("ERP Staccato");
  progressDialog->setWindowModality(Qt::WindowModal);
  progressDialog->setMaximum(0);
  progressDialog->setMinimum(0);
  progressDialog->show();

  QTime wait = QTime::currentTime().addSecs(10);

  while (QTime::currentTime() < wait) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    if (fileResposta.exists()) break;
  }

  progressDialog->cancel();

  if (not fileResposta.exists()) {
    QMessageBox::critical(this, "Erro!", "ACBr não respondeu, verificar se ele está aberto e funcionando!");

    if (fileGerar.exists()) fileGerar.remove();

    return false;
  }

  if (not fileResposta.open(QFile::ReadOnly)) {
    QMessageBox::critical(this, "Erro!", "Erro lendo arquivo: " + fileResposta.errorString());
    return false;
  }

  QTextStream ts(&fileResposta);

  const QString resposta = ts.readAll();
  fileResposta.remove();

  if (not resposta.contains("OK")) {
    QMessageBox::critical(this, "Erro!", "Resposta do ACBr: " + resposta);
    return false;
  }

  //

  if (not QDesktopServices::openUrl(QUrl::fromLocalFile(dirXml + "/" + chaveAcesso + "-nfe.pdf"))) {
    QMessageBox::critical(this, "Erro!", "Erro abrindo PDF!");
  }

  return true;
}

void WidgetCompraOC::on_tablePedido_entered(const QModelIndex &) { ui->tablePedido->resizeColumnsToContents(); }

void WidgetCompraOC::on_tableProduto_entered(const QModelIndex &) { ui->tableProduto->resizeColumnsToContents(); }

void WidgetCompraOC::on_tableNFe_entered(const QModelIndex &) { ui->tableNFe->resizeColumnsToContents(); }
