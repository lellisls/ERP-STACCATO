#include <QSqlError>

#include "widgetlogisticaentrega.h"
#include "ui_widgetlogisticaentrega.h"
#include "doubledelegate.h"
#include "usersession.h"
#include "entregascliente.h"

WidgetLogisticaEntrega::WidgetLogisticaEntrega(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetLogisticaEntrega) {
  ui->setupUi(this);

  if (UserSession::getTipoUsuario() == "VENDEDOR") {
    ui->tableEntregasCliente->hide();
    ui->labelEntregasCliente->hide();
  }
}

WidgetLogisticaEntrega::~WidgetLogisticaEntrega() { delete ui; }

void WidgetLogisticaEntrega::setupTables() {
  modelEntregasCliente.setTable("view_venda");

  ui->tableEntregasCliente->setModel(&modelEntregasCliente);
  ui->tableEntregasCliente->setItemDelegate(new DoubleDelegate(this));
}

QString WidgetLogisticaEntrega::updateTables() {
  if (not modelEntregasCliente.select()) {
    return "Erro lendo tabela vendas: " + modelEntregasCliente.lastError().text();
  }

  ui->tableEntregasCliente->resizeColumnsToContents();

  return QString();
}

void WidgetLogisticaEntrega::on_radioButtonEntregaLimpar_clicked() {
  modelEntregasCliente.setFilter("tipo = 'cliente'");
  ui->tableEntregasCliente->resizeColumnsToContents();
}

void WidgetLogisticaEntrega::on_radioButtonEntregaEnviado_clicked() {
  modelEntregasCliente.setFilter("status = 'enviado' AND tipo = 'cliente'");
  ui->tableEntregasCliente->resizeColumnsToContents();
}

void WidgetLogisticaEntrega::on_radioButtonEntregaPendente_clicked() {
  modelEntregasCliente.setFilter("status = 'pendente' AND tipo = 'cliente'");
  ui->tableEntregasCliente->resizeColumnsToContents();
}

void WidgetLogisticaEntrega::on_lineEditBuscaEntregas_textChanged(const QString &text) {
  modelEntregasCliente.setFilter(text.isEmpty() ? ""
                                                : "(idPedido LIKE '%" + text + "%') OR (status LIKE '%" + text + "%')");

  ui->tableEntregasCliente->resizeColumnsToContents();
}

void WidgetLogisticaEntrega::on_tableEntregasCliente_activated(const QModelIndex &index) {
  EntregasCliente *entregas = new EntregasCliente(this);
  entregas->viewEntrega(modelEntregasCliente.data(index.row(), "Código").toString());
}
