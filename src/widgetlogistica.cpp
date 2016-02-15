#include <QSqlError>

#include "ui_widgetlogistica.h"
#include "widgetlogistica.h"

WidgetLogistica::WidgetLogistica(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetLogistica) {
  ui->setupUi(this);

  ui->splitter_6->setStretchFactor(0, 0);
  ui->splitter_6->setStretchFactor(1, 1);
}

WidgetLogistica::~WidgetLogistica() { delete ui; }

void WidgetLogistica::setupTables() {
  model.setTable("view_fornecedor_logistica2");

  model.setHeaderData("fornecedor", "Fornecedor");

  ui->tableForn->setModel(&model);
}

QString WidgetLogistica::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) return "Erro lendo tabela: " + model.lastError().text();

  ui->tableForn->resizeColumnsToContents();

  switch (ui->tabWidgetLogistica->currentIndex()) {
    case 0:
      return ui->widgetColeta->updateTables();

    case 1:
      return ui->widgetRecebimento->updateTables();

    case 2:
      return ui->widgetEntrega->updateTables();
  }

  return QString();
}

void WidgetLogistica::on_tableForn_activated(const QModelIndex &index) {
  const QString fornecedor = model.data(index.row(), "fornecedor").toString();

  if (ui->tabWidgetLogistica->currentIndex() == 0) {
    ui->widgetColeta->TableFornLogistica_activated(fornecedor);
  }

  if (ui->tabWidgetLogistica->currentIndex() == 1) {
    ui->widgetRecebimento->TableFornLogistica_activated(fornecedor);
  }
}

void WidgetLogistica::on_tabWidgetLogistica_currentChanged(const int &) {
  int index = ui->tabWidgetLogistica->currentIndex();

  if (index == 0 or index == 1) ui->frameForn->show();
  if (index == 2) ui->frameForn->hide();

  updateTables();
}

// NOTE: arrumar filtros da tela de entrega
// NOTE: criar tela para organizar caminhão da coleta
