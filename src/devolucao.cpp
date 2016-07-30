#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include "checkboxdelegate.h"
#include "devolucao.h"
#include "porcentagemdelegate.h"
#include "reaisdelegate.h"
#include "ui_devolucao.h"
#include "usersession.h"

Devolucao::Devolucao(QString idVenda, QWidget *parent) : QDialog(parent), ui(new Ui::Devolucao), idVenda(idVenda) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables(idVenda);
}

Devolucao::~Devolucao() { delete ui; }

void Devolucao::setupTables(QString idVenda) {
  modelProdutos.setTable("venda_has_produto");
  modelProdutos.setEditStrategy(SqlTableModel::OnManualSubmit);
  modelProdutos.setFilter("idVenda = '" + idVenda + "' AND status != 'DEVOLVIDO'");
  modelProdutos.setHeaderData("selecionado", "");
  modelProdutos.setHeaderData("status", "Status");
  modelProdutos.setHeaderData("fornecedor", "Fornecedor");
  modelProdutos.setHeaderData("produto", "Produto");
  modelProdutos.setHeaderData("obs", "Obs.");
  modelProdutos.setHeaderData("prcUnitario", "R$ Unit.");
  modelProdutos.setHeaderData("caixas", "Caixas");
  modelProdutos.setHeaderData("quant", "Quant.");
  modelProdutos.setHeaderData("un", "Un.");
  modelProdutos.setHeaderData("unCaixa", "Un. Caixa");
  modelProdutos.setHeaderData("codComercial", "Cód. Com.");
  modelProdutos.setHeaderData("formComercial", "Form. Com.");
  modelProdutos.setHeaderData("total", "Total");

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela venda_has_produto: " + modelProdutos.lastError().text());
  }

  ui->tableProdutos->setModel(&modelProdutos);
  ui->tableProdutos->hideColumn("descUnitario");
  ui->tableProdutos->hideColumn("parcial");
  ui->tableProdutos->hideColumn("desconto");
  ui->tableProdutos->hideColumn("parcialDesc");
  ui->tableProdutos->hideColumn("descGlobal");
  ui->tableProdutos->hideColumn("descGlobal");
  ui->tableProdutos->hideColumn("estoque_promocao");
  ui->tableProdutos->hideColumn("selecionado");
  ui->tableProdutos->hideColumn("idVendaProduto");
  ui->tableProdutos->hideColumn("idNfeSaida");
  ui->tableProdutos->hideColumn("idLoja");
  ui->tableProdutos->hideColumn("idVenda");
  ui->tableProdutos->hideColumn("item");
  ui->tableProdutos->hideColumn("idProduto");
  ui->tableProdutos->hideColumn("idCompra");
  ui->tableProdutos->hideColumn("dataPrevCompra");
  ui->tableProdutos->hideColumn("dataRealCompra");
  ui->tableProdutos->hideColumn("dataPrevConf");
  ui->tableProdutos->hideColumn("dataRealConf");
  ui->tableProdutos->hideColumn("dataPrevFat");
  ui->tableProdutos->hideColumn("dataRealFat");
  ui->tableProdutos->hideColumn("dataPrevColeta");
  ui->tableProdutos->hideColumn("dataRealColeta");
  ui->tableProdutos->hideColumn("dataPrevReceb");
  ui->tableProdutos->hideColumn("dataRealReceb");
  ui->tableProdutos->hideColumn("dataPrevEnt");
  ui->tableProdutos->hideColumn("dataRealEnt");
  ui->tableProdutos->setItemDelegateForColumn("prcUnitario", new ReaisDelegate(this));
  ui->tableProdutos->setItemDelegateForColumn("total", new ReaisDelegate(this));

  for (int row = 0; row < modelProdutos.rowCount(); ++row) {
    ui->tableProdutos->openPersistentEditor(row, "selecionado");
  }

  ui->tableProdutos->resizeColumnsToContents();

  modelDevolvidos.setTable("venda_has_produto");
  modelDevolvidos.setEditStrategy(SqlTableModel::OnManualSubmit);
  modelDevolvidos.setFilter("idVenda = '" + idVenda + "D'");
  modelDevolvidos.setHeaderData("selecionado", "");
  modelDevolvidos.setHeaderData("status", "Status");
  modelDevolvidos.setHeaderData("fornecedor", "Fornecedor");
  modelDevolvidos.setHeaderData("produto", "Produto");
  modelDevolvidos.setHeaderData("obs", "Obs.");
  modelDevolvidos.setHeaderData("prcUnitario", "R$ Unit.");
  modelDevolvidos.setHeaderData("caixas", "Caixas");
  modelDevolvidos.setHeaderData("quant", "Quant.");
  modelDevolvidos.setHeaderData("un", "Un.");
  modelDevolvidos.setHeaderData("unCaixa", "Un. Caixa");
  modelDevolvidos.setHeaderData("codComercial", "Cód. Com.");
  modelDevolvidos.setHeaderData("formComercial", "Form. Com.");
  modelDevolvidos.setHeaderData("total", "Total");

  if (not modelDevolvidos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela venda_has_produto: " + modelDevolvidos.lastError().text());
  }

  ui->tableDevolvidos->setModel(&modelDevolvidos);
  ui->tableDevolvidos->hideColumn("selecionado");
  ui->tableDevolvidos->hideColumn("idVendaProduto");
  ui->tableDevolvidos->hideColumn("idNfeSaida");
  ui->tableDevolvidos->hideColumn("idLoja");
  ui->tableDevolvidos->hideColumn("idVenda");
  ui->tableDevolvidos->hideColumn("item");
  ui->tableDevolvidos->hideColumn("idProduto");
  ui->tableDevolvidos->hideColumn("idCompra");
  ui->tableDevolvidos->hideColumn("dataPrevCompra");
  ui->tableDevolvidos->hideColumn("dataRealCompra");
  ui->tableDevolvidos->hideColumn("dataPrevConf");
  ui->tableDevolvidos->hideColumn("dataRealConf");
  ui->tableDevolvidos->hideColumn("dataPrevFat");
  ui->tableDevolvidos->hideColumn("dataRealFat");
  ui->tableDevolvidos->hideColumn("dataPrevColeta");
  ui->tableDevolvidos->hideColumn("dataRealColeta");
  ui->tableDevolvidos->hideColumn("dataPrevReceb");
  ui->tableDevolvidos->hideColumn("dataRealReceb");
  ui->tableDevolvidos->hideColumn("dataPrevEnt");
  ui->tableDevolvidos->hideColumn("dataRealEnt");
  ui->tableDevolvidos->setItemDelegateForColumn("prcUnitario", new ReaisDelegate(this));
  ui->tableDevolvidos->setItemDelegateForColumn("total", new ReaisDelegate(this));

  modelPagamentos.setTable("conta_a_receber_has_pagamento");
  modelPagamentos.setEditStrategy(SqlTableModel::OnManualSubmit);
  modelPagamentos.setHeaderData("tipo", "Tipo");
  modelPagamentos.setHeaderData("parcela", "Parcela");
  modelPagamentos.setHeaderData("valor", "R$");
  modelPagamentos.setHeaderData("dataPagamento", "Data");
  modelPagamentos.setHeaderData("observacao", "Obs.");
  modelPagamentos.setHeaderData("status", "Status");
  modelPagamentos.setHeaderData("representacao", "Representação");
  modelPagamentos.setFilter("idVenda = '" + idVenda + "D'");

  if (not modelPagamentos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela pagamentos: " + modelPagamentos.lastError().text());
  }

  ui->tablePagamentos->setModel(&modelPagamentos);
  ui->tablePagamentos->hideColumn("idVenda");
  ui->tablePagamentos->hideColumn("idLoja");
  ui->tablePagamentos->hideColumn("idPagamento");
  ui->tablePagamentos->hideColumn("dataEmissao");
  ui->tablePagamentos->hideColumn("dataRealizado");
  ui->tablePagamentos->hideColumn("valorReal");
  ui->tablePagamentos->hideColumn("tipoReal");
  ui->tablePagamentos->hideColumn("parcelaReal");
  ui->tablePagamentos->hideColumn("contaDestino");
  ui->tablePagamentos->hideColumn("tipoDet");
  ui->tablePagamentos->hideColumn("centroCusto");
  ui->tablePagamentos->hideColumn("grupo");
  ui->tablePagamentos->hideColumn("subGrupo");
  ui->tablePagamentos->setItemDelegateForColumn(modelPagamentos.fieldIndex("representacao"),
                                                new CheckBoxDelegate(this, true));

  for (int row = 0; row < modelPagamentos.rowCount(); ++row) {
    ui->tablePagamentos->openPersistentEditor(row, "representacao");
  }

  ui->tablePagamentos->resizeColumnsToContents();

  modelVenda.setTable("venda");
  modelVenda.setEditStrategy(SqlTableModel::OnManualSubmit);
  modelVenda.setFilter("idVenda = '" + idVenda + "'");

  if (not modelVenda.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela venda: " + modelVenda.lastError().text());
  }

  modelCliente.setTable("cliente");
  modelCliente.setEditStrategy(SqlTableModel::OnManualSubmit);
  modelCliente.setFilter("idCliente = " + modelVenda.data(0, "idCliente").toString());

  if (not modelCliente.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela cliente: " + modelCliente.lastError().text());
  }

  // mapper
  mapperItem.setModel(&modelProdutos);
  mapperItem.setSubmitPolicy(QDataWidgetMapper::ManualSubmit);

  mapperItem.addMapping(ui->doubleSpinBoxQuant, modelProdutos.fieldIndex("quant"), "value");
  mapperItem.addMapping(ui->spinBoxCaixas, modelProdutos.fieldIndex("caixas"), "value");
  mapperItem.addMapping(ui->lineEditUn, modelProdutos.fieldIndex("un"), "text");
}

