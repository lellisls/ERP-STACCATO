#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include "doubledelegate.h"
#include "estoque.h"
#include "estoqueproxymodel.h"
#include "ui_estoque.h"
#include "xml_viewer.h"

Estoque::Estoque(QWidget *parent) : QDialog(parent), ui(new Ui::Estoque) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables();
}

Estoque::~Estoque() { delete ui; }

void Estoque::setupTables() {
  model.setTable("estoque");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  model.setHeaderData("idEstoque", "Estoque");
  model.setHeaderData("recebidoPor", "Recebido Por");
  model.setHeaderData("status", "Status");
  model.setHeaderData("local", "Local");
  model.setHeaderData("fornecedor", "Fornecedor");
  model.setHeaderData("descricao", "Produto");
  model.setHeaderData("quant", "Quant.");
  model.setHeaderData("un", "Un.");
  model.setHeaderData("caixas", "Caixas");
  model.setHeaderData("codComercial", "Cód. Com.");
  model.setHeaderData("lote", "Lote");
  model.setHeaderData("bloco", "Bloco");

  ui->tableEstoque->setModel(new EstoqueProxyModel(&model, this));
  ui->tableEstoque->setItemDelegateForColumn("quant", new DoubleDelegate(this, 4));
  ui->tableEstoque->hideColumn("quantUpd");
  ui->tableEstoque->hideColumn("idProduto");
  ui->tableEstoque->hideColumn("codBarras");
  ui->tableEstoque->hideColumn("ncm");
  ui->tableEstoque->hideColumn("cfop");
  ui->tableEstoque->hideColumn("valorUnid");
  ui->tableEstoque->hideColumn("valor");
  ui->tableEstoque->hideColumn("codBarrasTrib");
  ui->tableEstoque->hideColumn("unTrib");
  ui->tableEstoque->hideColumn("quantTrib");
  ui->tableEstoque->hideColumn("valorTrib");
  ui->tableEstoque->hideColumn("desconto");
  ui->tableEstoque->hideColumn("compoeTotal");
  ui->tableEstoque->hideColumn("numeroPedido");
  ui->tableEstoque->hideColumn("itemPedido");
  ui->tableEstoque->hideColumn("tipoICMS");
  ui->tableEstoque->hideColumn("orig");
  ui->tableEstoque->hideColumn("cstICMS");
  ui->tableEstoque->hideColumn("modBC");
  ui->tableEstoque->hideColumn("vBC");
  ui->tableEstoque->hideColumn("pICMS");
  ui->tableEstoque->hideColumn("vICMS");
  ui->tableEstoque->hideColumn("modBCST");
  ui->tableEstoque->hideColumn("pMVAST");
  ui->tableEstoque->hideColumn("vBCST");
  ui->tableEstoque->hideColumn("pICMSST");
  ui->tableEstoque->hideColumn("vICMSST");
  ui->tableEstoque->hideColumn("cEnq");
  ui->tableEstoque->hideColumn("cstIPI");
  ui->tableEstoque->hideColumn("cstPIS");
  ui->tableEstoque->hideColumn("vBCPIS");
  ui->tableEstoque->hideColumn("pPIS");
  ui->tableEstoque->hideColumn("vPIS");
  ui->tableEstoque->hideColumn("cstCOFINS");
  ui->tableEstoque->hideColumn("vBCCOFINS");
  ui->tableEstoque->hideColumn("pCOFINS");
  ui->tableEstoque->hideColumn("vCOFINS");

  modelConsumo.setTable("estoque_has_consumo");
  modelConsumo.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelConsumo.setFilter("0");

  modelViewConsumo.setTable("view_estoque_consumo");
  modelViewConsumo.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelViewConsumo.setHeaderData("status", "Status");
  modelViewConsumo.setHeaderData("ordemCompra", "OC");
  modelViewConsumo.setHeaderData("local", "Local");
  modelViewConsumo.setHeaderData("fornecedor", "Fornecedor");
  modelViewConsumo.setHeaderData("descricao", "Produto");
  modelViewConsumo.setHeaderData("quant", "Quant.");
  modelViewConsumo.setHeaderData("un", "Un.");
  modelViewConsumo.setHeaderData("caixas", "Caixas");
  modelViewConsumo.setHeaderData("codComercial", "Cód. Com.");
  modelViewConsumo.setHeaderData("created", "Criado");

  ui->tableConsumo->setModel(new EstoqueProxyModel(&modelViewConsumo, this));
  ui->tableConsumo->setItemDelegateForColumn("quant", new DoubleDelegate(this, 4));
  ui->tableConsumo->showColumn("created");
  ui->tableConsumo->hideColumn("idConsumo");
  ui->tableConsumo->hideColumn("idEstoque");
  ui->tableConsumo->hideColumn("idVendaProduto");
  ui->tableConsumo->hideColumn("idCompra");
  ui->tableConsumo->hideColumn("idProduto");
  ui->tableConsumo->hideColumn("quantUpd");
  ui->tableConsumo->hideColumn("codBarras");
  ui->tableConsumo->hideColumn("ncm");
  ui->tableConsumo->hideColumn("cfop");
  ui->tableConsumo->hideColumn("valorUnid");
  ui->tableConsumo->hideColumn("valor");
  ui->tableConsumo->hideColumn("codBarrasTrib");
  ui->tableConsumo->hideColumn("unTrib");
  ui->tableConsumo->hideColumn("quantTrib");
  ui->tableConsumo->hideColumn("valorTrib");
  ui->tableConsumo->hideColumn("desconto");
  ui->tableConsumo->hideColumn("compoeTotal");
  ui->tableConsumo->hideColumn("numeroPedido");
  ui->tableConsumo->hideColumn("itemPedido");
  ui->tableConsumo->hideColumn("tipoICMS");
  ui->tableConsumo->hideColumn("orig");
  ui->tableConsumo->hideColumn("cstICMS");
  ui->tableConsumo->hideColumn("modBC");
  ui->tableConsumo->hideColumn("vBC");
  ui->tableConsumo->hideColumn("pICMS");
  ui->tableConsumo->hideColumn("vICMS");
  ui->tableConsumo->hideColumn("modBCST");
  ui->tableConsumo->hideColumn("pMVAST");
  ui->tableConsumo->hideColumn("vBCST");
  ui->tableConsumo->hideColumn("pICMSST");
  ui->tableConsumo->hideColumn("vICMSST");
  ui->tableConsumo->hideColumn("cEnq");
  ui->tableConsumo->hideColumn("cstIPI");
  ui->tableConsumo->hideColumn("cstPIS");
  ui->tableConsumo->hideColumn("vBCPIS");
  ui->tableConsumo->hideColumn("pPIS");
  ui->tableConsumo->hideColumn("vPIS");
  ui->tableConsumo->hideColumn("cstCOFINS");
  ui->tableConsumo->hideColumn("vBCCOFINS");
  ui->tableConsumo->hideColumn("pCOFINS");
  ui->tableConsumo->hideColumn("vCOFINS");
}

void Estoque::on_tableEstoque_activated(const QModelIndex &) { exibirNota(); }

void Estoque::calcularRestante() {
  double quant = model.data(0, "quant").toDouble();

  for (int row = 0; row < modelViewConsumo.rowCount(); ++row) quant += modelViewConsumo.data(row, "quant").toDouble();

  ui->doubleSpinBoxRestante->setValue(quant);
  ui->doubleSpinBoxRestante->setSuffix(" " + model.data(0, "un").toString());
}

bool Estoque::viewRegisterById(const QString &idEstoque, bool showWindow) {
  if (idEstoque.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Estoque não encontrado!");
    return false;
  }

  model.setFilter("idEstoque = " + idEstoque);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + model.lastError().text());
    return false;
  }

  ui->tableEstoque->resizeColumnsToContents();

  if (not modelConsumo.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque consumo: " + modelConsumo.lastError().text());
    return false;
  }

  modelViewConsumo.setFilter("idEstoque = " + idEstoque);

  if (not modelViewConsumo.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela view consumo: " + modelViewConsumo.lastError().text());
    return false;
  }

  ui->tableConsumo->resizeColumnsToContents();

  calcularRestante();

  if (showWindow) show();

  return true;
}

void Estoque::on_pushButtonExibirNfe_clicked() { exibirNota(); }

void Estoque::exibirNota() {
  QSqlQuery query;
  query.prepare("SELECT xml FROM estoque e LEFT JOIN estoque_has_nfe ehn ON e.idEstoque = ehn.idEstoque LEFT JOIN nfe n ON ehn.idNFe = n.idNFe WHERE e.idEstoque = :idEstoque");
  query.bindValue(":idEstoque", model.data(0, "idEstoque"));

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando nfe: " + query.lastError().text());
    return;
  }

  while (query.next()) {
    auto *viewer = new XML_Viewer(this);
    viewer->exibirXML(query.value("xml").toByteArray());
  }
}

