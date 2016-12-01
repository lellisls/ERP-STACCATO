#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSqlError>
#include <QSqlQuery>
#include <QUrl>

#include "cadastrarnfe.h"
#include "calendarioentregas.h"
#include "doubledelegate.h"
#include "inputdialog.h"
#include "inputdialogconfirmacao.h"
#include "ui_calendarioentregas.h"
#include "usersession.h"

CalendarioEntregas::CalendarioEntregas(QWidget *parent) : QWidget(parent), ui(new Ui::CalendarioEntregas) {
  ui->setupUi(this);

  ui->splitter->setStretchFactor(0, 0);
  ui->splitter->setStretchFactor(1, 1);

  ui->pushButtonConfirmarEntrega->hide();
  ui->pushButtonGerarNFeEntregar->hide();
  ui->pushButtonImprimirDanfe->hide();
}

CalendarioEntregas::~CalendarioEntregas() { delete ui; }

bool CalendarioEntregas::updateTables() {
  if (modelCarga.tableName().isEmpty()) setupTables();

  if (not modelCarga.select()) {
    emit errorSignal("Erro lendo tabela de entregas: " + modelCarga.lastError().text());
    return false;
  }

  ui->tableCarga->resizeColumnsToContents();

  if (not modelCalendario.select()) {
    emit errorSignal("Erro lendo tabela calendario: " + modelCalendario.lastError().text());
    return false;
  }

  ui->tableCalendario->resizeColumnsToContents();

  if (not modelCarga.select()) {
    emit errorSignal("Erro lendo tabela calendario: " + modelCalendario.lastError().text());
    return false;
  }

  ui->tableCarga->resizeColumnsToContents();

  if (not modelProdutos.select()) {
    emit errorSignal("Erro lendo tabela calendario: " + modelCalendario.lastError().text());
    return false;
  }

  ui->tableProdutos->resizeColumnsToContents();

  return true;
}

void CalendarioEntregas::setupTables() {
  modelCalendario.setTable("view_calendario_entrega");
  modelCalendario.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelCalendario.setHeaderData("data", "Agendado");
  modelCalendario.setHeaderData("razaoSocial", "Transp.");
  modelCalendario.setHeaderData("modelo", "Modelo");
  modelCalendario.setHeaderData("placa", "Placa");
  modelCalendario.setHeaderData("idVenda", "Venda");
  modelCalendario.setHeaderData("status", "Status");

  if (not modelCalendario.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela calendario: " + modelCalendario.lastError().text());
  }

  ui->tableCalendario->setModel(&modelCalendario);

  //

  modelCarga.setTable("view_calendario_carga");
  modelCarga.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelCarga.setHeaderData("dataPrevEnt", "Agendado");
  modelCarga.setHeaderData("idVenda", "Venda");
  modelCarga.setHeaderData("status", "Status");
  modelCarga.setHeaderData("count(0)", "Produtos");

  modelCarga.setFilter("0");

  if (not modelCarga.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela cargas: " + modelCarga.lastError().text());
  }

  ui->tableCarga->setModel(&modelCarga);
  ui->tableCarga->hideColumn("idEvento");

  //

  modelProdutos.setTable("view_calendario_produto");
  modelProdutos.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelProdutos.setHeaderData("fornecedor", "Fornecedor");
  modelProdutos.setHeaderData("idVenda", "Venda");
  modelProdutos.setHeaderData("produto", "Produto");
  modelProdutos.setHeaderData("quant", "Quant.");
  modelProdutos.setHeaderData("idNfeSaida", "NFe");

  modelProdutos.setFilter("0");

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela produtos: " + modelProdutos.lastError().text());
  }

  ui->tableProdutos->setModel(&modelProdutos);
  ui->tableProdutos->hideColumn("idEvento");
  ui->tableProdutos->hideColumn("idVendaProduto");
}

void CalendarioEntregas::on_pushButtonReagendar_clicked() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not reagendar()) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  updateTables();
  QMessageBox::information(this, "Aviso!", "Reagendado com sucesso!");
}

