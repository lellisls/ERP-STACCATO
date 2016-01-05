#include "widgetcontareceber.h"
#include "doubledelegate.h"
#include "ui_widgetcontareceber.h"

#include <QSqlError>

WidgetContaReceber::WidgetContaReceber(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetContaReceber) {
  ui->setupUi(this);

  setupTables();
}

WidgetContaReceber::~WidgetContaReceber() { delete ui; }

void WidgetContaReceber::setupTables() {
  DoubleDelegate *doubledelegate = new DoubleDelegate(this);

  model.setTable("conta_a_receber_has_pagamento");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  ui->tableContasReceber->setModel(&model);
  ui->tableContasReceber->setItemDelegate(doubledelegate);
  ui->tableContasReceber->hideColumn("idPagamento");
}

QString WidgetContaReceber::updateTables() {
  if (not model.select()) {
    return "Erro lendo tabela conta_a_receber_has_pagamento: " + model.lastError().text();
  }

  ui->tableContasReceber->resizeColumnsToContents();

  return QString();
}
