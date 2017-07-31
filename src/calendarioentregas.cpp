#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSqlError>
#include <QSqlQuery>
#include <QUrl>
#include <xlsxdocument.h>

#include "acbr.h"
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

  if (modelCarga.rowCount() == 0) modelProdutos.setFilter("0");

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
  modelCalendario.setHeaderData("modelo", "Modelo");
  modelCalendario.setHeaderData("razaoSocial", "Transp.");
  modelCalendario.setHeaderData("kg", "Kg.");
  modelCalendario.setHeaderData("idVenda", "Venda");

  if (not modelCalendario.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela calendario: " + modelCalendario.lastError().text());
  }

  ui->tableCalendario->setModel(&modelCalendario);
  ui->tableCalendario->hideColumn("idVeiculo");
  ui->tableCalendario->setItemDelegateForColumn("kg", new DoubleDelegate(this));

  //

  modelCarga.setTable("view_calendario_carga");
  modelCarga.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelCarga.setHeaderData("dataPrevEnt", "Agendado");
  modelCarga.setHeaderData("numeroNFe", "NFe");
  modelCarga.setHeaderData("idVenda", "Venda");
  modelCarga.setHeaderData("status", "Status");
  modelCarga.setHeaderData("produtos", "Produtos");
  modelCarga.setHeaderData("kg", "Kg.");

  modelCarga.setFilter("0");

  if (not modelCarga.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela cargas: " + modelCarga.lastError().text());
  }

  ui->tableCarga->setModel(&modelCarga);
  ui->tableCarga->hideColumn("idEvento");
  ui->tableCarga->hideColumn("idNFe");
  ui->tableCarga->hideColumn("idVeiculo");
  ui->tableCarga->setItemDelegateForColumn("kg", new DoubleDelegate(this));

  //

  modelProdutos.setTable("view_calendario_produto");
  modelProdutos.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelProdutos.setHeaderData("fornecedor", "Fornecedor");
  modelProdutos.setHeaderData("idVenda", "Venda");
  modelProdutos.setHeaderData("produto", "Produto");
  modelProdutos.setHeaderData("caixas", "Cx.");
  modelProdutos.setHeaderData("kg", "Kg.");
  modelProdutos.setHeaderData("quant", "Quant.");
  modelProdutos.setHeaderData("un", "Un.");

  modelProdutos.setFilter("0");

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela produtos: " + modelProdutos.lastError().text());
  }

  ui->tableProdutos->setModel(&modelProdutos);
  ui->tableProdutos->hideColumn("idEvento");
  ui->tableProdutos->hideColumn("idVendaProduto");
  ui->tableProdutos->setItemDelegateForColumn("kg", new DoubleDelegate(this));
  ui->tableProdutos->setItemDelegateForColumn("quant", new DoubleDelegate(this));
}

void CalendarioEntregas::on_pushButtonReagendar_clicked() {
  const auto list = ui->tableCarga->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  InputDialog input(InputDialog::AgendarEntrega);

  if (input.exec() != InputDialog::Accepted) return;

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not reagendar(list, input.getNextDate())) {
    QSqlQuery("ROLLBACK").exec();
    if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
    return;
  }

  QSqlQuery("COMMIT").exec();

  updateTables();
  QMessageBox::information(this, "Aviso!", "Reagendado com sucesso!");
}

bool CalendarioEntregas::reagendar(const QModelIndexList &list, const QDate &dataPrevEnt) {
  QSqlQuery query;

  for (auto const &item : list) {
    for (int row = 0; row < modelProdutos.rowCount(); ++row) {
      query.prepare("UPDATE venda_has_produto SET dataPrevEnt = :dataPrevEnt WHERE idVendaProduto = :idVendaProduto");
      query.bindValue(":dataPrevEnt", dataPrevEnt);
      query.bindValue(":idVendaProduto", modelProdutos.data(row, "idVendaProduto"));

      if (not query.exec()) {
        error = "Erro atualizando data venda: " + query.lastError().text();
        return false;
      }

      query.prepare("UPDATE pedido_fornecedor_has_produto SET dataPrevEnt = :dataPrevEnt WHERE idVendaProduto = :idVendaProduto");
      query.bindValue(":dataPrevEnt", dataPrevEnt);
      query.bindValue(":idVendaProduto", modelProdutos.data(row, "idVendaProduto"));

      if (not query.exec()) {
        error = "Erro atualizando data pedido_fornecedor: " + query.lastError().text();
        return false;
      }
    }

    query.prepare("UPDATE veiculo_has_produto SET data = :data WHERE idEvento = :idEvento");
    query.bindValue(":data", dataPrevEnt);
    query.bindValue(":idEvento", modelCarga.data(item.row(), "idEvento"));

    if (not query.exec()) {
      error = "Erro atualizando data carga: " + query.lastError().text();
      return false;
    }
  }

  return true;
}