bool CalendarioEntregas::reagendar() {
  const auto list = ui->tableCarga->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return false;
  }

  InputDialog *input = new InputDialog(InputDialog::AgendarEntrega, this);

  if (input->exec() != InputDialog::Accepted) return false;

  const QDateTime dataPrevEnt = input->getNextDate();

  for (auto const &item : list) {
    QSqlQuery query;

    for (int row = 0; row < modelProdutos.rowCount(); ++row) {
      query.prepare("UPDATE venda_has_produto SET dataPrevEnt = :dataPrevEnt WHERE idVendaProduto = :idVendaProduto");
      query.bindValue(":dataPrevEnt", dataPrevEnt);
      query.bindValue(":idVendaProduto", modelProdutos.data(row, "idVendaProduto"));

      if (not query.exec()) {
        QMessageBox::critical(this, "Erro!", "Erro atualizando data venda: " + query.lastError().text());
        return false;
      }

      query.prepare(
          "UPDATE pedido_fornecedor_has_produto SET dataPrevEnt = :dataPrevEnt WHERE idVendaProduto = :idVendaProduto");
      query.bindValue(":dataPrevEnt", dataPrevEnt);
      query.bindValue(":idVendaProduto", modelProdutos.data(row, "idVendaProduto"));

      if (not query.exec()) {
        QMessageBox::critical(this, "Erro!", "Erro atualizando data pedido_fornecedor: " + query.lastError().text());
        return false;
      }
    }

    query.prepare("UPDATE veiculo_has_produto SET data = :data WHERE idEvento = :idEvento");
    query.bindValue(":data", dataPrevEnt);
    query.bindValue(":idEvento", modelCarga.data(item.row(), "idEvento"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando data carga: " + query.lastError().text());
      return false;
    }
  }

  return true;
}

void CalendarioEntregas::on_pushButtonGerarNFeEntregar_clicked() {
  const auto list = ui->tableCarga->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  const QString idVenda = modelCarga.data(list.first().row(), "idVenda").toString();

  QList<int> lista;

  for (int row = 0; row < modelProdutos.rowCount(); ++row) {
    lista.append(modelProdutos.data(row, "idVendaProduto").toInt());
  }

  CadastrarNFe *nfe = new CadastrarNFe(idVenda, this);
  nfe->prepararNFe(lista);
  nfe->show();
}

void CalendarioEntregas::on_tableCalendario_clicked(const QModelIndex &index) {
  const QString data = modelCalendario.data(index.row(), "data").toString();

  modelCarga.setFilter("DATE_FORMAT(dataPrevEnt, '%d-%m-%Y') = '" + data + "'");

  if (not modelCarga.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela entregas: " + modelCarga.lastError().text());
  }

  ui->tableCarga->resizeColumnsToContents();

  modelProdutos.setFilter("0");

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela produtos: " + modelProdutos.lastError().text());
  }
}

void CalendarioEntregas::on_tableCarga_clicked(const QModelIndex &index) {
  modelProdutos.setFilter("idVenda = '" + modelCarga.data(index.row(), "idVenda").toString() + "'");

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela produtos: " + modelProdutos.lastError().text());
  }

  ui->tableProdutos->resizeColumnsToContents();

  const QString status = modelCarga.data(index.row(), "status").toString();

  if (status == "ENTREGA AGEND.") {
    ui->pushButtonGerarNFeEntregar->show();
    ui->pushButtonConfirmarEntrega->hide();
    ui->pushButtonImprimirDanfe->hide();
  }

  if (status == "EM ENTREGA") {
    ui->pushButtonGerarNFeEntregar->hide();
    ui->pushButtonConfirmarEntrega->show();
    ui->pushButtonImprimirDanfe->show();
  }
}

bool CalendarioEntregas::confirmarEntrega() {
  const auto list = ui->tableCarga->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return false;
  }

  InputDialogConfirmacao *inputDlg = new InputDialogConfirmacao(InputDialogConfirmacao::Entrega, this);
  inputDlg->setFilter(modelCarga.data(list.first().row(), "idVenda").toString());

  if (inputDlg->exec() != InputDialogConfirmacao::Accepted) return false;

  const QDateTime dataRealEnt = inputDlg->getDate();
  const QString entregou = inputDlg->getEntregou();

  QSqlQuery query;

  for (int row = 0; row < modelProdutos.rowCount(); ++row) {
    const int idVendaProduto = modelProdutos.data(row, "idVendaProduto").toInt();

    query.prepare("UPDATE veiculo_has_produto SET status = 'ENTREGUE' WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":idVendaProduto", idVendaProduto);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando veiculo_has_produto: " + query.lastError().text());
      return false;
    }

    query.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'ENTREGUE', dataRealEnt = :dataRealEnt WHERE "
                  "idVendaProduto = :idVendaProduto");
    query.bindValue(":dataRealEnt", dataRealEnt);
    query.bindValue(":idVendaProduto", idVendaProduto);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando pedido_fornecedor: " + query.lastError().text());
      return false;
    }

    query.prepare("UPDATE venda_has_produto SET status = 'ENTREGUE', entregou = :entregou, dataRealEnt = :dataRealEnt "
                  "WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":entregou", entregou);
    query.bindValue(":dataRealEnt", dataRealEnt);
    query.bindValue(":idVendaProduto", idVendaProduto);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando venda_produto: " + query.lastError().text());
      return false;
    }
  }

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    return false;
  }

  return true;
}

void CalendarioEntregas::on_pushButtonConfirmarEntrega_clicked() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not confirmarEntrega()) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  updateTables();
  QMessageBox::information(this, "Aviso!", "Entrega confirmada!");
}

void CalendarioEntregas::on_pushButtonImprimirDanfe_clicked() {
  if (not imprimirDanfe()) {
    const QString dirEntrada = UserSession::settings("User/pastaEntACBr").toString();
    const QString dirSaida = UserSession::settings("User/pastaSaiACBr").toString();

    QFile fileGerar(dirEntrada + "/gerarDanfe.txt");

    if (fileGerar.exists()) fileGerar.remove();

    QFile fileResposta(dirSaida + "/gerarDanfe-resp.txt");

    if (fileResposta.exists()) fileResposta.remove();
  }
}

bool CalendarioEntregas::imprimirDanfe() {
  // 1. buscar xml pelo idNfeSaida
  // 2. salvar xml em um arquivo na pasta de saida do ACBr
  // 3. enviar comando para ACBr gerar a danfe - NFe.ImprimirDanfePDF("C:\ACBrNFeMonitor\XML\chaveAcesso-nfe.xml")
  // 4. abrir arquivo pdf gerado - "C:\ACBrNFeMonitor\XML\chaveAcesso-nfe.pdf"

  const auto list = ui->tableCarga->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return false;
  }

  const QString dirEntrada = UserSession::settings("User/pastaEntACBr").toString();
  const QString dirSaida = UserSession::settings("User/pastaSaiACBr").toString();
  const QString dirXml = UserSession::settings("User/pastaXmlACBr").toString();

  QSqlQuery query;
  query.prepare("SELECT idNfeSaida FROM view_calendario_produto WHERE idVenda = :idVenda");
  query.bindValue(":idVenda", modelCarga.data(list.first().row(), "idVenda"));

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando NFe: " + query.lastError().text());
    return false;
  }

  const int id = query.value("idNfeSaida").toInt();

  if (id == 0) {
    QMessageBox::critical(this, "Erro!", "Produto não possui nota!");
    return false;
  }

  query.prepare("SELECT chaveAcesso, xml FROM nfe WHERE idNFe = :idNFe");
  query.bindValue(":idNFe", id);

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

// TODO: tela entregas (calendario) colocar filtros mostrando por dia/semana
