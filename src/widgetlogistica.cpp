#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

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

bool WidgetLogistica::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  QString filter = model.filter();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela: " + model.lastError().text());
    return false;
  }

  model.setFilter(filter);

  ui->tableForn->resizeColumnsToContents();

  connect(ui->widgetAgendarColeta, &WidgetLogisticaAgendarColeta::errorSignal, this, &WidgetLogistica::errorSignal);
  connect(ui->widgetColeta, &WidgetLogisticaColeta::errorSignal, this, &WidgetLogistica::errorSignal);
  connect(ui->widgetRecebimento, &WidgetLogisticaRecebimento::errorSignal, this, &WidgetLogistica::errorSignal);
  connect(ui->widgetEntrega, &WidgetLogisticaEntrega::errorSignal, this, &WidgetLogistica::errorSignal);

  switch (ui->tabWidgetLogistica->currentIndex()) {
  case 0:
    return ui->widgetAgendarColeta->updateTables();

  case 1:
    return ui->widgetColeta->updateTables();

  case 2:
    return ui->widgetRecebimento->updateTables();

  case 4:
    return ui->widgetEntrega->updateTables();
  }

  return true;
}

void WidgetLogistica::on_tableForn_activated(const QModelIndex &index) {
  const QString fornecedor = model.data(index.row(), "fornecedor").toString();

  int currentIndex = ui->tabWidgetLogistica->currentIndex();

  switch (currentIndex) {
  case 0:
    ui->widgetAgendarColeta->TableFornLogistica_activated(fornecedor);
    break;

  case 1:
    ui->widgetColeta->TableFornLogistica_activated(fornecedor);
    break;

  case 2:
    ui->widgetRecebimento->TableFornLogistica_activated(fornecedor);
    break;

  case 3:
    // agendar entrega
    break;

  case 6:
    ui->widgetRecebimento->TableFornLogistica_activated(fornecedor);
    break;
  }
}

void WidgetLogistica::on_tabWidgetLogistica_currentChanged(const int &) {
  int index = ui->tabWidgetLogistica->currentIndex();

  if (index == 0 or index == 1) ui->frameForn->show();
  //  if (index == 2) ui->frameForn->hide();

  updateTables();
}

// NOTE: arrumar filtros da tela de entrega
// NOTE: criar tela para organizar caminhão da coleta
// NOTE: criar tela para gerenciar entregas por caminhao (mostrar por dia/caminhao) somar todos os pesos
// TODO: janela para visualizar coletas/recebimentos de representacao
// TODO: ao mudar de aba reaplicar o filtro fornecedor para o widget atual: coleta -> receb. aplicar filtro 'portinari'
// sem precisar clicar em fornecedor novamente
// TODO: tela para agendar coletas, tela para agendar entregas, tela para mostrar as entregas do dia (para emitir nota)
