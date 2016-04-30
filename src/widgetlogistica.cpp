#include <QMessageBox>
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
  model.setTable("view_fornecedor_logistica");

  model.setHeaderData("fornecedor", "Fornecedor");

  ui->tableForn->setModel(&model);
}

bool WidgetLogistica::updateTables(QString &error) {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) {
    error = "Erro lendo tabela: " + model.lastError().text();
    return false;
  }

  ui->tableForn->resizeColumnsToContents();

  switch (ui->tabWidgetLogistica->currentIndex()) {
  case 0:
    return ui->widgetColeta->updateTables(error);

  case 1:
    return ui->widgetRecebimento->updateTables(error);

  case 2:
    return ui->widgetEntrega->updateTables(error);
  }

  return true;
}

void WidgetLogistica::on_tableForn_activated(const QModelIndex &index) {
  const QString fornecedor = model.data(index.row(), "fornecedor").toString();

  if (ui->tabWidgetLogistica->currentIndex() == 0) ui->widgetColeta->TableFornLogistica_activated(fornecedor);
  if (ui->tabWidgetLogistica->currentIndex() == 1) ui->widgetRecebimento->TableFornLogistica_activated(fornecedor);
  if (ui->tabWidgetLogistica->currentIndex() == 4) ui->widgetRepresentacao->TableFornLogistica_activated(fornecedor);
}

void WidgetLogistica::on_tabWidgetLogistica_currentChanged(const int &) {
  int index = ui->tabWidgetLogistica->currentIndex();

  if (index == 0 or index == 1) ui->frameForn->show();
  if (index == 2) ui->frameForn->hide();

  QString error;

  if (not updateTables(error)) QMessageBox::critical(this, "Erro!", error);
}

// NOTE: arrumar filtros da tela de entrega
// NOTE: criar tela para organizar caminhão da coleta
// NOTE: criar tela para gerenciar entregas por caminhao (mostrar por dia/caminhao) somar todos os pesos
// TODO: janela para visualizar coletas/recebimentos de representacao