void CalendarioEntregas::on_pushButtonGerarNFeEntregar_clicked() {
  const auto list = ui->tableCarga->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  const QString idVenda = modelCarga.data(list.first().row(), "idVenda").toString();

  QList<int> lista;

  for (int row = 0; row < modelProdutos.rowCount(); ++row) lista.append(modelProdutos.data(row, "idVendaProduto").toInt());

  auto *nfe = new CadastrarNFe(idVenda, this);
  nfe->setAttribute(Qt::WA_DeleteOnClose);
  nfe->prepararNFe(lista);
  nfe->show();
}

void CalendarioEntregas::on_tableCalendario_clicked(const QModelIndex &index) {
  const QString data = modelCalendario.data(index.row(), "data").toString();
  const QString veiculo = modelCalendario.data(index.row(), "idVeiculo").toString();

  modelCarga.setFilter("DATE_FORMAT(dataPrevEnt, '%d-%m-%Y') = '" + data + "' AND idVeiculo = " + veiculo);

  if (not modelCarga.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela entregas: " + modelCarga.lastError().text());
  }

  ui->tableCarga->resizeColumnsToContents();

  modelProdutos.setFilter("0");

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela produtos: " + modelProdutos.lastError().text());
  }

  ui->pushButtonReagendar->setDisabled(true);
  ui->pushButtonConfirmarEntrega->setDisabled(true);
  ui->pushButtonGerarNFeEntregar->setDisabled(true);
  ui->pushButtonImprimirDanfe->setDisabled(true);
  ui->pushButtonCancelarEntrega->setDisabled(true);
}

void CalendarioEntregas::on_tableCarga_clicked(const QModelIndex &index) {
  modelProdutos.setFilter("idVenda = '" + modelCarga.data(index.row(), "idVenda").toString() + "' AND idEvento = " + modelCarga.data(index.row(), "idEvento").toString());

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela produtos: " + modelProdutos.lastError().text());
  }

  ui->tableProdutos->resizeColumnsToContents();

  const QString status = modelCarga.data(index.row(), "status").toString();

  ui->pushButtonReagendar->setEnabled(true);
  ui->pushButtonCancelarEntrega->setEnabled(true);

  if (status == "ENTREGA AGEND.") {
    ui->pushButtonGerarNFeEntregar->setEnabled(true);
    ui->pushButtonConsultarNFe->setDisabled(true);
    ui->pushButtonConfirmarEntrega->setEnabled(true);
    ui->pushButtonImprimirDanfe->setDisabled(true);
  }

  if (status == "EM ENTREGA") {
    ui->pushButtonGerarNFeEntregar->setDisabled(true);
    ui->pushButtonConsultarNFe->setDisabled(true);
    ui->pushButtonConfirmarEntrega->setEnabled(true);
    ui->pushButtonImprimirDanfe->setEnabled(true);
  }

  if (status == "NOTA PENDENTE") {
    ui->pushButtonGerarNFeEntregar->setDisabled(true);
    ui->pushButtonConsultarNFe->setEnabled(true);
    ui->pushButtonConfirmarEntrega->setDisabled(true);
    ui->pushButtonImprimirDanfe->setDisabled(true);
  }
}

bool CalendarioEntregas::confirmarEntrega(const QDateTime &dataRealEnt, const QString &entregou, const QString &recebeu) {
  QSqlQuery query;

  for (int row = 0; row < modelProdutos.rowCount(); ++row) {
    const int idVendaProduto = modelProdutos.data(row, "idVendaProduto").toInt();

    query.prepare("UPDATE veiculo_has_produto SET status = 'ENTREGUE' WHERE status != 'QUEBRADO' AND idVendaProduto = "
                  ":idVendaProduto");
    query.bindValue(":idVendaProduto", idVendaProduto);

    if (not query.exec()) {
      error = "Erro salvando veiculo_has_produto: " + query.lastError().text();
      return false;
    }

    query.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'ENTREGUE', "
                  "dataRealEnt = :dataRealEnt WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":dataRealEnt", dataRealEnt);
    query.bindValue(":idVendaProduto", idVendaProduto);

    if (not query.exec()) {
      error = "Erro salvando pedido_fornecedor: " + query.lastError().text();
      return false;
    }

    query.prepare("UPDATE venda_has_produto SET status = 'ENTREGUE', entregou = :entregou, recebeu = :recebeu, "
                  "dataRealEnt = :dataRealEnt WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":entregou", entregou);
    query.bindValue(":recebeu", recebeu);
    query.bindValue(":dataRealEnt", dataRealEnt);
    query.bindValue(":idVendaProduto", idVendaProduto);

    if (not query.exec()) {
      error = "Erro salvando venda_produto: " + query.lastError().text();
      return false;
    }
  }

  return true;
}

void CalendarioEntregas::on_pushButtonConfirmarEntrega_clicked() {
  const auto list = ui->tableCarga->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  InputDialogConfirmacao inputDlg(InputDialogConfirmacao::Entrega);
  inputDlg.setFilter(modelCarga.data(list.first().row(), "idVenda").toString(), modelCarga.data(list.first().row(), "idEvento").toString());

  if (inputDlg.exec() != InputDialogConfirmacao::Accepted) return;

  const QDateTime dataRealEnt = inputDlg.getDate();
  const QString entregou = inputDlg.getEntregou();
  const QString recebeu = inputDlg.getRecebeu();

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not confirmarEntrega(dataRealEnt, entregou, recebeu)) {
    QSqlQuery("ROLLBACK").exec();
    if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
    return;
  }

  QSqlQuery("COMMIT").exec();

  QSqlQuery query;

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    return;
  }

  updateTables();
  QMessageBox::information(this, "Aviso!", "Entrega confirmada!");
}

void CalendarioEntregas::on_pushButtonImprimirDanfe_clicked() {
  const auto list = ui->tableCarga->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  ACBr::gerarDanfe(modelCarga.data(list.first().row(), "idNFe").toInt());
}

void CalendarioEntregas::on_lineEditBuscar_textChanged(const QString &text) {
  modelCarga.setFilter(text.isEmpty() ? "0" : "idVenda LIKE '%" + text + "%'");

  if (not modelCarga.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela carga: " + modelCarga.lastError().text());
    return;
  }
}

void CalendarioEntregas::on_pushButtonCancelarEntrega_clicked() {
  const auto list = ui->tableCarga->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  QMessageBox msgBox(QMessageBox::Question, "Cancelar?", "Tem certeza que deseja cancelar?", QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Cancelar");
  msgBox.setButtonText(QMessageBox::No, "Voltar");

  if (msgBox.exec() == QMessageBox::No) return;

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not cancelarEntrega(list)) {
    QSqlQuery("ROLLBACK").exec();
    if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
    return;
  }

  QSqlQuery("COMMIT").exec();

  QMessageBox::information(this, "Aviso!", "Operação realizada com sucesso!");
}

