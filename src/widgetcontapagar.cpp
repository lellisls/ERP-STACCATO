#include <QSqlError>

#include "contasapagar.h"
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

bool WidgetContaPagar::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  QString filter = model.filter();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela conta_a_pagar_has_pagamento: " + model.lastError().text());
    return false;
  }

  model.setFilter(filter);

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetContaPagar::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetContaPagar::on_table_activated(const QModelIndex &index) {
  //  ContasAPagar *contas = new ContasAPagar(this);
  //  contas->viewConta(model.data(index.row(), "idVenda").toString());
}

// NOTE: tela para incluir lancamentos
