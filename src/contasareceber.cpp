#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "contasareceber.h"
#include "ui_contasareceber.h"
#include "mainwindow.h"

ContasAReceber::ContasAReceber(QWidget *parent) : QDialog(parent), ui(new Ui::ContasAReceber) {
  ui->setupUi(this);

  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

  modelItensConta.setTable("contaareceber_has_produto");
  modelItensConta.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelItensConta.select()) {
    qDebug() << "Failed to populate modelItensConta: " << modelItensConta.lastError();
  }

  modelContas.setTable("contaareceber");
  modelContas.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelContas.select()) {
    qDebug() << "Failed to populate TableContasAReceber:" << modelContas.lastError();
  }

  ui->tableView->setModel(&modelContas);

  show();
}

ContasAReceber::~ContasAReceber() { delete ui; }

void ContasAReceber::on_checkBox_toggled(bool checked) { Q_UNUSED(checked) }

void ContasAReceber::on_pushButtonSalvar_clicked() {
  if (ui->checkBox->isChecked()) {
    QSqlQuery qry;

    if (not qry.exec("UPDATE contaareceber SET pago = 'SIM' WHERE idVenda = '" + idVenda + "'")) {
      qDebug() << "Erro marcando conta como paga: " << qry.lastError();
    }

  } else {
    QSqlQuery qry;

    if (not qry.exec("UPDATE contaareceber SET pago = 'NÃO' WHERE idVenda = '" + idVenda + "'")) {
      qDebug() << "Erro marcando conta como não paga: " << qry.lastError();
    }
  }

  close();
}

void ContasAReceber::on_pushButtonCancelar_clicked() { close(); }

void ContasAReceber::viewConta(QString idVenda) {
  this->idVenda = idVenda;

  modelItensConta.setFilter("idVenda = '" + idVenda + "'");
  modelContas.setFilter("idVenda = '" + idVenda + "'");

  if (modelContas.data(modelContas.index(0, modelContas.fieldIndex("pago"))).toString() == "SIM") {
    ui->checkBox->setChecked(true);
  }
}
