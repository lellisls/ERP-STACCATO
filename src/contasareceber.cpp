#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "comboboxdelegate.h"
#include "contasareceber.h"
#include "doubledelegate.h"
#include "ui_contasareceber.h"

ContasAReceber::ContasAReceber(QWidget *parent) : QDialog(parent), ui(new Ui::ContasAReceber) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);
  setAttribute(Qt::WA_DeleteOnClose);

  setupTables();

  showMaximized();
}

ContasAReceber::~ContasAReceber() { delete ui; }

void ContasAReceber::setupTables() {
  model.setTable("conta_a_receber_has_pagamento");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela conta_a_receber_has_pagamento: " + model.lastError().text());
  }

  ui->table->setModel(&model);
  ui->table->setItemDelegate(new DoubleDelegate(this));
  ui->table->setItemDelegateForColumn(model.fieldIndex("status"),
                                      new ComboBoxDelegate(ComboBoxDelegate::StatusReceber, this));
  ui->table->setItemDelegateForColumn(model.fieldIndex("contaDestino"),
                                      new ComboBoxDelegate(ComboBoxDelegate::Conta, this));
  ui->table->hideColumn(model.fieldIndex("idPagamento"));
  ui->table->hideColumn(model.fieldIndex("idVenda"));
  ui->table->hideColumn(model.fieldIndex("idLoja"));
  ui->table->hideColumn(model.fieldIndex("created"));
  ui->table->hideColumn(model.fieldIndex("lastUpdated"));
}

void ContasAReceber::on_pushButtonSalvar_clicked() {
  // TODO: verificar consistencia dos dados antes de salvar

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados: " + model.lastError().text());
    return;
  }

  close();
}

void ContasAReceber::viewConta(const QString &idVenda) {
  this->idVenda = idVenda;

  setWindowTitle(windowTitle() + " - " + idVenda);

  model.setFilter("idVenda = '" + idVenda + "'");

  ui->table->resizeColumnsToContents();

  for (int row = 0, rowCount = model.rowCount(); row < rowCount; ++row) {
    ui->table->openPersistentEditor(model.index(row, model.fieldIndex("status")));
    ui->table->openPersistentEditor(model.index(row, model.fieldIndex("contaDestino")));
  }
}

void ContasAReceber::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

// TODO: colocar coluna para indicar quem paga (seja cliente ou fornecedor rep.)
// TODO: criar tela para dar baixa