void Devolucao::on_tableProdutos_clicked(const QModelIndex &index) {
  QSqlQuery query;
  query.prepare("SELECT m2cx, pccx FROM produto WHERE idProduto = :idProduto");
  query.bindValue(":idProduto", modelProdutos.data(index.row(), "idProduto"));

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(0, "Erro!", "Erro lendo tabela produto: " + query.lastError().text());
    return;
  }

  double prcUnitario =
      modelProdutos.data(index.row(), "total").toDouble() / modelProdutos.data(index.row(), "quant").toDouble();
  ui->doubleSpinBoxPrecoUn->setValue(prcUnitario);

  QString un = modelProdutos.data(index.row(), "un").toString();
  double quantCaixa =
      ((un == "M²") or (un == "M2") or (un == "ML")) ? query.value("m2cx").toDouble() : query.value("pccx").toDouble();
  ui->doubleSpinBoxQuant->setSingleStep(quantCaixa);

  mapperItem.setCurrentModelIndex(index);

  calcPrecoItemTotal();
}

void Devolucao::calcPrecoItemTotal() {
  ui->doubleSpinBoxTotalItem->setValue(ui->doubleSpinBoxQuant->value() * ui->doubleSpinBoxPrecoUn->value());
}

void Devolucao::on_spinBoxCaixas_valueChanged(const int &caixas) {
  const double quant = caixas * ui->doubleSpinBoxQuant->singleStep();

  if (ui->doubleSpinBoxQuant->value() != quant) ui->doubleSpinBoxQuant->setValue(quant);

  calcPrecoItemTotal();
}

