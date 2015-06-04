#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

#include "mainwindow.h"
#include "recebimentosfornecedor.h"
#include "ui_recebimentosfornecedor.h"

RecebimentosFornecedor::RecebimentosFornecedor(QWidget *parent) : QDialog(parent), ui(new Ui::RecebimentosFornecedor) {
  ui->setupUi(this);

  modelRecebimentos.setTable("pedidotransportadora");
  modelRecebimentos.setEditStrategy(QSqlTableModel::OnManualSubmit);
  if (not modelRecebimentos.select()) {
    qDebug() << "Failed to populate TableRecebimentos: " << modelRecebimentos.lastError();
  }

  ui->tableRecebimentosForncecedor->setModel(&modelRecebimentos);

  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

  show();
}

RecebimentosFornecedor::~RecebimentosFornecedor() { delete ui; }

void RecebimentosFornecedor::on_pushButtonSalvar_clicked() {
  if (ui->checkBoxEntregue->isChecked()) {
    QSqlQuery qry;
    if (not qry.exec("UPDATE pedidotransportadora SET status = 'RECEBIDO' WHERE idPedido = '" + idPedido +
                     "' AND tipo = 'fornecedor'")) {
      qDebug() << "Erro ao marcar como recebido: " << qry.lastError();
    }
  } else {
    QSqlQuery qry;
    if (not qry.exec("UPDATE pedidotransportadora SET status = 'PENDENTE' WHERE idPedido = '" + idPedido +
                     "' AND tipo = 'fornecedor'")) {
      qDebug() << "Erro ao marcar como não recebido: " << qry.lastError();
    }
  }

  if (MainWindow *window = qobject_cast<MainWindow *>(parentWidget())) {
    window->updateTables();
  }

  close();
}

void RecebimentosFornecedor::on_pushButtonCancelar_clicked() { close(); }

void RecebimentosFornecedor::viewRecebimento(QString idPedido) {
  this->idPedido = idPedido;
  modelRecebimentos.setFilter("idPedido = '" + idPedido + "' AND tipo = 'fornecedor'");
}
