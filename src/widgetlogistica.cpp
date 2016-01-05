#include <QSqlError>

#include "widgetlogistica.h"
#include "ui_widgetlogistica.h"

WidgetLogistica::WidgetLogistica(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetLogistica) {
  ui->setupUi(this);

  setupTables();

  ui->splitter_6->setStretchFactor(0, 0);
  ui->splitter_6->setStretchFactor(1, 1);
}

WidgetLogistica::~WidgetLogistica() { delete ui; }

void WidgetLogistica::setupTables() {
  modelPedForn.setTable("view_fornecedor_logistica");

  modelPedForn.setHeaderData("fornecedor", "Fornecedor");
  modelPedForn.setHeaderData("COUNT(fornecedor)", "Itens");

  ui->tableFornLogistica->setModel(&modelPedForn);
}

QString WidgetLogistica::updateTables() {
  if (not modelPedForn.select()) {
    return "Erro lendo tabela: " + modelPedForn.lastError().text();
  }

  ui->tableFornLogistica->resizeColumnsToContents();

  switch (ui->tabWidgetLogistica->currentIndex()) {
    case 0: { // Coletas
        return ui->widgetColeta->updateTables();
      }

    case 1: { // Recebimentos
        return ui->widgetRecebimento->updateTables();
      }

    case 2: { // Entregas
        return ui->widgetEntrega->updateTables();
      }
  }

  return QString();
}

void WidgetLogistica::on_tableFornLogistica_activated(const QModelIndex &index) {
  const QString fornecedor = modelPedForn.data(index.row(), "fornecedor").toString();

  ui->widgetColeta->TableFornLogistica_activated(fornecedor);
  ui->widgetRecebimento->TableFornLogistica_activated(fornecedor);
}

void WidgetLogistica::on_tabWidgetLogistica_currentChanged(const int &) { updateTables(); }

// NOTE: arrumar filtros da tela de entrega
// NOTE: criar tela para organizar caminhão da coleta