void Devolucao::on_doubleSpinBoxQuant_valueChanged(double) {
  const int caixas = qRound(ui->doubleSpinBoxQuant->value() / ui->doubleSpinBoxQuant->singleStep());

  if (ui->spinBoxCaixas->value() != caixas) ui->spinBoxCaixas->setValue(caixas);
}

void Devolucao::on_doubleSpinBoxQuant_editingFinished() {
  ui->doubleSpinBoxQuant->setValue(ui->spinBoxCaixas->value() * ui->doubleSpinBoxQuant->singleStep());
}

void Devolucao::criarDevolucao(QModelIndexList list) {
  // gerar pedido negativo
  double subTotalBru = 0;
  double subTotalLiq = 0;

  for (auto item : list) {
    subTotalBru += modelProdutos.data(item.row(), "parcial").toDouble();
    subTotalLiq += modelProdutos.data(item.row(), "parcialDesc").toDouble();
  }

  int currentRow = 0;
  int newRow = modelVenda.rowCount();
  modelVenda.insertRow(newRow);

  for (int column = 0; column < modelVenda.columnCount(); ++column) {
    modelVenda.setData(newRow, column, modelVenda.data(currentRow, column));
  }

  const double desconto = modelVenda.data(newRow, "descontoReais").toDouble();

  modelVenda.setData(newRow, "idVenda", idVenda + "D");
  modelVenda.setData(newRow, "data", QDateTime::currentDateTime());
  modelVenda.setData(newRow, "subTotalBru", subTotalBru * -1);
  modelVenda.setData(newRow, "subTotalLiq", subTotalLiq * -1);
  modelVenda.setData(newRow, "frete", 0);
  modelVenda.setData(newRow, "total", (subTotalLiq - desconto) * -1);
  modelVenda.setData(newRow, "prazoEntrega", 0);
  modelVenda.setData(newRow, "status", "DEVOLUÇÃO");

  if (not modelVenda.submitAll()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro salvando dados do pedido de devolução: " + modelVenda.lastError().text());
    return;
  }
}

void Devolucao::inserirItens(QModelIndexList list) {
  double quant = modelProdutos.data(mapperItem.currentIndex(), "quant").toDouble();
  double quantDevolvida = ui->doubleSpinBoxQuant->value();
  double restante = quant - quantDevolvida;
  double step = ui->doubleSpinBoxQuant->singleStep();

  for (auto item : list) {
    int currentRow = item.row();

    // copiar linha para devolucao
    int newRow = modelProdutos.rowCount();
    modelProdutos.insertRow(newRow);

    for (int column = 0; column < modelProdutos.columnCount(); ++column) {
      if (modelProdutos.record().fieldName(column) == "idVendaProduto") continue;
      modelProdutos.setData(newRow, column, modelProdutos.data(currentRow, column));
    }

    modelProdutos.setData(newRow, "idVenda", idVenda + "D");
    modelProdutos.setData(newRow, "caixas", quantDevolvida / step * -1);
    modelProdutos.setData(newRow, "quant", quantDevolvida * -1);
    modelProdutos.setData(newRow, "parcial", ui->doubleSpinBoxTotalItem->value() * -1);
    modelProdutos.setData(newRow, "desconto", 0);
    modelProdutos.setData(newRow, "parcialDesc", ui->doubleSpinBoxTotalItem->value() * -1);
    modelProdutos.setData(newRow, "descGlobal", 0);
    modelProdutos.setData(newRow, "total", ui->doubleSpinBoxTotalItem->value() * -1);
    //------------------------------------

    if (restante > 0) {
      int newRow = modelProdutos.rowCount();
      modelProdutos.insertRow(newRow);

      for (int column = 0; column < modelProdutos.columnCount(); ++column) {
        if (modelProdutos.record().fieldName(column) == "idVendaProduto") continue;
        modelProdutos.setData(newRow, column, modelProdutos.data(currentRow, column));
      }

      modelProdutos.setData(newRow, "idVenda", idVenda);
      modelProdutos.setData(newRow, "caixas", restante / step);
      modelProdutos.setData(newRow, "quant", restante);
      double parcial =
          modelProdutos.data(newRow, "quant").toDouble() * modelProdutos.data(newRow, "prcUnitario").toDouble();
      modelProdutos.setData(newRow, "parcial", parcial);
      double parcialDesc = parcial * (1 - (modelProdutos.data(newRow, "desconto").toDouble() / 100));
      modelProdutos.setData(newRow, "parcialDesc", parcialDesc);
      double total = parcialDesc * (1 - (modelProdutos.data(newRow, "descGlobal").toDouble() / 100));
      modelProdutos.setData(newRow, "total", total);
    }

    modelProdutos.setData(currentRow, "caixas", quantDevolvida / step);
    modelProdutos.setData(currentRow, "quant", quantDevolvida);
    double parcial2 =
        modelProdutos.data(currentRow, "quant").toDouble() * modelProdutos.data(currentRow, "prcUnitario").toDouble();
    modelProdutos.setData(currentRow, "parcial", parcial2);
    double parcialDesc2 = parcial2 * (1 - (modelProdutos.data(currentRow, "desconto").toDouble() / 100));
    modelProdutos.setData(currentRow, "parcialDesc", parcialDesc2);
    double total2 = parcialDesc2 * (1 - (modelProdutos.data(currentRow, "descGlobal").toDouble() / 100));
    modelProdutos.setData(currentRow, "total", total2);
    modelProdutos.setData(currentRow, "status", "DEVOLVIDO");
  }

  if (not modelProdutos.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando produtos da devolução: " + modelProdutos.lastError().text());
    return;
  }
}

