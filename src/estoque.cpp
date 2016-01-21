#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "estoque.h"
#include "ui_estoque.h"
#include "xml_viewer.h"

Estoque::Estoque(QWidget *parent) : QDialog(parent), ui(new Ui::Estoque) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);
  setAttribute(Qt::WA_DeleteOnClose);

  model.setTable("estoque");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + model.lastError().text());
  }

  ui->table->setModel(&model);
  ui->table->hideColumn("idEstoque");
  ui->table->hideColumn("idCompra");
  ui->table->hideColumn("idVendaProduto");
  ui->table->hideColumn("idProduto");
  ui->table->hideColumn("idNFe");
  ui->table->hideColumn("quantUpd");
}

Estoque::~Estoque() { delete ui; }

void Estoque::on_table_activated(const QModelIndex &index) {
  QSqlQuery query;
  query.prepare("SELECT xml FROM nfe WHERE idNFe = :idNFe");
  query.bindValue(":idNFe", model.data(index.row(), "idNFe"));

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando XML da NFe: " + query.lastError().text());
    return;
  }

  XML_Viewer *viewer = new XML_Viewer(this);
  viewer->exibirXML(query.value("xml").toByteArray());
}

void Estoque::viewRegisterById(const QString &codComercial) {
  if (codComercial.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Estoque não encontrado!");
    return;
  }

  model.setFilter("codComercial = '" + codComercial + "'");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + model.lastError().text());
    return;
  }

  for (int column = 0; column < model.columnCount(); ++column) {
    if (model.fieldIndex("xml") == column) continue;

    ui->table->resizeColumnToContents(column);
  }

  show();
}

void Estoque::on_pushButtonExibirNfe_clicked() {
  QSqlQuery query;
  query.prepare("SELECT xml FROM nfe WHERE idNFe = :idNFe");
  query.bindValue(":idNFe", model.data(0, "idNFe"));

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando nota fiscal: " + query.lastError().text());
    return;
  }

  XML_Viewer *viewer = new XML_Viewer(this);
  viewer->exibirXML(query.value("xml").toByteArray());
}

// TODO: pintar/filtrar
