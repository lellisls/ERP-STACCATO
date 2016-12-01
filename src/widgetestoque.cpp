#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>

#include "doubledelegate.h"
#include "estoque.h"
#include "importarxml.h"
#include "ui_widgetestoque.h"
#include "widgetestoque.h"
#include "xml.h"

WidgetEstoque::WidgetEstoque(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetEstoque) { ui->setupUi(this); }

WidgetEstoque::~WidgetEstoque() { delete ui; }

void WidgetEstoque::setupTables() {
  model.setTable("view_estoque");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);
  model.setFilter("`Quant Rest` > 0");

  ui->table->setModel(&model);
  ui->table->setItemDelegate(new DoubleDelegate(this));
}

bool WidgetEstoque::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela estoque: " + model.lastError().text());
    return false;
  }

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetEstoque::on_table_activated(const QModelIndex &index) {
  Estoque *estoque = new Estoque(this);
  estoque->viewRegisterById(model.data(index.row(), "idEstoque").toString());
}

void WidgetEstoque::on_radioButtonMaior_toggled(bool checked) {
  model.setFilter(checked ? "`Quant Rest` > 0" : "`Quant Rest` <= 0");
}

void WidgetEstoque::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

// NOTE: gerenciar lugares de estoque (cadastro/permissoes)
// TODO: mostrar nesta tela as datas do estoque (coleta/recebimento/entrega)
// TODO: mostrar numero da nfe e outras informacoes pertinentes
// TODO: colocar campo para busca