void Devolucao::criarContas() {
  int newRowPag = modelPagamentos.rowCount();
  modelPagamentos.insertRow(newRowPag);

  modelPagamentos.setData(newRowPag, "dataEmissao", QDate::currentDate());
  modelPagamentos.setData(newRowPag, "idVenda", idVenda + "D");
  modelPagamentos.setData(newRowPag, "idLoja", UserSession::idLoja());
  modelPagamentos.setData(newRowPag, "valor", ui->doubleSpinBoxCredito->value() * -1);
  modelPagamentos.setData(newRowPag, "tipo", "1. Conta Cliente");
  modelPagamentos.setData(newRowPag, "parcela", 1);
  modelPagamentos.setData(newRowPag, "observacao", "");
  //----------------

  if (not modelPagamentos.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando pagamentos: " + modelProdutos.lastError().text());
    return;
  }
}

void Devolucao::salvarCredito() {
  double credito = modelCliente.data(0, "credito").toDouble();
  credito += ui->doubleSpinBoxCredito->value();
  modelCliente.setData(0, "credito", credito);

  if (not modelCliente.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando crédito do cliente: " + modelCliente.lastError().text());
    return;
  }
}

void Devolucao::on_pushButtonDevolverItem_clicked() {
  auto list = ui->tableProdutos->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhuma linha selecionada!");
    return;
  }

  if (modelDevolvidos.rowCount() == 0) {
    criarDevolucao(list);
    modelDevolvidos.select();
  }

  inserirItens(list);
  atualizarDevolucao();
  criarContas();
  salvarCredito();

  modelProdutos.select();
  modelDevolvidos.select();
  modelPagamentos.select();

  ui->tableProdutos->resizeColumnsToContents();
  ui->tableDevolvidos->resizeColumnsToContents();
  ui->tablePagamentos->resizeColumnsToContents();

  QMessageBox::information(this, "Aviso!", "Devolução realizada com sucesso!");
}

void Devolucao::atualizarDevolucao() {
  QSqlQuery query;
  query.prepare("SELECT parcial, parcialDesc FROM venda_has_produto WHERE idVenda = :idVenda");
  query.bindValue(":idVenda", idVenda + "D");

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados da devolução: " + query.lastError().text());
    return;
  }

  double subTotalBru = 0;
  double subTotalLiq = 0;

  while (query.next()) {
    subTotalBru += query.value("parcial").toDouble();
    subTotalLiq += query.value("parcialDesc").toDouble();
  }

  query.prepare("UPDATE venda SET subTotalBru = :subTotalBru, subTotalLiq = :subTotalLiq, frete = :frete, total = "
                ":total WHERE idVenda = :idVenda");
  query.bindValue(":subTotalBru", subTotalBru);
  query.bindValue(":subTotalLiq", subTotalLiq);
  query.bindValue(":frete", 0);
  query.bindValue(":total", subTotalLiq);
  query.bindValue(":idVenda", idVenda + "D");

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando devolução: " + query.lastError().text());
    return;
  }
}

void Devolucao::on_doubleSpinBoxTotalItem_valueChanged(double value) {
  if (ui->groupBoxCredito->isChecked()) {
    ui->doubleSpinBoxCredito->setMaximum(value);
    ui->doubleSpinBoxCredito->setValue(value);
  }
}

void Devolucao::on_groupBoxCredito_toggled(bool) {
  if (ui->groupBoxCredito->isChecked()) {
    ui->doubleSpinBoxCredito->setValue(ui->doubleSpinBoxTotalItem->value());
    ui->doubleSpinBoxCredito->setMaximum(ui->doubleSpinBoxTotalItem->value());
  } else {
    ui->doubleSpinBoxCredito->setValue(0);
  }
}
