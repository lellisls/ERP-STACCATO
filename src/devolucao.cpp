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

Devolucao::Devolucao(QString idVenda, QWidget *parent) : QDialog(parent), idVenda(idVenda), ui(new Ui::Devolucao) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables();
}

Devolucao::~Devolucao() { delete ui; }

void Devolucao::setupTables() {
  modelProdutos.setTable("venda_has_produto");
  modelProdutos.setEditStrategy(SqlTableModel::OnManualSubmit);

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

  modelProdutos.setFilter("idVenda = '" + idVenda + "' AND status != 'DEVOLVIDO'");

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela venda_has_produto: " + modelProdutos.lastError().text());
  }

  ui->tableProdutos->setModel(&modelProdutos);
  ui->tableProdutos->hideColumn("entregou");
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

  for (int row = 0; row < modelProdutos.rowCount(); ++row) ui->tableProdutos->openPersistentEditor(row, "selecionado");

  ui->tableProdutos->resizeColumnsToContents();

  modelDevolvidos.setTable("venda_has_produto");
  modelDevolvidos.setEditStrategy(SqlTableModel::OnManualSubmit);

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

  modelDevolvidos.setFilter("idVenda = '" + idVenda + "D'");

  if (not modelDevolvidos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela venda_has_produto: " + modelDevolvidos.lastError().text());
  }

  ui->tableDevolvidos->setModel(&modelDevolvidos);
  ui->tableDevolvidos->hideColumn("entregou");
  ui->tableDevolvidos->hideColumn("descUnitario");
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
  ui->tablePagamentos->hideColumn("taxa");
  ui->tablePagamentos->hideColumn("contraParte");
  ui->tablePagamentos->hideColumn("comissao");
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
  const double total = modelProdutos.data(index.row(), "total").toDouble();
  const double quant = modelProdutos.data(index.row(), "quant").toDouble();
  const double caixas = modelProdutos.data(index.row(), "caixas").toInt();

  ui->doubleSpinBoxQuant->setSingleStep(quant / caixas);

  ui->doubleSpinBoxQuant->setMaximum(quant);
  ui->spinBoxCaixas->setMaximum(caixas);

  mapperItem.setCurrentModelIndex(index);

  ui->doubleSpinBoxPrecoUn->setValue(total / quant);

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

bool Devolucao::criarDevolucao() {
  const int newRow = modelVenda.rowCount();
  if (not modelVenda.insertRow(newRow)) return false;

  for (int column = 0; column < modelVenda.columnCount(); ++column) {
    if (not modelVenda.setData(newRow, column, modelVenda.data(0, column))) return false;
  }

  if (not modelVenda.setData(newRow, "idVenda", idVenda + "D")) return false;
  if (not modelVenda.setData(newRow, "data", QDateTime::currentDateTime())) return false;
  if (not modelVenda.setData(newRow, "subTotalBru", 0)) return false;
  if (not modelVenda.setData(newRow, "subTotalLiq", 0)) return false;
  if (not modelVenda.setData(newRow, "frete", 0)) return false;
  if (not modelVenda.setData(newRow, "total", 0)) return false;
  if (not modelVenda.setData(newRow, "prazoEntrega", 0)) return false;

  if (not modelVenda.submitAll()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro salvando dados do pedido de devolução: " + modelVenda.lastError().text());
    return false;
  }

  return true;
}

bool Devolucao::inserirItens(const QModelIndexList &list) {
  const double quant = modelProdutos.data(mapperItem.currentIndex(), "quant").toDouble();
  const double quantDevolvida = ui->doubleSpinBoxQuant->value();
  const double restante = quant - quantDevolvida;
  const double step = ui->doubleSpinBoxQuant->singleStep();

  for (auto const &item : list) {
    int currentRow = item.row();

    // copiar linha para devolucao
    int newRow = modelProdutos.rowCount();
    if (not modelProdutos.insertRow(newRow)) return false;

    for (int column = 0; column < modelProdutos.columnCount(); ++column) {
      if (modelProdutos.record().fieldName(column) == "idVendaProduto") continue;
      if (not modelProdutos.setData(newRow, column, modelProdutos.data(currentRow, column))) return false;
    }

    if (not modelProdutos.setData(newRow, "idVenda", idVenda + "D")) return false;
    if (not modelProdutos.setData(newRow, "caixas", quantDevolvida / step * -1)) return false;
    if (not modelProdutos.setData(newRow, "quant", quantDevolvida * -1)) return false;
    if (not modelProdutos.setData(newRow, "parcial", ui->doubleSpinBoxTotalItem->value() * -1)) return false;
    if (not modelProdutos.setData(newRow, "desconto", 0)) return false;
    if (not modelProdutos.setData(newRow, "parcialDesc", ui->doubleSpinBoxTotalItem->value() * -1)) return false;
    if (not modelProdutos.setData(newRow, "descGlobal", 0)) return false;
    if (not modelProdutos.setData(newRow, "total", ui->doubleSpinBoxTotalItem->value() * -1)) return false;
    //------------------------------------

    if (restante > 0) {
      const int newRow = modelProdutos.rowCount();
      if (not modelProdutos.insertRow(newRow)) return false;

      for (int column = 0; column < modelProdutos.columnCount(); ++column) {
        if (modelProdutos.record().fieldName(column) == "idVendaProduto") continue;
        if (not modelProdutos.setData(newRow, column, modelProdutos.data(currentRow, column))) return false;
      }

      if (not modelProdutos.setData(newRow, "idVenda", idVenda)) return false;
      if (not modelProdutos.setData(newRow, "caixas", restante / step)) return false;
      if (not modelProdutos.setData(newRow, "quant", restante)) return false;

      const double quant2 = modelProdutos.data(newRow, "quant").toDouble();
      const double prcUnitario = modelProdutos.data(newRow, "prcUnitario").toDouble();
      const double parcial = quant2 * prcUnitario;

      if (not modelProdutos.setData(newRow, "parcial", parcial)) return false;

      const double desconto = modelProdutos.data(newRow, "desconto").toDouble();
      const double parcialDesc = parcial * (1 - (desconto / 100));

      if (not modelProdutos.setData(newRow, "parcialDesc", parcialDesc)) return false;

      const double descGlobal = modelProdutos.data(newRow, "descGlobal").toDouble();
      const double total = parcialDesc * (1 - (descGlobal / 100));

      if (not modelProdutos.setData(newRow, "total", total)) return false;
    }

    if (not modelProdutos.setData(currentRow, "caixas", quantDevolvida / step)) return false;
    if (not modelProdutos.setData(currentRow, "quant", quantDevolvida)) return false;

    const double quant3 = modelProdutos.data(currentRow, "quant").toDouble();
    const double prcUnitario = modelProdutos.data(currentRow, "prcUnitario").toDouble();
    const double parcial2 = quant3 * prcUnitario;

    if (not modelProdutos.setData(currentRow, "parcial", parcial2)) return false;

    const double desconto = modelProdutos.data(currentRow, "desconto").toDouble();
    const double parcialDesc2 = parcial2 * (1 - (desconto / 100));

    if (not modelProdutos.setData(currentRow, "parcialDesc", parcialDesc2)) return false;

    const double descGlobal = modelProdutos.data(currentRow, "descGlobal").toDouble();
    const double total2 = parcialDesc2 * (1 - (descGlobal / 100));

    if (not modelProdutos.setData(currentRow, "total", total2)) return false;
    if (not modelProdutos.setData(currentRow, "status", "DEVOLVIDO")) return false;
  }

  if (not modelProdutos.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando produtos da devolução: " + modelProdutos.lastError().text());
    return false;
  }

  return true;
}

bool Devolucao::criarContas() {
  // TODO: se for representacao criar linha com comissao proporcional negativa
  if (not ui->groupBoxCredito->isChecked()) return true;

  int newRowPag = modelPagamentos.rowCount();
  if (not modelPagamentos.insertRow(newRowPag)) return false;

  if (not modelPagamentos.setData(newRowPag, "contraParte", modelCliente.data(0, "nome_razao"))) return false;
  if (not modelPagamentos.setData(newRowPag, "dataEmissao", QDate::currentDate())) return false;
  if (not modelPagamentos.setData(newRowPag, "idVenda", idVenda + "D")) return false;
  if (not modelPagamentos.setData(newRowPag, "idLoja", UserSession::idLoja())) return false;
  if (not modelPagamentos.setData(newRowPag, "valor", ui->doubleSpinBoxCredito->value() * -1)) return false;
  if (not modelPagamentos.setData(newRowPag, "tipo", "1. Conta Cliente")) return false;
  if (not modelPagamentos.setData(newRowPag, "parcela", 1)) return false;
  if (not modelPagamentos.setData(newRowPag, "observacao", "")) return false;
  //----------------

  if (not modelPagamentos.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando pagamentos: " + modelProdutos.lastError().text());
    return false;
  }

  return true;
}

bool Devolucao::salvarCredito() {
  const double credito = modelCliente.data(0, "credito").toDouble() + ui->doubleSpinBoxCredito->value();

  if (not modelCliente.setData(0, "credito", credito)) return false;

  if (not modelCliente.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando crédito do cliente: " + modelCliente.lastError().text());
    return false;
  }

  return true;
}

bool Devolucao::devolverItem() {
  const auto list = ui->tableProdutos->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhuma linha selecionada!");
    return false;
  }

  if (modelDevolvidos.rowCount() == 0) {
    // TODO: diferenciar devolucoes feitas em meses diferentes (ALPH-...D1 D2 ...?)
    if (not criarDevolucao()) return false;

    if (not modelDevolvidos.select()) {
      QMessageBox::critical(this, "Erro!", "Erro lendo tabela devolvidos: " + modelDevolvidos.lastError().text());
      return false;
    }
  }

  if (not inserirItens(list)) return false;
  if (not atualizarDevolucao()) return false;
  if (not criarContas()) return false;
  if (not salvarCredito()) return false;

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela produtos: " + modelProdutos.lastError().text());
    return false;
  }

  if (not modelDevolvidos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela devolvidos: " + modelDevolvidos.lastError().text());
    return false;
  }

  if (not modelPagamentos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela pagamentos: " + modelPagamentos.lastError().text());
    return false;
  }

  ui->tableProdutos->resizeColumnsToContents();
  ui->tableDevolvidos->resizeColumnsToContents();
  ui->tablePagamentos->resizeColumnsToContents();

  return true;
}

void Devolucao::on_pushButtonDevolverItem_clicked() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not devolverItem()) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  QMessageBox::information(this, "Aviso!", "Devolução realizada com sucesso!");
}

bool Devolucao::atualizarDevolucao() {
  QSqlQuery query;
  query.prepare("SELECT parcial, parcialDesc FROM venda_has_produto WHERE idVenda = :idVenda");
  query.bindValue(":idVenda", idVenda + "D");

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados da devolução: " + query.lastError().text());
    return false;
  }

  double subTotalBru = 0;
  double subTotalLiq = 0;

  while (query.next()) {
    subTotalBru += query.value("parcial").toDouble();
    subTotalLiq += query.value("parcialDesc").toDouble();
  }

  query.prepare("UPDATE venda SET subTotalBru = :subTotalBru, subTotalLiq = :subTotalLiq, "
                "total = :total WHERE idVenda = :idVenda");
  query.bindValue(":subTotalBru", subTotalBru);
  query.bindValue(":subTotalLiq", subTotalLiq);
  query.bindValue(":total", subTotalLiq);
  query.bindValue(":idVenda", idVenda + "D");

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando devolução: " + query.lastError().text());
    return false;
  }

  return true;
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

// TODO: devolucoes separadas juntando na mesma linha do fluxo
// TODO: ao fazer devolucao recalcular comissao? (nao estou gerando comissao negativa?)

// TODO: devolucao de estoque esta gerando consumo??? (devolucao estava gerando consumo de estoque ao contrario? talvez
// apenas cancelar/devolver o consumo)

// TODO: devolucao esta copiando descontoPorc e descontoReais
// TODO: comparar orcamento_has_produto com venda_has_produto e verificar quants diferentes (por causa do problema da
// devolucao que estava duplicando linhas e alterando valores)
// TODO: colocar coluna 'devolucao' para guardar flag no BD indicando quais vendas são de devolucao (assim posso filtrar
// independente do id da devolucao, se acaba com 'D' ou nao)
