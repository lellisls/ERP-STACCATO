#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "doubledelegate.h"
#include "ui_widgetnfeentrada.h"
#include "widgetnfeentrada.h"
#include "xml_viewer.h"

WidgetNfeEntrada::WidgetNfeEntrada(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetNfeEntrada) {
  ui->setupUi(this);
}

WidgetNfeEntrada::~WidgetNfeEntrada() { delete ui; }

bool WidgetNfeEntrada::updateTables() {
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

void WidgetNfeEntrada::setupTables() {
  DoubleDelegate *doubledelegate = new DoubleDelegate(this);

  model.setTable("view_nfe");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);
  model.setFilter("tipo = 'ENTRADA'");

  ui->table->setModel(&model);
  ui->table->hideColumn("Venda");
  ui->table->hideColumn("tipo");
  ui->table->hideColumn("CPF");
  ui->table->hideColumn("CNPJ");
  ui->table->hideColumn("Cliente");
  ui->table->setItemDelegate(doubledelegate);
}

void WidgetNfeEntrada::on_table_activated(const QModelIndex &index) {
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

void WidgetNfeEntrada::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetNfeEntrada::on_lineEditBusca_textChanged(const QString &text) {
  model.setFilter("tipo = 'ENTRADA' AND (NFe LIKE '%" + text + "%' OR OC LIKE '%" + text + "%')");
}
