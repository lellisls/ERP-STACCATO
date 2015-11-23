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

  modelEstoque.setTable("estoque");
  modelEstoque.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelEstoque.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + modelEstoque.lastError().text());
  }

  ui->tableEstoque->setModel(&modelEstoque);
  ui->tableEstoque->hideColumn("idEstoque");
  ui->tableEstoque->hideColumn("idCompra");
  ui->tableEstoque->hideColumn("idVendaProduto");
  ui->tableEstoque->hideColumn("idProduto");
  ui->tableEstoque->hideColumn("idNFe");
  ui->tableEstoque->hideColumn("quantUpd");
}

Estoque::~Estoque() { delete ui; }

void Estoque::on_tableEstoque_activated(const QModelIndex &index) {
  int id = modelEstoque.data(index.row(), "idXML").toInt();

  QSqlQuery query;
  query.prepare("SELECT xml FROM nfe WHERE idNFe = :idNFe");
  query.bindValue(":idNFe", id);

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando XML da NFe: " + query.lastError().text());
    return;
  }

  XML_Viewer *xml = new XML_Viewer(this);
  xml->exibirXML(query.value(0).toString());
  xml->show();
}

void Estoque::viewRegisterById(const QString &codComercial) {
  if (codComercial.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Estoque não encontrado!");
    return;
  }

  modelEstoque.setFilter("codComercial = '" + codComercial + "'");

  if (not modelEstoque.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + modelEstoque.lastError().text());
    return;
  }

  for (int column = 0; column < modelEstoque.columnCount(); ++column) {
    if (modelEstoque.fieldIndex("xml") == column) {
      continue;
    }

    ui->tableEstoque->resizeColumnToContents(column);
  }

  show();
}

void Estoque::on_pushButtonExibirNfe_clicked() {
  QSqlQuery query;
  query.prepare("SELECT xml FROM nfe WHERE idNFe = :idNFe");
  query.bindValue(":idNFe", modelEstoque.data(0, "idNFe"));

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando nota fiscal: " + query.lastError().text());
    return;
  }

  XML_Viewer *viewer = new XML_Viewer(this);
  viewer->exibirXML(query.value(0).toString());
  viewer->show();
}
