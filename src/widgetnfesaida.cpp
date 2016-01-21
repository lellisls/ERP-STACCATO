#include <QSqlError>

#include "doubledelegate.h"
#include "ui_widgetnfesaida.h"
#include "venda.h"
#include "widgetnfesaida.h"

WidgetNfeSaida::WidgetNfeSaida(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetNfeSaida) { ui->setupUi(this); }

WidgetNfeSaida::~WidgetNfeSaida() { delete ui; }

QString WidgetNfeSaida::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) return "Erro lendo tabela NFe: " + model.lastError().text();

  ui->table->resizeColumnsToContents();

  return QString();
}

void WidgetNfeSaida::setupTables() {
  DoubleDelegate *doubledelegate = new DoubleDelegate(this);

  model.setTable("nfe");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);
  model.setFilter("tipo = 'SAIDA'");

  ui->table->setModel(&model);
  ui->table->hideColumn("xml");
  ui->table->setItemDelegate(doubledelegate);
}

void WidgetNfeSaida::on_table_activated(const QModelIndex &index) {
  Venda *vendas = new Venda(this);
  vendas->viewRegisterById(model.data(index.row(), "idVenda"));
}

void WidgetNfeSaida::on_radioButtonAutorizado_clicked() {
  model.setFilter("status = 'AUTORIZADO'");
  ui->table->resizeColumnsToContents();
}

void WidgetNfeSaida::on_radioButtonEnviado_clicked() {
  model.setFilter("status = 'ENVIADO'");
  ui->table->resizeColumnsToContents();
}

void WidgetNfeSaida::on_radioButtonTodos_clicked() {
  model.setFilter("");
  ui->table->resizeColumnsToContents();
}

void WidgetNfeSaida::on_lineEditBusca_textChanged(const QString &text) {
  model.setFilter(text.isEmpty() ? "" : "(idVenda LIKE '%" + text + "%') OR (status LIKE '%" + text + "%')");
  ui->table->resizeColumnsToContents();
}
