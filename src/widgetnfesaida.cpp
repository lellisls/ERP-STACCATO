#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "doubledelegate.h"
#include "ui_widgetnfesaida.h"
#include "venda.h"
#include "widgetnfesaida.h"
#include "xml_viewer.h"

WidgetNfeSaida::WidgetNfeSaida(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetNfeSaida) { ui->setupUi(this); }

WidgetNfeSaida::~WidgetNfeSaida() { delete ui; }

bool WidgetNfeSaida::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  QString filter = model.filter();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela NFe: " + model.lastError().text());
    return false;
  }

  model.setFilter(filter);

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetNfeSaida::setupTables() {
  DoubleDelegate *doubledelegate = new DoubleDelegate(this);

  model.setTable("view_nfe");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);
  model.setFilter("tipo = 'SAIDA'");

  ui->table->setModel(&model);
  ui->table->hideColumn("OC");
  ui->table->hideColumn("tipo");
  ui->table->setItemDelegate(doubledelegate);
}

void WidgetNfeSaida::on_table_activated(const QModelIndex &index) {
  XML_Viewer *viewer = new XML_Viewer(this);

  QSqlQuery query;
  query.prepare("SELECT xml FROM nfe WHERE numeroNFe = :numeroNFe");
  query.bindValue(":numeroNFe", model.data(index.row(), "NFe"));

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando xml da nota: " + query.lastError().text());
    return;
  }

  viewer->exibirXML(query.value("xml").toByteArray());
}

void WidgetNfeSaida::on_radioButtonAutorizado_clicked() {
  model.setFilter("status = 'AUTORIZADO'");
  ui->table->resizeColumnsToContents();
}

void WidgetNfeSaida::on_radioButtonEnviado_clicked() {
  model.setFilter("status = 'ENVIADO'");
  ui->table->resizeColumnsToContents();
}

void WidgetNfeSaida::on_radioButtonTodos_clicked() {
  model.setFilter("");
  ui->table->resizeColumnsToContents();
}

void WidgetNfeSaida::on_lineEditBusca_textChanged(const QString &text) {
  model.setFilter("tipo = 'SAÍDA' AND (NFe LIKE '%" + text + "%' OR Venda LIKE '%" + text + "%' OR CPF LIKE '%" + text +
                  "%' OR CNPJ LIKE '%" + text + "%' OR Cliente LIKE '%" + text + "%')");
  ui->table->resizeColumnsToContents();
}

void WidgetNfeSaida::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }
