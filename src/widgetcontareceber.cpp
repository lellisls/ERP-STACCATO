#include <QSqlError>

#include "doubledelegate.h"
#include "ui_widgetcontareceber.h"
#include "widgetcontareceber.h"

WidgetContaReceber::WidgetContaReceber(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetContaReceber) {
  ui->setupUi(this);
}

WidgetContaReceber::~WidgetContaReceber() { delete ui; }

void WidgetContaReceber::setupTables() {
  model.setTable("conta_a_receber_has_pagamento");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  ui->table->setModel(&model);
  ui->table->setItemDelegate(new DoubleDelegate(this));
  ui->table->hideColumn("idPagamento");
}

QString WidgetContaReceber::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) return "Erro lendo tabela conta_a_receber_has_pagamento: " + model.lastError().text();

  ui->table->resizeColumnsToContents();

  return QString();
}
