#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

#include "comboboxdelegate.h"
#include "contasareceber.h"
#include "doubledelegate.h"
#include "ui_contasareceber.h"

ContasAReceber::ContasAReceber(QWidget *parent) : QDialog(parent), ui(new Ui::ContasAReceber) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

  modelContas.setTable("conta_a_receber_has_pagamento");
  modelContas.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelContas.select()) {
    qDebug() << "Failed to populate TableContasAReceber:" << modelContas.lastError();
    return;
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

void ContasAReceber::on_checkBox_toggled(const bool checked) { Q_UNUSED(checked) }

void ContasAReceber::on_pushButtonSalvar_clicked() {
  QSqlQuery query;

  if (ui->checkBox->isChecked()) {
    query.prepare("UPDATE conta_a_receber SET pago = 'SIM' WHERE idVenda = :idVenda");
    query.bindValue(":idVenda", idVenda);

    if (not query.exec()) {
      qDebug() << "Erro marcando conta como paga: " << query.lastError();
    }

  } else {
    query.prepare("UPDATE conta_a_receber SET pago = 'NÃO' WHERE idVenda = :idVenda");
    query.bindValue(":idVenda", idVenda);

    if (not query.exec()) {
      qDebug() << "Erro marcando conta como não paga: " << query.lastError();
    }
  }

  close();
}

void ContasAReceber::on_pushButtonCancelar_clicked() { close(); }

void ContasAReceber::viewConta(const QString idVenda) {
  this->idVenda = idVenda;

  modelContas.setFilter("idVenda = '" + idVenda + "'");

  if(modelContas.data(0, "pago").toString() == "SIM"){
    ui->checkBox->setChecked(true);
  }

  ui->tableView->resizeColumnsToContents();

  for (int row = 0, rowCount = modelContas.rowCount(); row < rowCount; ++row) {
    ui->tableView->openPersistentEditor(modelContas.index(row, modelContas.fieldIndex("status")));
  }
}
