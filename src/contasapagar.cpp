#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

#include "contasapagar.h"
#include "ui_contasapagar.h"

ContasAPagar::ContasAPagar(QWidget *parent) : QDialog(parent), ui(new Ui::ContasAPagar) {
  ui->setupUi(this);

  modelItensContas.setTable("conta_a_pagar_has_produto");
  modelItensContas.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelItensContas.select()) {
    qDebug() << "Failed to populate TableContas:" << modelItensContas.lastError();
    return;
  }

  modelContas.setTable("conta_a_pagar");
  modelContas.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelContas.select()) {
    qDebug() << "Erro carregando tabela conta_a_pagar: " << modelContas.lastError();
    return;
  }

  ui->tableContas->setModel(&modelItensContas);

  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

  show();
}

ContasAPagar::~ContasAPagar() { delete ui; }

void ContasAPagar::on_checkBoxPago_toggled(const bool checked) { Q_UNUSED(checked;) }

void ContasAPagar::on_pushButtonSalvar_clicked() {
  QSqlQuery query;

  if (ui->checkBoxPago->isChecked()) {
    query.prepare("UPDATE conta_a_pagar SET pago = 'SIM' WHERE idVenda = :idVenda");
    query.bindValue(":idVenda", idVenda);

    if (not query.exec()) {
      qDebug() << "Erro ao marcar conta como paga: " << query.lastError();
    }

  } else {
    query.prepare("UPDATE conta_a_pagar SET pago = 'NÃO' WHERE idVenda = :idVenda");
    query.bindValue(":idVenda", idVenda);

    if (not query.exec()) {
      qDebug() << "Erro ao marcar conta como não paga: " << query.lastError();
    }
  }

  close();
}

void ContasAPagar::on_pushButtonCancelar_clicked() {}

void ContasAPagar::viewConta(const QString idVenda) {
  this->idVenda = idVenda;

  modelItensContas.setFilter("idVenda = '" + idVenda + "'");
  modelContas.setFilter("idVenda = '" + idVenda + "'");

  if (modelContas.data(modelContas.index(0, modelContas.fieldIndex("pago"))).toString() == "SIM") {
    ui->checkBoxPago->setChecked(true);
  }
}
