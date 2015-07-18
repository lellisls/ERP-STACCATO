#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

#include "recebimentosfornecedor.h"
#include "ui_recebimentosfornecedor.h"

RecebimentosFornecedor::RecebimentosFornecedor(QWidget *parent) : QDialog(parent), ui(new Ui::RecebimentosFornecedor) {
  ui->setupUi(this);

  modelRecebimentos.setTable("pedido_transportadora");
  modelRecebimentos.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelRecebimentos.select()) {
    qDebug() << "Failed to populate TableRecebimentos: " << modelRecebimentos.lastError();
    return;
  }

  ui->tableRecebimentosForncecedor->setModel(&modelRecebimentos);
  ui->tableRecebimentosForncecedor->horizontalHeader()->setResizeContentsPrecision(0);
  ui->tableRecebimentosForncecedor->verticalHeader()->setResizeContentsPrecision(0);

  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

  show();

  ui->tableRecebimentosForncecedor->resizeColumnsToContents();
}

RecebimentosFornecedor::~RecebimentosFornecedor() { delete ui; }

void RecebimentosFornecedor::on_pushButtonSalvar_clicked() {
  QSqlQuery query;

  if (ui->checkBoxEntregue->isChecked()) {
    query.prepare(
          "UPDATE pedido_transportadora SET status = 'RECEBIDO' WHERE idPedido = :idPedido AND tipo = 'fornecedor'");
    query.bindValue("idPedido", idPedido);

    if (not query.exec()) {
      qDebug() << "Erro ao marcar como recebido: " << query.lastError();
    }
  } else {
    query.prepare(
          "UPDATE pedido_transportadora SET status = 'PENDENTE' WHERE idPedido = :idPedido AND tipo = 'fornecedor'");
    query.bindValue("idPedido", idPedido);

    if (not query.exec()) {
      qDebug() << "Erro ao marcar como não recebido: " << query.lastError();
    }
  }

  close();
}

void RecebimentosFornecedor::on_pushButtonCancelar_clicked() { close(); }

void RecebimentosFornecedor::viewRecebimento(const QString idPedido) {
  this->idPedido = idPedido;
  modelRecebimentos.setFilter("idPedido = '" + idPedido + "' AND tipo = 'fornecedor'");
}
