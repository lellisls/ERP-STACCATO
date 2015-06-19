#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "entregascliente.h"
#include "mainwindow.h"
#include "ui_entregascliente.h"

EntregasCliente::EntregasCliente(QWidget *parent) : QDialog(parent), ui(new Ui::EntregasCliente) {
  ui->setupUi(this);

  modelEntregas.setTable("pedidotransportadora");
  modelEntregas.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelEntregas.select()) {
    qDebug() << "Failed to populate TableEntregas:" << modelEntregas.lastError();
    return;
  }

  ui->tableEntregasCliente->setModel(&modelEntregas);

  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

  show();
}

EntregasCliente::~EntregasCliente() { delete ui; }

void EntregasCliente::on_pushButtonSalvar_clicked() {
  QSqlQuery query;

  if (ui->checkBoxEntregue->isChecked()) {
    query.prepare(
          "UPDATE PedidoTransportadora SET status = 'ENTREGUE' WHERE idPedido = :idPedido AND tipo = 'cliente'");
    query.bindValue(":idPedido", idPedido);

    if (not query.exec()) {
      qDebug() << "Erro ao marcar como entregue: " << query.lastError();
    }

    query.prepare("UPDATE Venda SET status = 'FECHADO' WHERE idVenda = :idPedido");
    query.bindValue(":idPedido", idPedido);

    if (not query.exec()) {
      qDebug() << "Erro ao concluir a venda:" << query.lastError();
    }

  } else {
    query.prepare(
          "UPDATE PedidoTransportadora SET status = 'PENDENTE' WHERE idPedido = :idPedido AND tipo = 'cliente'");
    query.bindValue(":idPedido", idPedido);

    if (not query.exec()) {
      qDebug() << "Erro ao marcar como não entregue: " << query.lastError();
    }

    query.prepare("UPDATE Venda SET status = 'ABERTO' WHERE idVenda = :idVenda");
    query.bindValue("idVenda", idPedido);

    if (not query.exec()) {
      qDebug() << "Erro ao marcar venda como não concluída: " << query.lastError();
    }
  }

  close();
}

void EntregasCliente::on_pushButtonCancelar_clicked() { close(); }

void EntregasCliente::on_checkBoxEntregue_clicked() {}

void EntregasCliente::viewEntrega(QString idPedido) {
  this->idPedido = idPedido;
  modelEntregas.setFilter("idPedido = '" + idPedido + "' AND tipo = 'cliente'");
}
