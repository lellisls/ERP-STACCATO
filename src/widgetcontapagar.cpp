#include "widgetcontapagar.h"
#include "doubledelegate.h"
#include "ui_widgetcontapagar.h"
#include "usersession.h"

#include <QSqlError>

WidgetContaPagar::WidgetContaPagar(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetContaPagar) {
  ui->setupUi(this);

  setupTables();
}

WidgetContaPagar::~WidgetContaPagar() { delete ui; }

void WidgetContaPagar::setupTables() {
  DoubleDelegate *doubledelegate = new DoubleDelegate(this);

  model.setTable("conta_a_pagar_has_pagamento");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  ui->tableContasPagar->setModel(&model);
  ui->tableContasPagar->setItemDelegate(doubledelegate);
  ui->tableContasPagar->hideColumn("idPagamento");
}

QString WidgetContaPagar::updateTables() {
  if (not model.select()) {
    return "Erro lendo tabela conta_a_pagar_has_pagamento: " + model.lastError().text();
  }

  ui->tableContasPagar->resizeColumnsToContents();

  return QString();
}
