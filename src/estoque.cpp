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

  ui->tableEstoque->setModel(new EstoqueProxyModel(&model, this));
  ui->tableEstoque->hideColumn("quantUpd");

  modelConsumo.setTable("estoque_has_consumo");
  modelConsumo.setEditStrategy(QSqlTableModel::OnManualSubmit);

  ui->tableConsumo->setModel(&modelConsumo);
}

void Estoque::on_tableEstoque_activated(const QModelIndex &) { exibirNota(); }

void Estoque::calcularRestante() {
  double quant = model.data(0, "quant").toDouble();

  for (int row = 0; row < modelConsumo.rowCount(); ++row) quant += modelConsumo.data(row, "quant").toDouble();

  ui->doubleSpinBoxRestante->setValue(quant);
  ui->doubleSpinBoxRestante->setSuffix(" " + model.data(0, "un").toString());
}

void Estoque::viewRegisterById(const QString &idEstoque) {
  if (idEstoque.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Estoque não encontrado!");
    return;
  }

  model.setFilter("idEstoque = " + idEstoque);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + model.lastError().text());
    return;
  }

  ui->tableEstoque->resizeColumnsToContents();

  modelConsumo.setFilter("idEstoque = " + idEstoque);

  if (not modelConsumo.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque consumo: " + model.lastError().text());
    return;
  }

  ui->tableConsumo->resizeColumnsToContents();

  calcularRestante();

  show();
}

void Estoque::on_pushButtonExibirNfe_clicked() { exibirNota(); }

void Estoque::exibirNota() {
  QSqlQuery query;
  query.prepare("SELECT xml FROM estoque e LEFT JOIN estoque_has_nfe ehn ON e.idEstoque = ehn.idEstoque LEFT JOIN "
                "nfe n ON ehn.idNFe = n.idNFe WHERE e.idEstoque = :idEstoque");
  query.bindValue(":idEstoque", model.data(0, "idEstoque"));

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando nfe: " + query.lastError().text());
    return;
  }

  XML_Viewer *viewer = new XML_Viewer(this);
  viewer->exibirXML(query.value("xml").toByteArray());
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

      const int newRow = modelConsumo.rowCount();

      modelConsumo.insertRow(newRow);

      for (int column = 0, columnCount = model.columnCount(); column < columnCount; ++column) {
        const QString field = model.record().fieldName(column);
        const int index = modelConsumo.fieldIndex(field);
        const QVariant value = model.data(row, column);

        if (index != -1) modelConsumo.setData(newRow, index, value);
      }

      const double quant = query.value("quant").toDouble() * -1;

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

void Estoque::on_tableEstoque_entered(const QModelIndex &) { ui->tableEstoque->resizeColumnsToContents(); }

void Estoque::on_tableConsumo_entered(const QModelIndex &) { ui->tableConsumo->resizeColumnsToContents(); }

// TODO: colocar data na tabela de estoque para consumo e estoque propriamente dito
// TODO: ordenar como: forn., cod, produto, quant, un, caixas
// TODO: busca descricao, cod
// TODO: atualizar quantidade de caixas pelo sql (usar joins para calcular a quant de caixas)
// TODO: colocar chaves estrangeiras na tabela estoque
// TODO: tabela estoque possui coluna 'ordemCompra' guardando um valor mas pode possuir varios valores pelas relacoes
// com pedido_fornecedor_has_produto. remover coluna?
