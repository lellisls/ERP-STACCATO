#include <QMessageBox>
#include <QSqlError>

#include "reaisdelegate.h"
#include "ui_widgetreceberresumo.h"
#include "widgetreceberresumo.h"

WidgetReceberResumo::WidgetReceberResumo(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetReceberResumo) {
  ui->setupUi(this);
}

WidgetReceberResumo::~WidgetReceberResumo() { delete ui; }

void WidgetReceberResumo::setupTables() {
  modelVencer.setTable("view_a_receber_vencer");
  modelVencer.setEditStrategy(SqlTableModel::OnManualSubmit);

  if (not modelVencer.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela view_a_receber_vencer: " + modelVencer.lastError().text());
  }

  ui->tableVencer->setModel(&modelVencer);
  ui->tableVencer->setItemDelegate(new ReaisDelegate(this));

  // --------------------------

  modelVencidos.setTable("view_a_receber_vencidos");
  modelVencidos.setEditStrategy(SqlTableModel::OnManualSubmit);

  if (not modelVencidos.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela view_a_receber_vencidos: " + modelVencidos.lastError().text());
  }

  ui->tableVencidos->setModel(&modelVencidos);
  ui->tableVencidos->setItemDelegate(new ReaisDelegate(this));
}

bool WidgetReceberResumo::updateTables() {
  if (modelVencer.tableName().isEmpty()) setupTables();

  if (not modelVencer.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela view_a_receber_vencer: " + modelVencer.lastError().text());
    return false;
  }

  ui->tableVencer->resizeColumnsToContents();

  if (not modelVencidos.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela view_a_receber_vencidos: " + modelVencidos.lastError().text());
    return false;
  }

  ui->tableVencidos->resizeColumnsToContents();

  return true;
}

void WidgetReceberResumo::on_tableVencidos_entered(const QModelIndex &) {
  ui->tableVencidos->resizeColumnsToContents();
}

void WidgetReceberResumo::on_tableVencer_entered(const QModelIndex &) { ui->tableVencer->resizeColumnsToContents(); }
