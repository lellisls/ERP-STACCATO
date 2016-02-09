#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "estoque.h"
#include "estoqueproxymodel.h"
#include "ui_estoque.h"
#include "xml_viewer.h"

Estoque::Estoque(QWidget *parent) : QDialog(parent), ui(new Ui::Estoque) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);
  setAttribute(Qt::WA_DeleteOnClose);

  setupTables();
}

Estoque::~Estoque() { delete ui; }

void Estoque::setupTables() {
  model.setTable("estoque");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + model.lastError().text());
  }

  ui->table->setModel(new EstoqueProxyModel(&model, this));
  ui->table->hideColumn("idCompra");
  ui->table->hideColumn("idVendaProduto");
  ui->table->hideColumn("idProduto");
  ui->table->hideColumn("idNFe");
  ui->table->hideColumn("quantUpd");
}

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

void Estoque::viewRegisterById(const QString &codComercial) { // TODO: change this to idEstoque
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

void Estoque::criarConsumo() {
  // TODO: implement

  for (int row = 0; row < model.rowCount(); ++row) {
    QString codComercial = model.data(row, "codComercial").toString();
    QString idCompra = model.data(row, "idCompra").toString().replace(",", " OR idCompra = ");

    QSqlQuery query;
    if (not query.exec("SELECT quant, idVendaProduto FROM venda_has_produto AS v LEFT JOIN produto AS p ON v.idProduto "
                       "= p.idProduto WHERE p.codComercial = '" +
                       codComercial + "' AND idCompra = " + idCompra + " AND status = 'EM FATURAMENTO'")) {
      QMessageBox::critical(this, "Erro!", "Erro buscando em venda_has_produto: " + model.lastError().text());
      return;
    }

    while (query.next()) {
      if (query.value("quant") > model.data(row, "quant")) continue;

      int newRow = model.rowCount();

      model.insertRow(newRow);

      for (int column = 0; column < model.columnCount(); ++column) {
        if (not model.setData(newRow, column, model.data(row, column))) return;
      }

      double quant = query.value("quant").toDouble() * -1;

      if (not model.setData(newRow, "quant", quant)) return;
      if (not model.setData(newRow, "quantUpd", 4)) return; // DarkGreen
      if (not model.setData(newRow, "idVendaProduto", query.value("idVendaProduto"))) return;
      if (not model.setData(newRow, "idEstoqueConsumido", model.data(row, "idEstoque"))) return;
    }
  }
}
