#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "contasapagar.h"
#include "ui_contasapagar.h"

ContasAPagar::ContasAPagar(QWidget *parent) : QDialog(parent), ui(new Ui::ContasAPagar) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);
  setAttribute(Qt::WA_DeleteOnClose);

  setupTables();

  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

  show();
}

ContasAPagar::~ContasAPagar() { delete ui; }

void ContasAPagar::setupTables() {
  model.setTable("conta_a_pagar_has_pagamento");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela conta_a_pagar_has_pagamento: " + model.lastError().text());
  }

  ui->table->setModel(&model);
  ui->table->resizeColumnsToContents();
}

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

void ContasAPagar::viewConta(const QString &idVenda) {
  this->idVenda = idVenda;

  model.setFilter("idVenda = '" + idVenda + "'");

  if (model.data(0, "pago").toString() == "SIM") ui->checkBoxPago->setChecked(true);
}

void ContasAPagar::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }
