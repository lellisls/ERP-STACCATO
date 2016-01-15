#include <QMessageBox>
#include <QSqlError>
#include <QSqlRecord>

#include "cadastrocliente.h"
#include "checkboxdelegate.h"
#include "doubledelegate.h"
#include "excel.h"
#include "impressao.h"
#include "orcamento.h"
#include "ui_venda.h"
#include "usersession.h"
#include "venda.h"

Venda::Venda(QWidget *parent) : RegisterDialog("venda", "idVenda", parent), ui(new Ui::Venda) {
  ui->setupUi(this);

  for (const auto *line : findChildren<QLineEdit *>()) {
    connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);
  }

  setupTables();

  ui->itemBoxCliente->setSearchDialog(SearchDialog::cliente(this));
  ui->itemBoxCliente->setRegisterDialog(new CadastroCliente(this));
  ui->itemBoxVendedor->setSearchDialog(SearchDialog::vendedor(this));
  ui->itemBoxProfissional->setSearchDialog(SearchDialog::profissional(this));
  ui->itemBoxEndereco->setSearchDialog(SearchDialog::enderecoCliente(this));
  ui->itemBoxEnderecoFat->setSearchDialog(SearchDialog::enderecoCliente(this));

  // NOTE: make this runtime changeable
  QStringList list{"Escolha uma opção!", "Cartão de débito", "Cartão de crédito", "Cheque",
                   "Dinheiro",           "Boleto",           "Transf. Banc."};

  ui->comboBoxPgt1->insertItems(0, list);
  ui->comboBoxPgt2->insertItems(0, list);
  ui->comboBoxPgt3->insertItems(0, list);

  ui->dateEditPgt1->setDate(QDate::currentDate());
  ui->dateEditPgt2->setDate(QDate::currentDate());
  ui->dateEditPgt3->setDate(QDate::currentDate());

  setupMapper();
  newRegister();

  // NOTE: make this work
  ui->splitter->setStretchFactor(0, 1);
  ui->splitter->setStretchFactor(1, 0);

  connect(ui->checkBoxRep1, &QCheckBox::stateChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->checkBoxRep2, &QCheckBox::stateChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->checkBoxRep3, &QCheckBox::stateChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->comboBoxPgt1Parc, &QComboBox::currentTextChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->comboBoxPgt2Parc, &QComboBox::currentTextChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->comboBoxPgt3Parc, &QComboBox::currentTextChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->dateEditPgt1, &QDateEdit::dateChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->dateEditPgt2, &QDateEdit::dateChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->dateEditPgt3, &QDateEdit::dateChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->lineEditPgt1, &QLineEdit::textChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->lineEditPgt2, &QLineEdit::textChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->lineEditPgt3, &QLineEdit::textChanged, this, &Venda::montarFluxoCaixa);

  show();
}

Venda::~Venda() { delete ui; }

void Venda::setupTables() {
  modelItem.setTable("venda_has_produto");
  modelItem.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelItem.setHeaderData("selecionado", "");
  modelItem.setHeaderData("fornecedor", "Fornecedor");
  modelItem.setHeaderData("produto", "Produto");
  modelItem.setHeaderData("obs", "Obs.");
  modelItem.setHeaderData("prcUnitario", "Preço/Un");
  modelItem.setHeaderData("caixas", "Caixas");
  modelItem.setHeaderData("quant", "Quant.");
  modelItem.setHeaderData("un", "Un.");
  modelItem.setHeaderData("unCaixa", "Un./Caixa");
  modelItem.setHeaderData("codComercial", "Cód. Com.");
  modelItem.setHeaderData("formComercial", "Form. Com.");
  modelItem.setHeaderData("parcial", "Parcial");
  modelItem.setHeaderData("desconto", "Desconto");
  modelItem.setHeaderData("parcialDesc", "Desc. Parc.");
  modelItem.setHeaderData("descGlobal", "Desc. Glob.");
  modelItem.setHeaderData("total", "Total");
  modelItem.setHeaderData("status", "Status");
  modelItem.setHeaderData("dataPrevCompra", "Prev. Compra");
  modelItem.setHeaderData("dataRealCompra", "Data Compra");
  modelItem.setHeaderData("dataPrevConf", "Prev. Confirm.");
  modelItem.setHeaderData("dataRealConf", "Data Confirm.");
  modelItem.setHeaderData("dataPrevFat", "Prev. Fat.");
  modelItem.setHeaderData("dataRealFat", "Data Fat.");
  modelItem.setHeaderData("dataPrevColeta", "Prev. Coleta");
  modelItem.setHeaderData("dataRealColeta", "Data Coleta");
  modelItem.setHeaderData("dataPrevReceb", "Prev. Receb.");
  modelItem.setHeaderData("dataRealReceb", "Data Receb.");
  modelItem.setHeaderData("dataPrevEnt", "Prev. Ent.");
  modelItem.setHeaderData("dataRealEnt", "Data Ent.");

  if (not modelItem.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela venda_has_produto: " + modelItem.lastError().text());
    return;
  }

  ui->tableVenda->setModel(&modelItem);
  ui->tableVenda->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableVenda->hideColumn("idNfeSaida");
  ui->tableVenda->hideColumn("idVendaProduto");
  ui->tableVenda->hideColumn("selecionado");
  ui->tableVenda->hideColumn("idVenda");
  ui->tableVenda->hideColumn("idLoja");
  ui->tableVenda->hideColumn("idProduto");
  ui->tableVenda->hideColumn("item");
  ui->tableVenda->setItemDelegate(new DoubleDelegate(this));
  ui->tableVenda->setItemDelegateForColumn("selecionado", new CheckBoxDelegate(this));

  modelFluxoCaixa.setTable("conta_a_receber_has_pagamento");
  modelFluxoCaixa.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelFluxoCaixa.setHeaderData("tipo", "Tipo");
  modelFluxoCaixa.setHeaderData("parcela", "Parcela");
  modelFluxoCaixa.setHeaderData("valor", "R$");
  modelFluxoCaixa.setHeaderData("dataPagamento", "Data");
  modelFluxoCaixa.setHeaderData("observacao", "Obs.");
  modelFluxoCaixa.setHeaderData("status", "Status");
  modelFluxoCaixa.setHeaderData("representacao", "Representação");

  if (not modelFluxoCaixa.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela conta_a_receber_has_pagamento: " + modelFluxoCaixa.lastError().text());
    return;
  }

  ui->tableFluxoCaixa->setModel(&modelFluxoCaixa);
  ui->tableFluxoCaixa->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableFluxoCaixa->hideColumn("idVenda");
  ui->tableFluxoCaixa->hideColumn("idLoja");
  ui->tableFluxoCaixa->hideColumn("idPagamento");
  ui->tableFluxoCaixa->hideColumn("dataEmissao");
  ui->tableFluxoCaixa->hideColumn("dataRealizado");
  ui->tableFluxoCaixa->hideColumn("valorReal");
  ui->tableFluxoCaixa->hideColumn("tipoReal");
  ui->tableFluxoCaixa->hideColumn("parcelaReal");
  ui->tableFluxoCaixa->hideColumn("contaDestino");
  ui->tableFluxoCaixa->hideColumn("tipoDet");
  ui->tableFluxoCaixa->hideColumn("centroCusto");
  ui->tableFluxoCaixa->hideColumn("grupo");
  ui->tableFluxoCaixa->hideColumn("subGrupo");
  ui->tableFluxoCaixa->setItemDelegate(new DoubleDelegate(this));
  ui->tableFluxoCaixa->setItemDelegateForColumn(modelFluxoCaixa.fieldIndex("representacao"),
                                                new CheckBoxDelegate(this));
}

void Venda::resetarPagamentos() {
  ui->doubleSpinBoxTotalPag->setValue(ui->doubleSpinBoxTotal->value());
  ui->doubleSpinBoxPgt1->setMaximum(ui->doubleSpinBoxTotal->value());
  ui->doubleSpinBoxPgt1->setValue(ui->doubleSpinBoxTotal->value());
  ui->doubleSpinBoxPgt2->setValue(0);
  ui->doubleSpinBoxPgt3->setValue(0);
  ui->doubleSpinBoxPgt2->setMaximum(0);
  ui->doubleSpinBoxPgt3->setMaximum(0);
  ui->comboBoxPgt1->setCurrentIndex(0);
  ui->comboBoxPgt2->setCurrentIndex(0);
  ui->comboBoxPgt3->setCurrentIndex(0);

  ui->comboBoxPgt1Parc->setCurrentIndex(0);
  ui->comboBoxPgt2Parc->setCurrentIndex(0);
  ui->comboBoxPgt3Parc->setCurrentIndex(0);

  ui->comboBoxPgt2->setDisabled(true);
  ui->comboBoxPgt3->setDisabled(true);
  ui->comboBoxPgt2Parc->setDisabled(true);
  ui->comboBoxPgt3Parc->setDisabled(true);
  ui->dateEditPgt2->setDisabled(true);
  ui->dateEditPgt3->setDisabled(true);

  ui->doubleSpinBoxPgt2->setDisabled(true);
  ui->doubleSpinBoxPgt3->setDisabled(true);

  montarFluxoCaixa();
}

