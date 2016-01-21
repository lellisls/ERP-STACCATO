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
  model.setFilter("Quant > 0");

  ui->table->setModel(&model);
  ui->table->setItemDelegate(new DoubleDelegate(this));
}

QString WidgetEstoque::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) return "Erro lendo tabela estoque: " + model.lastError().text();

  ui->table->resizeColumnsToContents();

  return QString();
}

void WidgetEstoque::on_table_activated(const QModelIndex &index) {
  Estoque *estoque = new Estoque(this);
  estoque->viewRegisterById(model.data(index.row(), "Cód Com").toString());
}

// NOTE: gerenciar lugares de estoque (cadastro/permissoes)
// TODO: adicionar caixas
