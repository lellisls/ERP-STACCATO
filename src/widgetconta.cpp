#include "src/widgetconta.h"
#include "contasapagar.h"
#include "contasareceber.h"
#include "doubledelegate.h"
#include "ui_widgetconta.h"
#include "usersession.h"

#include <QMessageBox>
#include <QSqlError>

WidgetConta::WidgetConta(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetConta) {
  ui->setupUi(this);

  setupTables();

  if (UserSession::getTipoUsuario() == "VENDEDOR") {
    ui->tableContasPagar->hide();
    ui->labelContasPagar->hide();
    ui->tableContasReceber->hide();
    ui->labelContasReceber->hide();
  }
}

WidgetConta::~WidgetConta() { delete ui; }

void WidgetConta::setupTables() {
  DoubleDelegate *doubledelegate = new DoubleDelegate(this);

  // Contas a pagar ----------------------------------------------------------------------------------------------------
  modelCAPagar.setTable("conta_a_pagar");
  modelCAPagar.setEditStrategy(QSqlTableModel::OnManualSubmit);

  ui->tableContasPagar->setModel(&modelCAPagar);
  ui->tableContasPagar->setItemDelegate(doubledelegate);

  // Contas a receber --------------------------------------------------------------------------------------------------
  modelCAReceber.setTable("conta_a_receber");
  modelCAReceber.setEditStrategy(QSqlTableModel::OnManualSubmit);

  ui->tableContasReceber->setModel(&modelCAReceber);
  ui->tableContasReceber->setItemDelegate(doubledelegate);
}

bool WidgetConta::updateTables() {
  if (not modelCAPagar.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela conta_a_pagar: " + modelCAPagar.lastError().text());
    return false;
  }

  if (not modelCAReceber.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela conta_a_receber: " + modelCAReceber.lastError().text());
    return false;
  }

  return true;
}

void WidgetConta::on_radioButtonContaPagarLimpar_clicked() {
  modelCAPagar.setFilter("");
  ui->tableContasPagar->resizeColumnsToContents();
}

void WidgetConta::on_radioButtonContaPagarPago_clicked() {
  modelCAPagar.setFilter("pago = 'sim'");
  ui->tableContasPagar->resizeColumnsToContents();
}

void WidgetConta::on_radioButtonContaPagarPendente_clicked() {
  modelCAPagar.setFilter("pago = 'não'");
  ui->tableContasPagar->resizeColumnsToContents();
}

void WidgetConta::on_radioButtonContaReceberLimpar_clicked() {
  modelCAReceber.setFilter("");
  ui->tableContasReceber->resizeColumnsToContents();
}

void WidgetConta::on_radioButtonContaReceberRecebido_clicked() {
  modelCAReceber.setFilter("pago = 'sim'");
  ui->tableContasReceber->resizeColumnsToContents();
}

void WidgetConta::on_radioButtonContaReceberPendente_clicked() {
  modelCAReceber.setFilter("pago = 'não'");
  ui->tableContasReceber->resizeColumnsToContents();
}

void WidgetConta::on_tableContasPagar_activated(const QModelIndex &index) {
  ContasAPagar *contas = new ContasAPagar(this);
  contas->viewConta(modelCAPagar.data(index.row(), "idVenda").toString());
}

void WidgetConta::on_tableContasReceber_activated(const QModelIndex &index) {
  ContasAReceber *contas = new ContasAReceber(this);
  contas->viewConta(modelCAReceber.data(index.row(), "idVenda").toString());
}

void WidgetConta::on_lineEditBuscaContasPagar_textChanged(const QString &text) {
  modelCAPagar.setFilter(text.isEmpty() ? "" : "(idVenda LIKE '%" + text + "%') OR (pago LIKE '%" + text + "%')");

  ui->tableContasPagar->resizeColumnsToContents();
}

void WidgetConta::on_lineEditBuscaContasReceber_textChanged(const QString &text) {
  modelCAReceber.setFilter(text.isEmpty() ? "" : "(idVenda LIKE '%" + text + "%') OR (pago LIKE '%" + text + "%')");

  ui->tableContasReceber->resizeColumnsToContents();
}