void Venda::fecharOrcamento(const QString &idOrcamento) {
  m_idOrcamento = idOrcamento;

  const int row = model.rowCount();
  model.insertRow(row);
  mapper.toLast();

  QSqlQuery queryProdutos;
  queryProdutos.prepare("SELECT * FROM orcamento_has_produto WHERE idOrcamento = :idOrcamento");
  queryProdutos.bindValue(":idOrcamento", idOrcamento);

  if (not queryProdutos.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando produtos: " + queryProdutos.lastError().text());
    return;
  }

  QSqlQuery queryOrc;
  queryOrc.prepare("SELECT * FROM orcamento WHERE idOrcamento = :idOrcamento");
  queryOrc.bindValue(":idOrcamento", idOrcamento);

  if (not queryOrc.exec() or not queryOrc.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando orçamento: " + queryOrc.lastError().text());
    return;
  }

  ui->itemBoxVendedor->setValue(queryOrc.value("idUsuario"));

  generateId();
  QString idVenda = ui->lineEditVenda->text();

  modelItem.setFilter("idVenda = '" + idVenda + "'");

  while (queryProdutos.next()) {
    const int rowItem = modelItem.rowCount();
    modelItem.insertRow(rowItem);

    if (not modelItem.setData(rowItem, "idVenda", queryProdutos.value("idOrcamento"))) return;

    for (int column = 0, columnCount = queryProdutos.record().count(); column < columnCount; ++column) {
      QString field = queryProdutos.record().fieldName(column);

      if (field == "idVenda") continue;
      if (modelItem.fieldIndex(field) == -1) continue;
      if (not modelItem.setData(rowItem, field, queryProdutos.value(field))) return;
    }

    if (not modelItem.setData(rowItem, "idVenda", idVenda)) return;
    if (not modelItem.setData(rowItem, "status", "PENDENTE")) return;
  }

  for (int column = 0; column < queryOrc.record().count(); ++column) {
    QString field = queryOrc.record().fieldName(column);

    if (field == "idVenda") continue;
    if (model.fieldIndex(field) == -1) continue;
    if (not model.setData(mapper.currentIndex(), field, queryOrc.value(column))) return;
  }

  if (not model.setData(row, "idVenda", idVenda)) return;

  resetarPagamentos();

  modelFluxoCaixa.setFilter("idVenda = '" + idVenda + "'");
  ui->itemBoxEndereco->searchDialog()->setFilter("idCliente = " + queryOrc.value("idCliente").toString() +
                                                 " AND desativado = FALSE");
  ui->itemBoxEnderecoFat->searchDialog()->setFilter("idCliente = " + queryOrc.value("idCliente").toString() +
                                                    " AND desativado = FALSE");

  ui->lineEditVenda->setText(idVenda);
  ui->itemBoxCliente->setValue(queryOrc.value("idCliente"));
  ui->itemBoxProfissional->setValue(queryOrc.value("idProfissional"));
  ui->itemBoxEndereco->setValue(queryOrc.value("idEnderecoEntrega"));
  ui->itemBoxEnderecoFat->setValue(queryOrc.value("idEnderecoFaturamento"));

  ui->dateTimeEditOrc->setDateTime(data("data").toDateTime());
  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

  ui->tableVenda->resizeColumnsToContents();

  QSqlQuery queryFrete;
  queryFrete.prepare("SELECT * FROM loja WHERE idLoja = :idLoja");
  queryFrete.bindValue(":idLoja", UserSession::fromLoja("usuario.idLoja", ui->itemBoxVendedor->text()));

  if (not queryFrete.exec() or not queryFrete.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando parâmetros do frete: " + queryFrete.lastError().text());
    return;
  }

  minimoFrete = queryFrete.value("valorMinimoFrete").toDouble();
  porcFrete = queryFrete.value("porcentagemFrete").toDouble();

  if (not model.data(mapper.currentIndex(), "representacao").toBool()) {
    ui->checkBoxRep1->hide();
    ui->checkBoxRep2->hide();
    ui->checkBoxRep3->hide();
  }
}

bool Venda::verifyFields() {
  if (ui->spinBoxPrazoEntrega->value() == 0) {
    QMessageBox::critical(this, "Erro!", "Por favor preencha o prazo de entrega.");
    ui->spinBoxPrazoEntrega->setFocus();
    return false;
  }

  if (not qFuzzyCompare(ui->doubleSpinBoxPgt1->value() + ui->doubleSpinBoxPgt2->value() +
                        ui->doubleSpinBoxPgt3->value(),
                        ui->doubleSpinBoxTotalPag->value())) {
    QMessageBox::critical(this, "Erro!", "Soma dos pagamentos não é igual ao total! Favor verificar.");
    return false;
  }

  if (ui->doubleSpinBoxPgt1->value() > 0 and ui->comboBoxPgt1->currentText() == "Escolha uma opção!") {
    QMessageBox::critical(this, "Erro!", "Por favor escolha a forma de pagamento 1.");
    ui->doubleSpinBoxPgt1->setFocus();
    return false;
  }
  if (ui->doubleSpinBoxPgt2->value() > 0 and ui->comboBoxPgt2->currentText() == "Escolha uma opção!") {
    QMessageBox::critical(this, "Erro!", "Por favor escolha a forma de pagamento 2.");
    ui->doubleSpinBoxPgt2->setFocus();
    return false;
  }
  if (ui->doubleSpinBoxPgt3->value() > 0 and ui->comboBoxPgt3->currentText() == "Escolha uma opção!") {
    QMessageBox::critical(this, "Erro!", "Por favor escolha a forma de pagamento 3.");
    ui->doubleSpinBoxPgt3->setFocus();
    return false;
  }

  if (ui->itemBoxEnderecoFat->text().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Deve selecionar um endereço de faturamento!");
    ui->itemBoxEnderecoFat->setFocus();
    return false;
  }

  return true;
}

void Venda::calcPrecoGlobalTotal() {
  double subTotalItens = 0.;
  double subTotalBruto = 0.;

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    const double itemBruto = modelItem.data(row, "quant").toDouble() * modelItem.data(row, "prcUnitario").toDouble();
    const double descItem = modelItem.data(row, "desconto").toDouble() / 100.;
    const double stItem = itemBruto * (1. - descItem);
    subTotalBruto += itemBruto;
    subTotalItens += stItem;
    modelItem.setData(row, "parcial", itemBruto);
    modelItem.setData(row, "parcialDesc", stItem);
  }

  double descGlobal = ui->doubleSpinBoxDescontoGlobal->value() / 100.;
  double subTotal = subTotalItens * (1. - descGlobal);

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    modelItem.setData(row, "descGlobal", descGlobal * 100.);
    modelItem.setData(row, "total", modelItem.data(row, "parcialDesc").toDouble() * (1 - descGlobal));
  }

  ui->doubleSpinBoxSubTotalBruto->setValue(subTotalBruto);
  ui->doubleSpinBoxSubTotalLiq->setValue(subTotalItens);
  if (not isBlockedReais) ui->doubleSpinBoxDescontoGlobalReais->setValue(subTotalItens - subTotal);
  ui->doubleSpinBoxDescontoGlobalReais->setMaximum(ui->doubleSpinBoxSubTotalLiq->value());
  if (not isBlockedTotal) ui->doubleSpinBoxTotal->setValue(subTotal + ui->doubleSpinBoxFrete->value());
  ui->doubleSpinBoxTotal->setMinimum(ui->doubleSpinBoxFrete->value());

  resetarPagamentos();
}

void Venda::clearFields() {}

void Venda::setupMapper() {
  addMapping(ui->lineEditVenda, "idVenda");
  addMapping(ui->itemBoxCliente, "idCliente", "value");
  addMapping(ui->itemBoxEndereco, "idEnderecoEntrega", "value");
  addMapping(ui->itemBoxEnderecoFat, "idEnderecoFaturamento", "value");
  addMapping(ui->itemBoxVendedor, "idUsuario", "value");
  addMapping(ui->itemBoxProfissional, "idProfissional", "value");
  addMapping(ui->dateTimeEditOrc, "dataOrc");
  addMapping(ui->dateTimeEdit, "data");
  addMapping(ui->doubleSpinBoxSubTotalBruto, "subTotalBru");
  addMapping(ui->doubleSpinBoxSubTotalLiq, "subTotalLiq");
  addMapping(ui->doubleSpinBoxDescontoGlobal, "descontoPorc");
  addMapping(ui->doubleSpinBoxDescontoGlobalReais, "descontoReais");
  addMapping(ui->doubleSpinBoxFrete, "frete");
  addMapping(ui->doubleSpinBoxTotal, "total");
  addMapping(ui->spinBoxPrazoEntrega, "prazoEntrega");
  addMapping(ui->textEdit, "observacao");
}

void Venda::on_pushButtonCadastrarPedido_clicked() { update(); }

void Venda::calculoSpinBox1() const {
  const double pgt1 = ui->doubleSpinBoxPgt1->value();
  const double pgt2 = ui->doubleSpinBoxPgt2->value();
  const double pgt3 = ui->doubleSpinBoxPgt3->value();
  const double total = ui->doubleSpinBoxTotalPag->value();
  double restante = total - (pgt1 + pgt2 + pgt3);

  if (restante == 0.) return;

  ui->doubleSpinBoxPgt2->setValue(pgt2 + restante);
  ui->doubleSpinBoxPgt2->setEnabled(true);
  ui->comboBoxPgt2->setEnabled(true);

  if (pgt2 == 0. or pgt3 >= 0.) {
    ui->doubleSpinBoxPgt2->setMaximum(restante + pgt2);
    ui->doubleSpinBoxPgt2->setValue(restante + pgt2);
    ui->doubleSpinBoxPgt3->setMaximum(pgt3);

    return;
  }

  if (pgt3 == 0.) {
    ui->doubleSpinBoxPgt3->setMaximum(restante + pgt3);
    ui->doubleSpinBoxPgt3->setValue(restante + pgt3);
    ui->doubleSpinBoxPgt2->setMaximum(pgt2);

    return;
  }
}

void Venda::on_doubleSpinBoxPgt1_editingFinished() {
  calculoSpinBox1();
  montarFluxoCaixa();
}

void Venda::calculoSpinBox2() const {
  const double pgt1 = ui->doubleSpinBoxPgt1->value();
  const double pgt2 = ui->doubleSpinBoxPgt2->value();
  const double pgt3 = ui->doubleSpinBoxPgt3->value();
  const double total = ui->doubleSpinBoxTotalPag->value();
  double restante = total - (pgt1 + pgt2 + pgt3);

  if (restante == 0.) return;

  ui->doubleSpinBoxPgt3->setMaximum(restante + pgt3);
  ui->doubleSpinBoxPgt3->setValue(restante + pgt3);

  ui->doubleSpinBoxPgt3->setEnabled(true);
  ui->comboBoxPgt3->setEnabled(true);
}

void Venda::on_doubleSpinBoxPgt2_editingFinished() {
  calculoSpinBox2();
  montarFluxoCaixa();
}

void Venda::on_doubleSpinBoxPgt3_editingFinished() { montarFluxoCaixa(); }

void Venda::on_comboBoxPgt1_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") return;

  ui->comboBoxPgt1Parc->setEnabled((text == "Cartão de crédito" or text == "Cheque" or text == "Boleto") ? true
                                                                                                         : false);

  ui->dateEditPgt1->setEnabled(true);

  montarFluxoCaixa();
}

void Venda::on_comboBoxPgt2_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") return;

  ui->comboBoxPgt2Parc->setEnabled((text == "Cartão de crédito" or text == "Cheque" or text == "Boleto") ? true
                                                                                                         : false);

  ui->dateEditPgt2->setEnabled(true);

  montarFluxoCaixa();
}

void Venda::on_comboBoxPgt3_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") return;

  ui->comboBoxPgt3Parc->setEnabled((text == "Cartão de crédito" or text == "Cheque" or text == "Boleto") ? true
                                                                                                         : false);

  ui->dateEditPgt3->setEnabled(true);

  montarFluxoCaixa();
}

bool Venda::savingProcedures() {
  if (not setData("idEnderecoEntrega", ui->itemBoxEndereco->value())) return false;
  if (not setData("idEnderecoFaturamento", ui->itemBoxEnderecoFat->value())) return false;
  if (not setData("status", "PENDENTE")) return false;
  if (not setData("data", ui->dateTimeEdit->dateTime().toString("yyyy-MM-dd"))) return false;
  if (not setData("dataOrc", ui->dateTimeEditOrc->dateTime().toString("yyyy-MM-dd"))) return false;
  if (not setData("prazoEntrega", ui->spinBoxPrazoEntrega->value())) return false;
  if (not setData("observacao", ui->textEdit->toPlainText())) return false;

  return true;
}