bool Estoque::criarConsumo(const int idVendaProduto, double quant) {
  if (model.filter().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Não setou idEstoque!");
    return false;
  }

  bool consumed = false;

  for (int row = 0; row < model.rowCount(); ++row) {
    if (quant == 0) {
      QSqlQuery queryQuant;
      queryQuant.prepare("SELECT quant FROM venda_has_produto WHERE idVendaProduto = :idVendaProduto");
      queryQuant.bindValue(":idVendaProduto", idVendaProduto);

      if (not queryQuant.exec() or not queryQuant.first()) {
        QMessageBox::critical(this, "Erro!", "Erro buscando em venda_has_produto: " + model.lastError().text());
        return false;
      }

      quant = queryQuant.value("quant").toDouble();
    }

    if (quant > model.data(row, "quant").toDouble()) continue;

    const int newRow = modelConsumo.rowCount();

    modelConsumo.insertRow(newRow);

    for (int column = 0, columnCount = model.columnCount(); column < columnCount; ++column) {
      const QString field = model.record().fieldName(column);
      const int index = modelConsumo.fieldIndex(field);
      const QVariant value = model.data(row, column);

      if (index != -1 and not modelConsumo.setData(newRow, index, value)) return false;
    }

    QSqlQuery query;
    query.prepare("SELECT UPPER(un) AS un, m2cx, pccx FROM produto WHERE idProduto = :idProduto");
    query.bindValue(":idProduto", model.data(row, "idProduto"));

    if (not query.exec() or not query.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando dados do produto: " + query.lastError().text());
      return false;
    }

    const QString un = query.value("un").toString();
    const double m2cx = query.value("m2cx").toDouble();
    const double pccx = query.value("pccx").toDouble();

    const double unCaixa = un == "M2" or un == "M²" or un == "ML" ? m2cx : pccx;

    const double caixas = qRound(quant / unCaixa * 100) / 100;

    const double proporcao = quant / model.data(row, "quant").toDouble();

    const double valor = model.data(row, "valor").toDouble() * proporcao;
    const double vBC = model.data(row, "vBC").toDouble() * proporcao;
    const double vICMS = model.data(row, "vICMS").toDouble() * proporcao;
    const double vBCST = model.data(row, "vBCST").toDouble() * proporcao;
    const double vICMSST = model.data(row, "vICMSST").toDouble() * proporcao;
    const double vBCPIS = model.data(row, "vBCPIS").toDouble() * proporcao;
    const double vPIS = model.data(row, "vPIS").toDouble() * proporcao;
    const double vBCCOFINS = model.data(row, "vBCCOFINS").toDouble() * proporcao;
    const double vCOFINS = model.data(row, "vCOFINS").toDouble() * proporcao;

    // -------------------------------------

    if (not modelConsumo.setData(newRow, "quant", quant * -1)) return false;
    if (not modelConsumo.setData(newRow, "caixas", caixas)) return false;
    if (not modelConsumo.setData(newRow, "quantUpd", DarkGreen)) return false; // DarkGreen
    if (not modelConsumo.setData(newRow, "idVendaProduto", idVendaProduto)) return false;
    if (not modelConsumo.setData(newRow, "idEstoque", model.data(row, "idEstoque"))) return false;
    if (not modelConsumo.setData(newRow, "status", "CONSUMO")) return false;

    if (not modelConsumo.setData(newRow, "valor", valor)) return false;
    if (not modelConsumo.setData(newRow, "vBC", vBC)) return false;
    if (not modelConsumo.setData(newRow, "vICMS", vICMS)) return false;
    if (not modelConsumo.setData(newRow, "vBCST", vBCST)) return false;
    if (not modelConsumo.setData(newRow, "vICMSST", vICMSST)) return false;
    if (not modelConsumo.setData(newRow, "vBCPIS", vBCPIS)) return false;
    if (not modelConsumo.setData(newRow, "vPIS", vPIS)) return false;
    if (not modelConsumo.setData(newRow, "vBCCOFINS", vBCCOFINS)) return false;
    if (not modelConsumo.setData(newRow, "vCOFINS", vCOFINS)) return false;

    consumed = true;
  }

  if (not modelConsumo.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados: " + modelConsumo.lastError().text());
    return false;
  }

  if (not consumed) {
    QMessageBox::critical(this, "Erro!", "Erro criando consumo!");
    return false;
  }

  return true;
}

void Estoque::on_tableEstoque_entered(const QModelIndex &) { ui->tableEstoque->resizeColumnsToContents(); }

void Estoque::on_tableConsumo_entered(const QModelIndex &) { ui->tableConsumo->resizeColumnsToContents(); }

// TODO: 1colocar o botao de desvincular consumo nesta tela
