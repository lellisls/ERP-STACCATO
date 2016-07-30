#include <QSqlError>

#include "doubledelegate.h"
#include "entregascliente.h"
#include "orcamentoproxymodel.h"
#include "ui_widgetlogisticaentrega.h"
#include "usersession.h"
#include "widgetlogisticaentrega.h"

WidgetLogisticaEntrega::WidgetLogisticaEntrega(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetLogisticaEntrega) {
  ui->setupUi(this);

  if (UserSession::tipoUsuario() == "VENDEDOR") {
    ui->table->hide();
    ui->labelEntregasCliente->hide();
  }
}

WidgetLogisticaEntrega::~WidgetLogisticaEntrega() { delete ui; }

void WidgetLogisticaEntrega::setupTables() {
  model.setTable("view_venda");
  model.setFilter("statusEntrega = 'ESTOQUE'");

  ui->table->setModel(new OrcamentoProxyModel(&model, this));
  ui->table->setItemDelegate(new DoubleDelegate(this));
  ui->table->hideColumn("statusEntrega");

  if (UserSession::tipoUsuario() != "VENDEDOR ESPECIAL") ui->table->hideColumn("Indicou");

  model2.setTable("venda_has_produto");

  ui->tableView2->setModel(&model2);
}

bool WidgetLogisticaEntrega::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  QString filter = model.filter();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela vendas: " + model.lastError().text());
    return false;
  }

  model.setFilter(filter);

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetLogisticaEntrega::on_radioButtonEntregaLimpar_clicked() {
  model.setFilter("tipo = 'cliente'");
  ui->table->resizeColumnsToContents();
}

void WidgetLogisticaEntrega::on_radioButtonEntregaEnviado_clicked() {
  model.setFilter("status = 'ENVIADO' AND tipo = 'cliente'");
  ui->table->resizeColumnsToContents();
}

void WidgetLogisticaEntrega::on_radioButtonEntregaPendente_clicked() {
  model.setFilter("status = 'PENDENTE' AND tipo = 'cliente'");
  ui->table->resizeColumnsToContents();
}

void WidgetLogisticaEntrega::on_lineEditBuscaEntregas_textChanged(const QString &text) {
  model.setFilter(text.isEmpty() ? "statusEntrega = 'ESTOQUE'" : "statusEntrega = 'ESTOQUE' AND (idPedido LIKE '%" +
                                                                     text + "%') OR (status LIKE '%" + text + "%')");
  ui->table->resizeColumnsToContents();
}

void WidgetLogisticaEntrega::on_table_activated(const QModelIndex &index) {
  EntregasCliente *entregas = new EntregasCliente(this);
  entregas->viewEntrega(model.data(index.row(), "Código").toString());
}

void WidgetLogisticaEntrega::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetLogisticaEntrega::on_table_clicked(const QModelIndex &index) {
  model2.setFilter("idVenda = '" + model.data(index.row(), "Código").toString() + "'");
  model2.select();
}
