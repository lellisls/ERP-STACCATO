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
  model.setTable("view_fornecedor_logistica_agendar_coleta");

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

  const QString currentText = ui->tabWidgetLogistica->tabText(ui->tabWidgetLogistica->currentIndex());

  if (currentText == "Agendar Coleta") {
    ui->frameForn->show();
    model.setTable("view_fornecedor_logistica_agendar_coleta");
    model.select();
    return ui->widgetAgendarColeta->updateTables();
  }

  if (currentText == "Coleta") {
    ui->frameForn->show();
    model.setTable("view_fornecedor_logistica_coleta");
    model.select();
    return ui->widgetColeta->updateTables();
  }

  if (currentText == "Recebimento") {
    ui->frameForn->show();
    model.setTable("view_fornecedor_logistica_recebimento");
    model.select();
    return ui->widgetRecebimento->updateTables();
  }

  if (currentText == "Agendar Entrega") {
    ui->frameForn->hide();
    return ui->widgetAgendaEntrega->updateTables();
  }

  if (currentText == "Entregas") {
    ui->frameForn->hide();
    return ui->widgetCalendarioEntrega->updateTables();
  }

  if (currentText == "Caminhões") {
    ui->frameForn->hide();
    return ui->widgetCaminhao->updateTables();
  }

  if (currentText == "Representação") {
    ui->frameForn->show();
    model.setTable("view_fornecedor_logistica_representacao");
    model.select();
    return ui->widgetRepresentacao->updateTables();
  }

  if (currentText == "Entregues") {
    ui->frameForn->hide();
    return ui->widgetEntregues->updateTables();
  }

  return true;
}

void WidgetLogistica::on_tableForn_activated(const QModelIndex &index) {
  const QString fornecedor = model.data(index.row(), "fornecedor").toString();

  const QString currentText = ui->tabWidgetLogistica->tabText(ui->tabWidgetLogistica->currentIndex());

  if (currentText == "Agendar Coleta") ui->widgetAgendarColeta->tableFornLogistica_activated(fornecedor);
  if (currentText == "Coleta") ui->widgetColeta->tableFornLogistica_activated(fornecedor);
  if (currentText == "Recebimento") ui->widgetRecebimento->tableFornLogistica_activated(fornecedor);
  if (currentText == "Representação") ui->widgetRepresentacao->tableFornLogistica_activated(fornecedor);
}

void WidgetLogistica::on_tabWidgetLogistica_currentChanged(const int &) { updateTables(); }

// TODO: colorir prazoEntrega
// TODO: colocar nas configuracoes do usuario pasta para 'pdf entregas/danfe'
// TODO: tela para guardar imagens (fotos/documentos scaneados)
// TODO: followup das entregas
// TODO: criar opcao 'cliente retira'
