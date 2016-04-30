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

bool WidgetContaReceber::updateTables(QString &error) {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) {
    error = "Erro lendo tabela conta_a_receber_has_pagamento: " + model.lastError().text();
    return false;
  }

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetContaReceber::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }
