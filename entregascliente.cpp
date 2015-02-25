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
  if (!modelEntregas.select()) {
    qDebug() << "Failed to populate TableEntregas!" << modelEntregas.lastError();
  }

  ui->tableEntregasCliente->setModel(&modelEntregas);

  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

  show();
}

EntregasCliente::~EntregasCliente() {
  delete ui;
}

void EntregasCliente::on_pushButtonSalvar_clicked() {

  if (ui->checkBoxEntregue->isChecked()) {
    QSqlQuery qry;
    if (!qry.exec("UPDATE pedidotransportadora SET status = 'ENTREGUE' WHERE idPedido = '" + idPedido +
                  "' AND tipo = 'cliente'")) {
      qDebug() << "Erro ao marcar como entregue: " << qry.lastError();
    }

    if (!qry.exec("UPDATE venda SET status = 'FECHADO' WHERE idVenda = '" + idPedido + "'")) {
      qDebug() << "Erro ao concluir a venda:" << qry.lastError();
    }
  } else {
    QSqlQuery qry;
    if (!qry.exec("UPDATE pedidotransportadora SET status = 'PENDENTE' WHERE idPedido = '" + idPedido +
                  "' AND tipo = 'cliente'")) {
      qDebug() << "Erro ao marcar como não entregue: " << qry.lastError();
    }

    if(!qry.exec("UPDATE venda set status = 'ABERTO' WHERE idVenda = '"+ idPedido +"'")) {
      qDebug() << "Erro ao marcar venda como não concluída: " << qry.lastError();
    }
  }

  if (MainWindow *window = qobject_cast<MainWindow *>(parentWidget())) {
    window->updateTables();
  }

  close();
}

void EntregasCliente::on_pushButtonCancelar_clicked() {
  close();
}

void EntregasCliente::on_checkBoxEntregue_clicked() {}

void EntregasCliente::viewEntrega(QString idPedido) {
  this->idPedido = idPedido;
  modelEntregas.setFilter("idPedido = '" + idPedido + "' AND tipo = 'cliente'");
}
