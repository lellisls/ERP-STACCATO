#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

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

//  if (not model.select()) { // TODO: remover este select e deixar apenas quando for visualizar depois de filtrar?
//    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + model.lastError().text());
//  }

  ui->table->setModel(new EstoqueProxyModel(&model, this));
  ui->table->hideColumn("quantUpd");

  modelConsumo.setTable("estoque_has_consumo");
  modelConsumo.setEditStrategy(QSqlTableModel::OnManualSubmit);

//  if (not modelConsumo.select()) {
//    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + modelConsumo.lastError().text());
//  }
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

void Estoque::calcularRestante() {
  double quant = 0;

  for (int row = 0; row < model.rowCount(); ++row) {
    quant += model.data(row, "quant").toDouble();
  }

  ui->doubleSpinBoxRestante->setValue(quant);
}

void Estoque::viewRegisterById(const QString &idEstoque) {
  if (idEstoque.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Estoque não encontrado!");
    return;
  }

  qDebug() << "idEstoque: " << idEstoque;
  model.setFilter("idEstoque = " + idEstoque);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + model.lastError().text());
    return;
  }

  ui->table->resizeColumnsToContents();

  calcularRestante();

  show();
}

void Estoque::on_pushButtonExibirNfe_clicked() {
  QStringList idList = model.data(0, "idNFe").toString().split(",");

  for (auto id : idList) {
    QSqlQuery query;
    query.prepare("SELECT xml FROM nfe WHERE idNFe = :idNFe");
    query.bindValue(":idNFe", id);

    if (not query.exec() or not query.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando nota fiscal: " + query.lastError().text());
      return;
    }

    XML_Viewer *viewer = new XML_Viewer(this);
    viewer->exibirXML(query.value("xml").toByteArray());
  }
}

bool Estoque::criarConsumo(const int &idVendaProduto) {
  showMaximized();

  for (int row = 0; row < model.rowCount(); ++row) {
    QSqlQuery query;
    query.prepare("SELECT quant FROM venda_has_produto WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":idVendaProduto", idVendaProduto);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando em venda_has_produto: " + model.lastError().text());
      return false;
    }

    while (query.next()) {
      if (query.value("quant") > model.data(row, "quant")) continue;

      int newRow = modelConsumo.rowCount();

      modelConsumo.insertRow(newRow);

      for (int column = 0, columnCount = model.columnCount(); column < columnCount; ++column) {
        QString field = model.record().fieldName(column);
        int index = modelConsumo.fieldIndex(field);
        QVariant value = model.data(row, column);

        if (index != -1) modelConsumo.setData(newRow, index, value);
      }

      double quant = query.value("quant").toDouble() * -1;

      if (not modelConsumo.setData(newRow, "quant", quant)) return false;
      if (not modelConsumo.setData(newRow, "quantUpd", 4)) return false; // DarkGreen
      if (not modelConsumo.setData(newRow, "idVendaProduto", idVendaProduto)) return false;
      if (not modelConsumo.setData(newRow, "idEstoque", model.data(row, "idEstoque"))) return false;
      if (not modelConsumo.setData(newRow, "status", "CONSUMO")) return false;
    }
  }

  if (not modelConsumo.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados: " + model.lastError().text());
    return false;
  }

  calcularRestante();

  return true;
}

void Estoque::on_table_doubleClicked(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

// TODO: colocar data na tabela de estoque para consumo e estoque propriamente dito
// TODO: ordenar como: forn., cod, produto, quant, un, caixas
// TODO: busca descricao, cod
// TODO: pensar em uma forma melhor de relacionar estoque com nfe estoque.idNFe(x,y) -> nfe.idNFe(x) e (y) (tabela de
// relação?)
// TODO: atualizar quantidade de caixas pelo sql (usar joins para calcular a quant de caixas)
// TODO: colocar chaves estrangeiras na tabela estoque
