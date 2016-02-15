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

  setupTables();

  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

  show();
}

ContasAReceber::~ContasAReceber() { delete ui; }

void ContasAReceber::setupTables() {
  model.setTable("conta_a_receber_has_pagamento");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela conta_a_receber_has_pagamento: " + model.lastError().text());
  }

  ui->table->setModel(&model);
  ui->table->setItemDelegate(new DoubleDelegate(this));
  ui->table->setItemDelegateForColumn(model.fieldIndex("status"), new ComboBoxDelegate(this));
  ui->table->hideColumn(model.fieldIndex("idPagamento"));
  ui->table->hideColumn(model.fieldIndex("idVenda"));
  ui->table->hideColumn(model.fieldIndex("idLoja"));
}

void ContasAReceber::on_pushButtonSalvar_clicked() { close(); }

void ContasAReceber::viewConta(const QString &idVenda) {
  this->idVenda = idVenda;

  model.setFilter("idVenda = '" + idVenda + "'");

  ui->checkBox->setChecked(model.data(0, "pago").toString() == "SIM" ? true : false);

  ui->table->resizeColumnsToContents();

  for (int row = 0, rowCount = model.rowCount(); row < rowCount; ++row) {
    ui->table->openPersistentEditor(model.index(row, model.fieldIndex("status")));
  }
}

void ContasAReceber::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }
