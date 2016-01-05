#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "comboboxdelegate.h"
#include "contasareceber.h"
#include "doubledelegate.h"
#include "ui_contasareceber.h"

ContasAReceber::ContasAReceber(QWidget *parent) : QDialog(parent), ui(new Ui::ContasAReceber) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);
  setAttribute(Qt::WA_DeleteOnClose);

  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

  modelContas.setTable("conta_a_receber_has_pagamento");
  modelContas.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelContas.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela conta_a_receber_has_pagamento: " + modelContas.lastError().text());
  }

  ui->tableView->setModel(&modelContas);
  ui->tableView->setItemDelegate(new DoubleDelegate(this));
  ui->tableView->setItemDelegateForColumn(modelContas.fieldIndex("status"), new ComboBoxDelegate(this));
  ui->tableView->hideColumn(modelContas.fieldIndex("idPagamento"));
  ui->tableView->hideColumn(modelContas.fieldIndex("idVenda"));
  ui->tableView->hideColumn(modelContas.fieldIndex("idLoja"));

  show();
}

ContasAReceber::~ContasAReceber() { delete ui; }

void ContasAReceber::on_pushButtonSalvar_clicked() {
  close();
}

void ContasAReceber::on_pushButtonCancelar_clicked() { close(); }

void ContasAReceber::viewConta(const QString &idVenda) {
  this->idVenda = idVenda;

  modelContas.setFilter("idVenda = '" + idVenda + "'");

  ui->checkBox->setChecked(modelContas.data(0, "pago").toString() == "SIM" ? true : false);

  ui->tableView->resizeColumnsToContents();

  for (int row = 0, rowCount = modelContas.rowCount(); row < rowCount; ++row) {
    ui->tableView->openPersistentEditor(modelContas.index(row, modelContas.fieldIndex("status")));
  }
}
