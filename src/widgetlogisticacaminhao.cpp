#include "widgetlogisticacaminhao.h"
#include "ui_widgetlogisticacaminhao.h"

#include <QMessageBox>
#include <QSqlError>

WidgetLogisticaCaminhao::WidgetLogisticaCaminhao(QWidget *parent)
    : QWidget(parent), ui(new Ui::WidgetLogisticaCaminhao) {
  ui->setupUi(this);
}

WidgetLogisticaCaminhao::~WidgetLogisticaCaminhao() { delete ui; }

void WidgetLogisticaCaminhao::setupTables() {
  model.setTable("view_caminhao");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela caminhâo: " + model.lastError().text());
  }

  ui->table->setModel(&model);
}

bool WidgetLogisticaCaminhao::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela caminhão: " + model.lastError().text());
    return false;
  }

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetLogisticaCaminhao::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

// TODO: colocar uma segunda tabela a direita para ver o conteudo das cargas
