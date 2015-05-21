#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "contasapagar.h"
#include "mainwindow.h"
#include "ui_contasapagar.h"

ContasAPagar::ContasAPagar(QWidget *parent) : QDialog(parent), ui(new Ui::ContasAPagar) {
  ui->setupUi(this);

  //  modelItensContas = new QSqlRelationalTableModel(this);
  modelItensContas.setTable("contaapagar_has_produto");
  modelItensContas.setEditStrategy(QSqlTableModel::OnManualSubmit);
  if (not modelItensContas.select()) {
    qDebug() << "Failed to populate TableContas!" << modelItensContas.lastError();
  }

  modelContas.setTable("contaapagar");
  modelContas.setEditStrategy(QSqlTableModel::OnManualSubmit);
  if (not modelContas.select()) {
    qDebug() << "Erro carregando tabela contaapagar: " << modelContas.lastError();
  }

  ui->tableContas->setModel(&modelItensContas);

  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

  show();
}

ContasAPagar::~ContasAPagar() { delete ui; }

void ContasAPagar::on_checkBoxPago_toggled(bool checked) { Q_UNUSED(checked;) }

void ContasAPagar::on_pushButtonSalvar_clicked() {
  if (ui->checkBoxPago->isChecked()) {
    QSqlQuery qry;
    if (not qry.exec("UPDATE contaapagar SET pago = 'SIM' WHERE idVenda = '" + idVenda + "'")) {
      qDebug() << "Erro ao marcar conta como paga: " << qry.lastError();
    }
  } else {
    QSqlQuery qry;
    if (not qry.exec("UPDATE contaapagar SET pago = 'NÃO' WHERE idVenda = '" + idVenda + "'")) {
      qDebug() << "Erro ao marcar conta como não paga: " << qry.lastError();
    }
  }

  if (MainWindow *window = qobject_cast<MainWindow *>(parentWidget())) {
    window->updateTables();
  }

  close();
}

void ContasAPagar::on_pushButtonCancelar_clicked() {}

void ContasAPagar::viewConta(QString idVenda) {
  this->idVenda = idVenda;

  modelItensContas.setFilter("idVenda = '" + idVenda + "'");
  modelContas.setFilter("idVenda = '" + idVenda + "'");

  if (modelContas.data(modelContas.index(0, modelContas.fieldIndex("pago"))).toString() == "SIM") {
    ui->checkBoxPago->setChecked(true);
  }
}