bool CalendarioEntregas::cancelarEntrega(const QModelIndexList &list) {
  const int idEvento = modelCarga.data(list.first().row(), "idEvento").toInt();

  QSqlQuery query;
  query.prepare("SELECT idVendaProduto FROM veiculo_has_produto WHERE idEvento = :idEvento");
  query.bindValue(":idEvento", idEvento);

  if (not query.exec()) {
    error = "Erro buscando produtos: " + query.lastError().text();
    return false;
  }

  while (query.next()) {
    QSqlQuery query2;
    query2.prepare("UPDATE venda_has_produto SET status = 'ESTOQUE', dataPrevEnt = NULL WHERE idVendaProduto = :idVendaProduto");
    query2.bindValue(":idVendaProduto", query.value("idVendaProduto"));

    if (not query2.exec()) {
      error = "Erro voltando status produto: " + query2.lastError().text();
      return false;
    }
  }

  query.prepare("DELETE FROM veiculo_has_produto WHERE idEvento = :idEvento");
  query.bindValue(":idEvento", idEvento);

  if (not query.exec()) {
    error = "Erro deletando evento: " + query.lastError().text();
    return false;
  }

  return true;
}

// TODO: 2quando cancelar/devolver um produto cancelar/devolver na logistica/veiculo_has_produto
// TODO: 1refazer sistema para permitir multiplas notas para uma mesma carga/pedido (notas parciais)
// TODO: no filtro de 'parte estoque' nao considerar 'devolvido' e 'cancelado'

void CalendarioEntregas::on_tableCarga_entered(const QModelIndex &) { ui->tableCarga->resizeColumnsToContents(); }

void CalendarioEntregas::on_pushButtonConsultarNFe_clicked() {
  const auto selection = ui->tableCarga->selectionModel()->selectedRows();

  if (selection.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhuma linha selecionada!");
    return;
  }

  const int idNFe = modelCarga.data(selection.first().row(), "idNFe").toInt();

  QSqlQuery query;
  query.prepare("SELECT xml FROM nfe WHERE idNFe = :idNFe");
  query.bindValue(":idNFe", idNFe);

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando XML: " + query.lastError().text());
    return;
  }

  // TODO:__project public code - change this to use current .exe directory
  //  QFile modelo(QDir::currentPath() + "/" + arquivoModelo);
  QFile file("C:/ERP-Staccato/temp.xml");

  if (not file.open(QFile::WriteOnly)) {
    QMessageBox::critical(this, "Erro!", "Erro abrindo arquivo para escrita: " + file.errorString());
    return;
  }

  QTextStream stream(&file);

  stream << query.value("xml").toString();

  file.close();

  QString resposta;

  ACBr::enviarComando("NFE.ConsultarNFe(" + file.fileName() + ")", resposta);

  if (not resposta.contains("XMotivo=Autorizado o uso da NF-e")) {
    QMessageBox::critical(this, "Resposta ConsultarNFe", resposta);
    return;
  }

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not consultarNFe(idNFe)) {
    QSqlQuery("ROLLBACK").exec();
    if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
    return;
  }

  QSqlQuery("COMMIT").exec();
}

