#include <QSqlError>

#include "doubledelegate.h"
#include "ui_widgetcontapagar.h"
#include "usersession.h"
#include "widgetcontapagar.h"

WidgetContaPagar::WidgetContaPagar(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetContaPagar) {
  ui->setupUi(this);
}

WidgetContaPagar::~WidgetContaPagar() { delete ui; }

void WidgetContaPagar::setupTables() {
  model.setTable("conta_a_pagar_has_pagamento");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  ui->table->setModel(&model);
  ui->table->setItemDelegate(new DoubleDelegate(this));
  ui->table->hideColumn("idPagamento");
}

QString WidgetContaPagar::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) return "Erro lendo tabela conta_a_pagar_has_pagamento: " + model.lastError().text();

  ui->table->resizeColumnsToContents();

  return QString();
}
