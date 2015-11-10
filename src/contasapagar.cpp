#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "contasapagar.h"
#include "ui_contasapagar.h"

ContasAPagar::ContasAPagar(QWidget *parent) : QDialog(parent), ui(new Ui::ContasAPagar) {
  ui->setupUi(this);

  modelItensContas.setTable("conta_a_pagar_has_pagamento");
  modelItensContas.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelItensContas.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela conta_a_pagar_has_pagamento: " + modelItensContas.lastError().text());
  }

  modelContas.setTable("conta_a_pagar");
  modelContas.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelContas.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela conta_a_pagar: " + modelContas.lastError().text());
  }

  ui->tableContas->setModel(&modelItensContas);
  ui->tableContas->resizeColumnsToContents();

  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

  show();
}

ContasAPagar::~ContasAPagar() { delete ui; }

void ContasAPagar::on_pushButtonSalvar_clicked() {
  QSqlQuery query;
  query.prepare("UPDATE conta_a_pagar SET pago = '" + QString(ui->checkBoxPago->isChecked() ? "SIM" : "NÃO") +
                "' WHERE idVenda = :idVenda");
  query.bindValue(":idVenda", idVenda);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro ao marcar conta: " + query.lastError().text());
    return;
  }

  close();
}

void ContasAPagar::on_pushButtonCancelar_clicked() {}

void ContasAPagar::viewConta(const QString idVenda) {
  this->idVenda = idVenda;

  modelItensContas.setFilter("idVenda = '" + idVenda + "'");
  modelContas.setFilter("idVenda = '" + idVenda + "'");

  if (modelContas.data(0, "pago").toString() == "SIM") {
    ui->checkBoxPago->setChecked(true);
  }
}