bool CalendarioEntregas::consultarNFe(const int idNFe) {
  // alterar status da nota para 'autorizado'

  QSqlQuery query;
  query.prepare("UPDATE nfe SET status = 'AUTORIZADO' WHERE idNFe = :idNFe");
  query.bindValue(":idNFe", idNFe);

  if (not query.exec()) {
    error = "Erro marcando nota como 'AUTORIZADO': " + query.lastError().text();
    return false;
  }

  // alterar status do item/veiculo para 'em entrega'

  for (int row = 0; row < modelProdutos.rowCount(); ++row) {
    query.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'EM ENTREGA' WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":idVendaProduto", modelProdutos.data(row, "idVendaProduto"));

    if (not query.exec()) {
      error = "Erro atualizando status do pedido_fornecedor: " + query.lastError().text();
      return false;
    }

    query.prepare("UPDATE venda_has_produto SET status = 'EM ENTREGA' WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":idVendaProduto", modelProdutos.data(row, "idVendaProduto"));

    if (not query.exec()) {
      error = "Erro salvando NFe nos produtos: " + query.lastError().text();
      return false;
    }

    query.prepare("UPDATE veiculo_has_produto SET status = 'EM ENTREGA' WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":idVendaProduto", modelProdutos.data(row, "idVendaProduto"));

    if (not query.exec()) {
      error = "Erro atualizando carga veiculo: " + query.lastError().text();
      return false;
    }
  }

  return true;
}

// TODO: mudar nome desta classe

void CalendarioEntregas::on_pushButtonTestarProtocolo_clicked() {
  const auto list = ui->tableCarga->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  const QString idVenda = modelCarga.data(list.first().row(), "idVenda").toString();

  QList<int> lista;

  for (int row = 0; row < modelProdutos.rowCount(); ++row) lista.append(modelProdutos.data(row, "idVendaProduto").toInt());

  //-------------------------------------------------------------------------

  if (modelProdutos.rowCount() > 20) {
    QMessageBox::critical(this, "Erro!", "Mais produtos do que cabe no modelo do Excel!");
    return;
  }

  //-------------------------------------------------------------------------

  if (UserSession::settings("User/EntregasPdfFolder").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Não há uma pasta definida para salvar PDF/Excel. Por favor escolha uma.");
    UserSession::setSettings("User/EntregasPdfFolder", QFileDialog::getExistingDirectory(this, "Pasta PDF/Excel"));

    if (UserSession::settings("User/EntregasPdfFolder").toString().isEmpty()) return;
  }

  const QString path = UserSession::settings("User/EntregasPdfFolder").toString();

  QDir dir(path);

  if (not dir.exists() and not dir.mkdir(path)) {
    QMessageBox::critical(this, "Erro!", "Erro ao criar a pasta escolhida nas configurações!");
    return;
  }

  const QString arquivoModelo = "protocolo entrega.xlsx";

  QFile modelo(QDir::currentPath() + "/" + arquivoModelo);

  if (not modelo.exists()) {
    QMessageBox::critical(this, "Erro!", "Não encontrou o modelo do Excel!");
    return;
  }

  const QString fileName = path + "/teste_protocolo_entrega.xlsx";

  QFile file(fileName);

  if (not file.open(QFile::WriteOnly)) {
    QMessageBox::critical(this, "Erro!", "Não foi psossível abrir o arquivo para escrita:\n" + fileName);
    QMessageBox::critical(this, "Erro!", "Erro: " + file.errorString());
    return;
  }

  file.close();

  QXlsx::Document xlsx(arquivoModelo);

  QSqlQuery query;
  query.prepare("SELECT nome_razao, logradouro, numero, complemento, bairro, cidade, cep FROM cliente c LEFT JOIN cliente_has_endereco che ON c.idCliente = che.idCliente WHERE c.idCliente = (SELECT "
                "idCliente FROM venda WHERE idVenda = :idVenda)");
  query.bindValue(":idVenda", idVenda);

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro: " + query.lastError().text());
    return;
  }

  xlsx.write("E8", QDate::currentDate());
  xlsx.write("E10", idVenda);
  xlsx.write("B11", query.value("nome_razao"));
  xlsx.write("B12", query.value("logradouro").toString() + " " + query.value("numero").toString() + " " + query.value("complemento").toString() + " - " + query.value("bairro").toString() + ", " +
                        query.value("cidade").toString());
  xlsx.write("B13", query.value("cep"));
  //  xlsx.write("C40", "NFe: ");

  for (int row = 19, remain = 18 + modelProdutos.rowCount(), index = 0; row <= remain; ++row, ++index) {
    xlsx.write("A" + QString::number(row), index + 1);                                                                                        // item
    xlsx.write("B" + QString::number(row), modelProdutos.data(index, "fornecedor"));                                                          // marca
    xlsx.write("C" + QString::number(row), modelProdutos.data(index, "produto"));                                                             // produto
    xlsx.write("D" + QString::number(row), modelProdutos.data(index, "quant").toString() + " " + modelProdutos.data(index, "un").toString()); // quant
    xlsx.write("E" + QString::number(row), modelProdutos.data(index, "caixas"));                                                              // caixas
    xlsx.write("F" + QString::number(row), "LOTE");                                                                                           // lote
  }

  if (not xlsx.saveAs(fileName)) {
    QMessageBox::critical(this, "Erro!", "Ocorreu algum erro ao salvar o arquivo.");
    return;
  }

  QMessageBox::information(this, "Ok!", "Arquivo salvo como " + fileName);
  QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}
