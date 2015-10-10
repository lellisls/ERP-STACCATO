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

  modelEstoque.setTable("estoque");
  modelEstoque.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelEstoque.select()) {
    qDebug() << "Erro carregando estoque: " << modelEstoque.lastError();
  }

  ui->tableEstoque->setModel(&modelEstoque);
  ui->tableEstoque->setColumnHidden(modelEstoque.fieldIndex("idEstoque"), true);
  ui->tableEstoque->setColumnHidden(modelEstoque.fieldIndex("idCompra"), true);
  ui->tableEstoque->setColumnHidden(modelEstoque.fieldIndex("idVendaProduto"), true);
  ui->tableEstoque->setColumnHidden(modelEstoque.fieldIndex("idProduto"), true);
  ui->tableEstoque->setColumnHidden(modelEstoque.fieldIndex("idNFe"), true);
  ui->tableEstoque->setColumnHidden(modelEstoque.fieldIndex("quantUpd"), true);
}

Estoque::~Estoque() { delete ui; }

void Estoque::on_tableEstoque_activated(const QModelIndex &index) {
  int id = modelEstoque.data(index.row(), "idXML").toInt();

  QSqlQuery query;
  query.prepare("SELECT xml FROM nfe WHERE idNFe = :idNFe");
  query.bindValue(":idNFe", id);

  if (not query.exec() or not query.first()) {
    qDebug() << "Erro buscando xml nfe: " << query.lastError();
    return;
  }

  XML_Viewer *xml = new XML_Viewer(this);
  xml->exibirXML(query.value(0).toString());
  xml->show();
}

void Estoque::viewRegisterById(const QString codComercial) {
  if (codComercial.isEmpty()) {
    QMessageBox::warning(this, "Aviso!", "Estoque não encontrado!");
    return;
  }

  modelEstoque.setFilter("codComercial = '" + codComercial + "'");

  if (not modelEstoque.select()) {
    qDebug() << "erro modelEstoque: " << modelEstoque.lastError();
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
    QMessageBox::warning(this, "Aviso!", "Erro buscando nota fiscal!");
    return;
  }

  XML_Viewer *viewer = new XML_Viewer(this);
  viewer->exibirXML(query.value(0).toString());
  viewer->show();
}
