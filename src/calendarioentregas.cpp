#include <QDateTime>
#include <QDesktopServices>
#include <QFile>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSqlError>
#include <QSqlQuery>
#include <QUrl>

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

  ui->pushButtonGerarNFeEntregar->setEnabled(true);
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

bool CalendarioEntregas::reagendar(const QModelIndexList &list, const QDateTime &dataPrevEnt) {
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

      query.prepare(
          "UPDATE pedido_fornecedor_has_produto SET dataPrevEnt = :dataPrevEnt WHERE idVendaProduto = :idVendaProduto");
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

  for (int row = 0; row < modelProdutos.rowCount(); ++row) {
    lista.append(modelProdutos.data(row, "idVendaProduto").toInt());
  }

  CadastrarNFe *nfe = new CadastrarNFe(idVenda, this);
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
  //  ui->pushButtonGerarNFeEntregar->setDisabled(true);
  ui->pushButtonImprimirDanfe->setDisabled(true);
}

void CalendarioEntregas::on_tableCarga_clicked(const QModelIndex &index) {
  modelProdutos.setFilter("idVenda = '" + modelCarga.data(index.row(), "idVenda").toString() + "' AND idEvento = " +
                          modelCarga.data(index.row(), "idEvento").toString());

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela produtos: " + modelProdutos.lastError().text());
  }

  ui->tableProdutos->resizeColumnsToContents();

  const QString status = modelCarga.data(index.row(), "status").toString();

  ui->pushButtonReagendar->setEnabled(true);

  if (status == "ENTREGA AGEND.") {
    ui->pushButtonGerarNFeEntregar->setEnabled(true);
    ui->pushButtonConfirmarEntrega->setEnabled(true);
    ui->pushButtonImprimirDanfe->setDisabled(true);
  }

  if (status == "EM ENTREGA") {
    //    ui->pushButtonGerarNFeEntregar->setDisabled(true);
    ui->pushButtonConfirmarEntrega->setEnabled(true);
    ui->pushButtonImprimirDanfe->setEnabled(true);
  }
}

bool CalendarioEntregas::confirmarEntrega(const QDateTime &dataRealEnt, const QString &entregou,
                                          const QString &recebeu) {
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

    //    query.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'ENTREGUE', "
    //                  "dataRealEnt = :dataRealEnt WHERE idVendaProduto = :idVendaProduto");
    //    query.bindValue(":dataRealEnt", dataRealEnt);
    //    query.bindValue(":idVendaProduto", idVendaProduto);

    //    if (not query.exec()) {
    //      error = "Erro salvando pedido_fornecedor: " + query.lastError().text();
    //      return false;
    //    }

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
  inputDlg.setFilter(modelCarga.data(list.first().row(), "idVenda").toString(),
                     modelCarga.data(list.first().row(), "idEvento").toString());

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
    query2.prepare(
        "UPDATE venda_has_produto SET status = 'ESTOQUE', dataPrevEnt = NULL WHERE idVendaProduto = :idVendaProduto");
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
// TODO: 1avisar gerar nfe quando linha ja possuir nfe
// TODO: 1refazer sistema para permitir multiplas notas para uma mesma carga/pedido (notas parciais)