void Venda::registerMode() {
  ui->framePagamentos->show();
  ui->pushButtonGerarExcel->hide();
  ui->pushButtonImprimir->hide();
  ui->pushButtonCancelamento->hide();
  ui->pushButtonCadastrarPedido->show();
  ui->pushButtonVoltar->show();
  ui->doubleSpinBoxDescontoGlobal->setReadOnly(false);
  ui->doubleSpinBoxDescontoGlobal->setFrame(true);
  ui->doubleSpinBoxDescontoGlobal->setButtonSymbols(QDoubleSpinBox::UpDownArrows);
  ui->doubleSpinBoxTotal->setReadOnly(false);
  ui->doubleSpinBoxTotal->setFrame(true);
  ui->doubleSpinBoxTotal->setButtonSymbols(QDoubleSpinBox::UpDownArrows);
  ui->checkBoxFreteManual->show();
}

void Venda::updateMode() {
  ui->framePagamentos_2->hide();
  ui->pushButtonGerarExcel->show();
  ui->pushButtonImprimir->show();
  ui->pushButtonCancelamento->show();
  ui->pushButtonCadastrarPedido->hide();
  ui->pushButtonVoltar->hide();
  ui->doubleSpinBoxDescontoGlobal->setReadOnly(true);
  ui->doubleSpinBoxDescontoGlobal->setFrame(false);
  ui->doubleSpinBoxDescontoGlobal->setButtonSymbols(QDoubleSpinBox::NoButtons);
  ui->doubleSpinBoxDescontoGlobalReais->setReadOnly(true);
  ui->doubleSpinBoxDescontoGlobalReais->setFrame(false);
  ui->doubleSpinBoxDescontoGlobalReais->setButtonSymbols(QDoubleSpinBox::NoButtons);
  ui->doubleSpinBoxTotal->setReadOnly(true);
  ui->doubleSpinBoxTotal->setFrame(false);
  ui->doubleSpinBoxTotal->setButtonSymbols(QDoubleSpinBox::NoButtons);
  ui->checkBoxFreteManual->hide();
}

bool Venda::viewRegister(const QModelIndex &index) {
  modelItem.setFilter("idVenda = '" + model.data(index.row(), "idVenda").toString() + "'");

  if (not modelItem.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela venda_has_produto: " + modelItem.lastError().text());
    return false;
  }

  if (not RegisterDialog::viewRegister(index)) return false;

  QString idCliente = ui->itemBoxCliente->value().toString();

  ui->itemBoxEndereco->searchDialog()->setFilter("idCliente = " + idCliente + " AND desativado = FALSE");
  ui->itemBoxEnderecoFat->searchDialog()->setFilter("idCliente = " + idCliente + " AND desativado = FALSE");

  modelFluxoCaixa.setFilter("idVenda = '" + ui->lineEditVenda->text() + "'");

  if (not modelFluxoCaixa.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela conta_a_receber_has_pagamento: " + modelFluxoCaixa.lastError().text());
    return false;
  }

  for (int row = 0; row < modelFluxoCaixa.rowCount(); ++row) {
    ui->tableFluxoCaixa->openPersistentEditor(row, "representacao");
  }

  ui->tableFluxoCaixa->resizeColumnsToContents();

  ui->tableVenda->resizeColumnsToContents();

  ui->spinBoxPrazoEntrega->setReadOnly(true);

  ui->itemBoxCliente->setReadOnlyItemBox(true);
  ui->itemBoxEndereco->setReadOnlyItemBox(true);
  ui->itemBoxEnderecoFat->setReadOnlyItemBox(true);
  ui->itemBoxProfissional->setReadOnlyItemBox(true);
  ui->itemBoxVendedor->setReadOnlyItemBox(true);

  if (data("status").toString() == "CANCELADO") ui->pushButtonCancelamento->hide();

  return true;
}

void Venda::on_pushButtonVoltar_clicked() {
  Orcamento *orcamento = new Orcamento(parentWidget());
  orcamento->viewRegisterById(m_idOrcamento);
  orcamento->show();

  isDirty = false;

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
    return;
  }

  close();
}

void Venda::montarFluxoCaixa() {
  if (ui->framePagamentos_2->isHidden()) return;

  modelFluxoCaixa.removeRows(0, modelFluxoCaixa.rowCount());

  const QList<QCheckBox *> checkBoxRep({ui->checkBoxRep1, ui->checkBoxRep2, ui->checkBoxRep3});
  const QList<QComboBox *> comboParc({ui->comboBoxPgt1Parc, ui->comboBoxPgt2Parc, ui->comboBoxPgt3Parc});
  const QList<QComboBox *> comboPgt({ui->comboBoxPgt1, ui->comboBoxPgt2, ui->comboBoxPgt3});
  const QList<QDateEdit *> datePgt({ui->dateEditPgt1, ui->dateEditPgt2, ui->dateEditPgt3});
  const QList<QDoubleSpinBox *> spinPgt({ui->doubleSpinBoxPgt1, ui->doubleSpinBoxPgt2, ui->doubleSpinBoxPgt3});
  const QList<QLineEdit *> linePgt({ui->lineEditPgt1, ui->lineEditPgt2, ui->lineEditPgt3});

  for (int i = 0; i < 3; ++i) {
    if (comboPgt.at(i)->currentText() != "Escolha uma opção!") {
      const int parcelas = comboParc.at(i)->currentIndex() + 1;
      const double valor = spinPgt.at(i)->value();
      const int temp2 = comboPgt.at(i)->currentText() == "Cartão de crédito" ? 1 : 0;
      const float temp = static_cast<float>(static_cast<int>(static_cast<float>(valor / parcelas) * 100)) / 100;
      const double resto = static_cast<double>(valor - (temp * parcelas));
      const double parcela = static_cast<double>(temp);

      for (int x = 0, y = parcelas - 1; x < parcelas; ++x, --y) {
        const int row = modelFluxoCaixa.rowCount();
        modelFluxoCaixa.insertRow(row);
        modelFluxoCaixa.setData(row, "dataEmissao", QDate::currentDate().toString("yyyy-MM-dd"));
        modelFluxoCaixa.setData(row, "idVenda", ui->lineEditVenda->text());
        modelFluxoCaixa.setData(row, "idLoja", data("idLoja"));
        modelFluxoCaixa.setData(row, "dataPagamento", datePgt.at(i)->date().addMonths(x + temp2));
        modelFluxoCaixa.setData(row, "valor", parcela + (x == 0 ? resto : 0));
        modelFluxoCaixa.setData(row, "tipo", QString::number(i + 1) + ". " + comboPgt.at(i)->currentText());
        modelFluxoCaixa.setData(row, "parcela", parcelas - y);
        modelFluxoCaixa.setData(row, "observacao", linePgt.at(i)->text());
        modelFluxoCaixa.setData(row, "representacao", checkBoxRep.at(i)->isChecked());
      }
    }
  }

  for (int row = 0; row < modelFluxoCaixa.rowCount(); ++row) {
    ui->tableFluxoCaixa->openPersistentEditor(row, "representacao");
  }

  ui->tableFluxoCaixa->resizeColumnsToContents();
}

void Venda::on_pushButtonLimparPag_clicked() { resetarPagamentos(); }

void Venda::on_doubleSpinBoxTotal_valueChanged(const double &) {
  if (isBlockedGlobal) return;

  double liq = ui->doubleSpinBoxSubTotalLiq->value();
  double frete = ui->doubleSpinBoxFrete->value();
  double total = ui->doubleSpinBoxTotal->value();
  double value = 100. * (1. - ((total - frete) / liq));

  if (liq == 0.) return;

  isBlockedTotal = true;
  ui->doubleSpinBoxDescontoGlobal->setValue(value);
  isBlockedTotal = false;
}

void Venda::on_checkBoxFreteManual_clicked(const bool &checked) {
  ui->doubleSpinBoxFrete->setFrame(checked);
  ui->doubleSpinBoxFrete->setReadOnly(not checked);
  ui->doubleSpinBoxFrete->setButtonSymbols(checked ? QDoubleSpinBox::UpDownArrows : QDoubleSpinBox::NoButtons);

  ui->doubleSpinBoxFrete->setValue(ui->checkBoxFreteManual->isChecked()
                                   ? ui->doubleSpinBoxFrete->value()
                                   : qMax(ui->doubleSpinBoxSubTotalBruto->value() * porcFrete / 100., minimoFrete));
}

void Venda::on_doubleSpinBoxFrete_valueChanged(const double &) { calcPrecoGlobalTotal(); }

void Venda::on_doubleSpinBoxDescontoGlobal_valueChanged(const double &) {
  if (isBlockedGlobal) return;

  isBlockedGlobal = true;
  calcPrecoGlobalTotal();
  isBlockedGlobal = false;
}

void Venda::on_doubleSpinBoxDescontoGlobalReais_valueChanged(const double &) {
  if (isBlockedGlobal) return;

  double liq = ui->doubleSpinBoxSubTotalLiq->value();
  double desc = ui->doubleSpinBoxDescontoGlobalReais->value();

  if (liq == 0) return;

  isBlockedReais = true;
  ui->doubleSpinBoxDescontoGlobal->setValue(100 * (1 - ((liq - desc) / liq)));
  isBlockedReais = false;
}

void Venda::on_pushButtonImprimir_clicked() {
  Impressao impressao(ui->lineEditVenda->text(), this);
  impressao.print();
}

void Venda::successMessage() { QMessageBox::information(this, "Atenção!", "Venda cadastrada com sucesso!"); }

void Venda::on_pushButtonGerarExcel_clicked() {
  Excel excel(ui->lineEditVenda->text(), this);
  excel.gerarExcel();
}

bool Venda::save(const bool &isUpdate) {
  if (not verifyFields()) return false;

  if (not isUpdate and not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
    return false;
  }

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  row = isUpdate ? mapper.currentIndex() : model.rowCount();

  if (row == -1) {
    QMessageBox::critical(this, "Erro!", "Erro linha - 1");
    QSqlQuery("ROLLBACK").exec();
    return false;
  }

  if (not isUpdate) model.insertRow(row);

  if (not savingProcedures()) {
    errorMessage();
    QSqlQuery("ROLLBACK").exec();
    return false;
  }

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro ao cadastrar: " + model.lastError().text());
    QSqlQuery("ROLLBACK").exec();
    return false;
  }

  if (not modelFluxoCaixa.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados na tabela conta_a_receber_has_pagamento: " +
                          modelFluxoCaixa.lastError().text());
    return false;
  }

  if (not modelItem.submitAll()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro salvando dados na tabela venda_has_produto: " + modelItem.lastError().text());
    return false;
  }

  QSqlQuery query;

  query.prepare("DELETE FROM orcamento_has_produto WHERE idOrcamento = :idOrcamento");
  query.bindValue(":idOrcamento", ui->lineEditVenda->text() + "O");

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro deletando itens no orçamento_has_produto: " + query.lastError().text());
    return false;
  }

  query.prepare("DELETE FROM orcamento WHERE idOrcamento = :idOrcamento");
  query.bindValue(":idOrcamento", ui->lineEditVenda->text() + "O");

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro deletando orçamento: " + query.lastError().text());
    return false;
  }

  QSqlQuery("COMMIT").exec();

  isDirty = false;

  viewRegisterById(ui->lineEditVenda->text());
  sendUpdateMessage();

  if (not silent) successMessage();

  return true;
}

void Venda::on_pushButtonCancelamento_clicked() {
  // TODO: copiar de volta para orcamento e remover contas_a_receber (alcada gerente)
  QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Tem certeza que deseja cancelar?",
                     QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Sim");
  msgBox.setButtonText(QMessageBox::No, "Não");

  if (msgBox.exec() == QMessageBox::Yes) {
    if (model.data(mapper.currentIndex(), "status").toString() != "PENDENTE") {
      QMessageBox::critical(this, "Erro!", "Status diferente de 'PENDENTE'! Deve fazer estorno/devolução.");
      return;
    }

    if (not model.setData(mapper.currentIndex(), "status", "CANCELADO")) return;

    if (not model.submitAll()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando status 'CANCELADO': " + model.lastError().text());
      return;
    }

    QMessageBox::information(this, "Aviso!", "Venda cancelada!");
    close();
  }
}

void Venda::generateId() {
  QString id = UserSession::fromLoja("sigla", ui->itemBoxVendedor->text()) + "-" + QDate::currentDate().toString("yy");

  QSqlQuery query;
  query.prepare("SELECT idVenda FROM venda WHERE idVenda LIKE :id ORDER BY idVenda ASC");
  query.bindValue(":id", id + "%");

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro na query: " + query.lastError().text());
    return;
  }

  int last = 0;

  if (query.last()) {
    QString temp = query.value("idVenda").toString().mid(id.size());

    if (temp.endsWith("O")) temp.remove(temp.size() - 1, 1);
    if (temp.endsWith("R")) temp.remove(temp.size() - 1, 1);

    last = temp.toInt();
  }

  id += UserSession::fromLoja("loja.idLoja", ui->itemBoxVendedor->text());
  id += QString("%1").arg(last + 1, 3, 10, QChar('0'));
  id += (m_idOrcamento.endsWith("R") or m_idOrcamento.endsWith("RO")) ? "R" : "";

  ui->lineEditVenda->setText(id);
}

// NOTE: reorganizar tela de venda, talvez colocar fluxo de caixa numa aba separada ou embaixo da tabela principal
