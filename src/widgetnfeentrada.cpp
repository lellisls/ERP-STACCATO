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

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela NFe: " + model.lastError().text());
    return false;
  }

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetNfeEntrada::setupTables() {
  model.setTable("view_nfe_entrada");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());

  ui->table->setModel(&model);
  ui->table->hideColumn("idNFe");
  ui->table->setItemDelegate(new DoubleDelegate(this));
}

void WidgetNfeEntrada::on_table_activated(const QModelIndex &index) {
  QSqlQuery query;
  query.prepare("SELECT xml FROM nfe WHERE numeroNFe = :numeroNFe");
  query.bindValue(":numeroNFe", model.data(index.row(), "NFe"));

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando xml da nota: " + query.lastError().text());
    return;
  }

  XML_Viewer *viewer = new XML_Viewer(this);
  viewer->setAttribute(Qt::WA_DeleteOnClose);
  viewer->exibirXML(query.value("xml").toByteArray());
}

void WidgetNfeEntrada::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetNfeEntrada::on_lineEditBusca_textChanged(const QString &text) {
  model.setFilter("NFe LIKE '%" + text + "%' OR OC LIKE '%" + text + "%' OR Venda LIKE '%" + text + "%'");

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
}

void WidgetNfeEntrada::on_pushButtonCancelarNFe_clicked() {
  // TODO: 1quando cancelar nota pegar os estoques e cancelar/remover da logistica (exceto quando estiverem entregues?)
}
