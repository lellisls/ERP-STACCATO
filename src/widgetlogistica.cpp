#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "ui_widgetlogistica.h"
#include "widgetlogistica.h"

WidgetLogistica::WidgetLogistica(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetLogistica) {
  ui->setupUi(this);

  ui->splitter_6->setStretchFactor(0, 0);
  ui->splitter_6->setStretchFactor(1, 1);

  ui->tabWidgetLogistica->setTabEnabled(5, false);
  ui->tabWidgetLogistica->setTabEnabled(6, false);
  ui->tabWidgetLogistica->setTabEnabled(7, false);
}

WidgetLogistica::~WidgetLogistica() { delete ui; }

void WidgetLogistica::setupTables() {
  model.setTable("view_fornecedor_logistica");

  ui->tableForn->setModel(&model);
}

bool WidgetLogistica::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela: " + model.lastError().text());
    return false;
  }

  ui->tableForn->resizeColumnsToContents();

  connect(ui->widgetCalendarioEntrega, &CalendarioEntregas::errorSignal, this, &WidgetLogistica::errorSignal);
  connect(ui->widgetAgendarColeta, &WidgetLogisticaAgendarColeta::errorSignal, this, &WidgetLogistica::errorSignal);
  connect(ui->widgetRecebimento, &WidgetLogisticaRecebimento::errorSignal, this, &WidgetLogistica::errorSignal);
  connect(ui->widgetAgendaEntrega, &WidgetLogisticaEntrega::errorSignal, this, &WidgetLogistica::errorSignal);

  switch (ui->tabWidgetLogistica->currentIndex()) {
  case 0: {
    ui->frameForn->show();
    model.setTable("view_fornecedor_logistica_agendar_coleta");
    model.select();
    return ui->widgetAgendarColeta->updateTables();
  }

  case 1: {
    ui->frameForn->show();
    model.setTable("view_fornecedor_logistica_coleta");
    model.select();
    return ui->widgetColeta->updateTables();
  }

  case 2: {
    ui->frameForn->show();
    model.setTable("view_fornecedor_logistica_recebimento");
    model.select();
    return ui->widgetRecebimento->updateTables();
  }

  case 3: {
    ui->frameForn->hide();
    return ui->widgetAgendaEntrega->updateTables();
  }

  case 4: {
    ui->frameForn->hide();
    return ui->widgetCalendarioEntrega->updateTables();
  }
  }

  return true;
}

void WidgetLogistica::on_tableForn_activated(const QModelIndex &index) {
  const QString fornecedor = model.data(index.row(), "fornecedor").toString();

  int currentIndex = ui->tabWidgetLogistica->currentIndex();

  // TODO: usar o texto no lugar do indice
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
    ui->widgetRepresentacao->TableFornLogistica_activated(fornecedor);
    break;
  }
}

void WidgetLogistica::on_tabWidgetLogistica_currentChanged(const int &) {
  int index = ui->tabWidgetLogistica->currentIndex();

  if (index == 0 or index == 1) model.setTable("view_fornecedor_logistica");
  if (index == 6) model.setTable("view_fornecedor_logistica_representacao");

  updateTables();
}

// NOTE: arrumar filtros da tela de entrega
// NOTE: criar tela para organizar caminhão da coleta
// NOTE: criar tela para gerenciar entregas por caminhao (mostrar por dia/caminhao) somar todos os pesos
// TODO: janela para visualizar coletas/recebimentos de representacao
// TODO: ao mudar de aba reaplicar o filtro fornecedor para o widget atual: coleta -> receb. aplicar filtro 'portinari'
// sem precisar clicar em fornecedor novamente
// TODO: tela para agendar coletas, tela para agendar entregas, tela para mostrar as entregas do dia (para emitir nota)
