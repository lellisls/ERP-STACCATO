#include <QSqlError>
#include <QSqlQuery>

#include "contasareceber.h"
#include "doubledelegate.h"
#include "ui_widgetcontareceber.h"
#include "widgetcontareceber.h"

WidgetContaReceber::WidgetContaReceber(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetContaReceber) {
  ui->setupUi(this);

  //  QSqlQuery query("SELECT descricao, idLoja FROM loja WHERE desativado = FALSE");

  //  while (query.next()) {
  //    ui->comboBoxLojas->addItem(query.value("descricao").toString(), query.value("idLoja"));
  //  }
}

WidgetContaReceber::~WidgetContaReceber() { delete ui; }

void WidgetContaReceber::setupTables() {
  model.setTable("conta_a_receber_has_pagamento");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  ui->table->setModel(&model);
  ui->table->setItemDelegate(new DoubleDelegate(this));
  ui->table->hideColumn("idPagamento");
}

bool WidgetContaReceber::updateTables() {
  if (model.tableName().isEmpty()) {
    QSqlQuery query("SELECT descricao, idLoja FROM loja WHERE desativado = FALSE");

    while (query.next()) {
      ui->comboBoxLojas->addItem(query.value("descricao").toString(), query.value("idLoja"));
    }

    setupTables();
  }

  QString filter = model.filter();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela conta_a_receber_has_pagamento: " + model.lastError().text());
    return false;
  }

  model.setFilter(filter);

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetContaReceber::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetContaReceber::on_table_activated(const QModelIndex &index) {
  ContasAReceber *contas = new ContasAReceber(this);
  contas->viewConta(model.data(index.row(), "idVenda").toString());
}

void WidgetContaReceber::on_radioButtonContaReceberRecebido_toggled(bool checked) {
  if (checked) model.setFilter("status = 'RECEBIDO'");
}

void WidgetContaReceber::on_radioButtonContaReceberPendente_toggled(bool checked) {
  if (checked) model.setFilter("status = 'PENDENTE'");
}

void WidgetContaReceber::on_radioButtonContaReceberLimpar_toggled(bool checked) {
  if (checked) model.setFilter("");
}

// TODO: filtrar por mes (copiar do orcamento)
// TODO: resumo do dia e dos proximos dias
// TODO: resumo do mes
// TODO: tela para incluir lancamentos
