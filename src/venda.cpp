#include <QSqlRecord>
#include <QSqlError>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include <QDesktopServices>

#include "venda.h"
#include "ui_venda.h"
#include "orcamento.h"
#include "usersession.h"
#include "cadastrarnfe.h"
#include "doubledelegate.h"
#include "checkboxdelegate.h"
#include "qtrpt.h"
#include "cadastrocliente.h"
#include "xlsxdocument.h"

Venda::Venda(QWidget *parent) : RegisterDialog("venda", "idVenda", parent), ui(new Ui::Venda) {
  ui->setupUi(this);

  setupTables();

  ui->itemBoxCliente->setSearchDialog(SearchDialog::cliente(this));
  ui->itemBoxCliente->setRegisterDialog(new CadastroCliente(this));
  ui->itemBoxVendedor->setSearchDialog(SearchDialog::vendedor(this));
  ui->itemBoxProfissional->setSearchDialog(SearchDialog::profissional(this));
  ui->itemBoxEndereco->setSearchDialog(SearchDialog::enderecoCliente(this));
  ui->itemBoxEnderecoFat->setSearchDialog(SearchDialog::enderecoCliente(this));

  // TODO: make this runtime changeable
  QStringList list{"Escolha uma opção!", "Cartão de débito", "Cartão de crédito", "Cheque", "Dinheiro", "Boleto",
                   "Transf. Banc."};

  ui->comboBoxPgt1->insertItems(0, list);
  ui->comboBoxPgt2->insertItems(0, list);
  ui->comboBoxPgt3->insertItems(0, list);

  ui->dateEditPgt1->setDate(QDate::currentDate());
  ui->dateEditPgt2->setDate(QDate::currentDate());
  ui->dateEditPgt3->setDate(QDate::currentDate());

  setupMapper();
  newRegister();

  for (const auto *line : findChildren<QLineEdit *>(QString(), Qt::FindDirectChildrenOnly)) {
    connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);
  }

  // TODO: make this work
  ui->splitter->setStretchFactor(0, 1);
  ui->splitter->setStretchFactor(1, 0);

  show();
}

Venda::~Venda() { delete ui; }

void Venda::setupTables() {
  modelItem.setTable("venda_has_produto");
  modelItem.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelItem.setHeaderData(modelItem.fieldIndex("selecionado"), Qt::Horizontal, "");
  modelItem.setHeaderData(modelItem.fieldIndex("fornecedor"), Qt::Horizontal, "Fornecedor");
  modelItem.setHeaderData(modelItem.fieldIndex("produto"), Qt::Horizontal, "Produto");
  modelItem.setHeaderData(modelItem.fieldIndex("obs"), Qt::Horizontal, "Obs.");
  modelItem.setHeaderData(modelItem.fieldIndex("prcUnitario"), Qt::Horizontal, "Preço/Un");
  modelItem.setHeaderData(modelItem.fieldIndex("caixas"), Qt::Horizontal, "Caixas");
  modelItem.setHeaderData(modelItem.fieldIndex("quant"), Qt::Horizontal, "Quant.");
  modelItem.setHeaderData(modelItem.fieldIndex("un"), Qt::Horizontal, "Un.");
  modelItem.setHeaderData(modelItem.fieldIndex("unCaixa"), Qt::Horizontal, "Un./Caixa");
  modelItem.setHeaderData(modelItem.fieldIndex("codComercial"), Qt::Horizontal, "Cód. Com.");
  modelItem.setHeaderData(modelItem.fieldIndex("formComercial"), Qt::Horizontal, "Form. Com.");
  modelItem.setHeaderData(modelItem.fieldIndex("parcial"), Qt::Horizontal, "Parcial");
  modelItem.setHeaderData(modelItem.fieldIndex("desconto"), Qt::Horizontal, "Desconto");
  modelItem.setHeaderData(modelItem.fieldIndex("parcialDesc"), Qt::Horizontal, "Desc. Parc.");
  modelItem.setHeaderData(modelItem.fieldIndex("descGlobal"), Qt::Horizontal, "Desc. Glob.");
  modelItem.setHeaderData(modelItem.fieldIndex("total"), Qt::Horizontal, "Total");
  modelItem.setHeaderData(modelItem.fieldIndex("status"), Qt::Horizontal, "Status");
  modelItem.setHeaderData(modelItem.fieldIndex("dataPrevCompra"), Qt::Horizontal, "Prev. Compra");
  modelItem.setHeaderData(modelItem.fieldIndex("dataRealCompra"), Qt::Horizontal, "Data Compra");
  modelItem.setHeaderData(modelItem.fieldIndex("dataPrevConf"), Qt::Horizontal, "Prev. Confirm.");
  modelItem.setHeaderData(modelItem.fieldIndex("dataRealConf"), Qt::Horizontal, "Data Confirm.");
  modelItem.setHeaderData(modelItem.fieldIndex("dataPrevFat"), Qt::Horizontal, "Prev. Fat.");
  modelItem.setHeaderData(modelItem.fieldIndex("dataRealFat"), Qt::Horizontal, "Data Fat.");
  modelItem.setHeaderData(modelItem.fieldIndex("dataPrevColeta"), Qt::Horizontal, "Prev. Coleta");
  modelItem.setHeaderData(modelItem.fieldIndex("dataRealColeta"), Qt::Horizontal, "Data Coleta");
  modelItem.setHeaderData(modelItem.fieldIndex("dataPrevReceb"), Qt::Horizontal, "Prev. Receb.");
  modelItem.setHeaderData(modelItem.fieldIndex("dataRealReceb"), Qt::Horizontal, "Data Receb.");
  modelItem.setHeaderData(modelItem.fieldIndex("dataPrevEnt"), Qt::Horizontal, "Prev. Ent.");
  modelItem.setHeaderData(modelItem.fieldIndex("dataRealEnt"), Qt::Horizontal, "Data Ent.");

  if (not modelItem.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela venda_has_produto: " + modelItem.lastError().text());
    return;
  }

  ui->tableVenda->setModel(&modelItem);
  ui->tableVenda->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableVenda->setColumnHidden(modelItem.fieldIndex("idNfeSaida"), true);
  ui->tableVenda->setColumnHidden(modelItem.fieldIndex("idVendaProduto"), true);
  ui->tableVenda->setColumnHidden(modelItem.fieldIndex("selecionado"), true);
  ui->tableVenda->setColumnHidden(modelItem.fieldIndex("idVenda"), true);
  ui->tableVenda->setColumnHidden(modelItem.fieldIndex("idLoja"), true);
  ui->tableVenda->setColumnHidden(modelItem.fieldIndex("idProduto"), true);
  ui->tableVenda->setColumnHidden(modelItem.fieldIndex("item"), true);
  ui->tableVenda->setItemDelegate(new DoubleDelegate(this));
  ui->tableVenda->setItemDelegateForColumn(modelItem.fieldIndex("selecionado"), new CheckBoxDelegate(this));

  modelFluxoCaixa.setTable("conta_a_receber_has_pagamento");
  modelFluxoCaixa.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelFluxoCaixa.setHeaderData(modelFluxoCaixa.fieldIndex("tipo"), Qt::Horizontal, "Tipo");
  modelFluxoCaixa.setHeaderData(modelFluxoCaixa.fieldIndex("parcela"), Qt::Horizontal, "Parcela");
  modelFluxoCaixa.setHeaderData(modelFluxoCaixa.fieldIndex("valor"), Qt::Horizontal, "R$");
  modelFluxoCaixa.setHeaderData(modelFluxoCaixa.fieldIndex("data"), Qt::Horizontal, "Data");
  modelFluxoCaixa.setHeaderData(modelFluxoCaixa.fieldIndex("observacao"), Qt::Horizontal, "Obs.");
  modelFluxoCaixa.setHeaderData(modelFluxoCaixa.fieldIndex("status"), Qt::Horizontal, "Status");

  if (not modelFluxoCaixa.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela conta_a_receber_has_pagamento: " + modelFluxoCaixa.lastError().text());
    return;
  }

  ui->tableFluxoCaixa->setModel(&modelFluxoCaixa);
  ui->tableFluxoCaixa->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableFluxoCaixa->setColumnHidden(modelFluxoCaixa.fieldIndex("idVenda"), true);
  ui->tableFluxoCaixa->setColumnHidden(modelFluxoCaixa.fieldIndex("idLoja"), true);
  ui->tableFluxoCaixa->setColumnHidden(modelFluxoCaixa.fieldIndex("idPagamento"), true);
  ui->tableFluxoCaixa->setItemDelegate(new DoubleDelegate(this));
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
  const int row = model.rowCount();
  model.insertRow(row);
  mapper.toLast();

  ui->lineEditVenda->setText(idOrcamento);

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

  QSqlQuery queryCliente;
  queryCliente.prepare("SELECT * FROM cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", queryOrc.value("idCliente"));

  if (not queryCliente.exec() or not queryCliente.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando cliente: " + queryCliente.lastError().text());
    return;
  }

  QSqlQuery queryEndFat;
  queryEndFat.prepare("SELECT * FROM cliente_has_endereco WHERE idEndereco = :idEndereco");
  queryEndFat.bindValue(":idEndereco", queryOrc.value("idEnderecoFaturamento"));

  if (not queryEndFat.exec() or not queryEndFat.first()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro buscando endereço faturamento do cliente: " + queryEndFat.lastError().text());
    return;
  }

  QSqlQuery queryEndEnt;
  queryEndEnt.prepare("SELECT * FROM cliente_has_endereco WHERE idEndereco= :idEndereco");
  queryEndEnt.bindValue(":idEndereco", queryOrc.value("idEnderecoEntrega"));

  if (not queryEndEnt.exec() or not queryEndEnt.first()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro buscando endereço entrega do cliente: " + queryEndEnt.lastError().text());
    return;
  }

  QSqlQuery queryVendedor;
  queryVendedor.prepare("SELECT * FROM usuario WHERE idUsuario = :idUsuario");
  queryVendedor.bindValue(":idUsuario", queryOrc.value("idUsuario"));

  if (not queryVendedor.exec() or not queryVendedor.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando vendedor: " + queryVendedor.lastError().text());
    return;
  }

  QSqlQuery queryProfissional;
  queryProfissional.prepare("SELECT * FROM profissional WHERE idProfissional = :idProfissional");
  queryProfissional.bindValue(":idProfissional", queryOrc.value("idProfissional"));

  if (not queryProfissional.exec() or not queryProfissional.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando profissional: " + queryProfissional.lastError().text());
    return;
  }

  modelItem.setFilter("idVenda = '" + idOrcamento + "'");

  while (queryProdutos.next()) {
    const int rowItem = modelItem.rowCount();
    modelItem.insertRow(rowItem);

    if (not modelItem.setData(rowItem, "idVenda", queryProdutos.value("idOrcamento"))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando idVenda: " + modelItem.lastError().text());
      return;
    }

    for (int field = 1, fieldCount = queryProdutos.record().count(); field < fieldCount; ++field) {
      if (modelItem.fieldIndex(queryProdutos.record().fieldName(field)) != -1 and
          not modelItem.setData(rowItem, queryProdutos.record().fieldName(field),
                                queryProdutos.value(queryProdutos.record().fieldName(field)))) {
        QMessageBox::critical(this, "Erro!", "Erro guardando itens venda: " + modelItem.lastError().text());
        return;
      }
    }

    if (not modelItem.setData(rowItem, "status", "PENDENTE")) {
      QMessageBox::critical(this, "Erro!", "Erro guardando status PENDENTE: " + modelItem.lastError().text());
      return;
    }
  }

  if (not model.setData(row, "idVenda", queryOrc.value("idOrcamento"))) {
    QMessageBox::critical(this, "Erro!", "Erro guardando idVenda: " + model.lastError().text());
    return;
  }

  for (int field = 1, columnCount = queryOrc.record().count(); field < columnCount; ++field) {
    if (model.fieldIndex(queryOrc.record().fieldName(field)) != -1 and
        not model.setData(mapper.currentIndex(), queryOrc.record().fieldName(field), queryOrc.value(field))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando dados venda: " + model.lastError().text());
      return;
    }
  }

  fillTotals();
  resetarPagamentos();

  modelFluxoCaixa.setFilter("idVenda = '" + idOrcamento + "'");
  ui->itemBoxEndereco->searchDialog()->setFilter("idCliente = " + queryOrc.value("idCliente").toString() +
                                                 " AND desativado = FALSE");
  ui->itemBoxEnderecoFat->searchDialog()->setFilter("idCliente = " + queryOrc.value("idCliente").toString() +
                                                    " AND desativado = FALSE");

  ui->lineEditVenda->setText(queryOrc.value("idOrcamento").toString());
  ui->itemBoxVendedor->setValue(queryOrc.value("idUsuario"));
  ui->itemBoxCliente->setValue(queryOrc.value("idCliente"));
  ui->dateTimeEdit->setDateTime(queryOrc.value("data").toDateTime());
  ui->itemBoxProfissional->setValue(queryOrc.value("idProfissional"));
  ui->itemBoxEndereco->setValue(queryOrc.value("idEnderecoEntrega"));
  ui->itemBoxEnderecoFat->setValue(queryOrc.value("idEnderecoFaturamento"));

  ui->dateTimeEditOrc->setDateTime(model.data(mapper.currentIndex(), "data").toDateTime());
  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

  ui->tableVenda->resizeColumnsToContents();
}

bool Venda::verifyFields() {
  if (ui->spinBoxPrazoEntrega->value() == 0) {
    QMessageBox::warning(this, "Aviso!", "Por favor preencha o prazo de entrega.");
    return false;
  }

  if (ui->doubleSpinBoxPgt1->value() + ui->doubleSpinBoxPgt2->value() + ui->doubleSpinBoxPgt3->value() <
      ui->doubleSpinBoxTotalPag->value()) {
    QMessageBox::critical(this, "Erro!", "Soma dos pagamentos não é igual ao total! Favor verificar.");
    return false;
  }

  if (ui->doubleSpinBoxPgt1->value() > 0 and ui->comboBoxPgt1->currentText() == "Escolha uma opção!") {
    QMessageBox::warning(this, "Aviso!", "Por favor escolha a forma de pagamento 1.");
    return false;
  }
  if (ui->doubleSpinBoxPgt2->value() > 0 and ui->comboBoxPgt2->currentText() == "Escolha uma opção!") {
    QMessageBox::warning(this, "Aviso!", "Por favor escolha a forma de pagamento 2.");
    return false;
  }
  if (ui->doubleSpinBoxPgt3->value() > 0 and ui->comboBoxPgt3->currentText() == "Escolha uma opção!") {
    QMessageBox::warning(this, "Aviso!", "Por favor escolha a forma de pagamento 3.");
    return false;
  }

  if (ui->itemBoxEnderecoFat->text().isEmpty()) {
    QMessageBox::warning(this, "Aviso!", "Deve selecionar um endereço de faturamento.");
    return false;
  }

  return true;
}

bool Venda::verifyRequiredField(QLineEdit *line) const {
  Q_UNUSED(line);

  return true;
}

QString Venda::requiredStyle() const { return QString(); }

void Venda::calcPrecoGlobalTotal(const bool ajusteTotal) {
  QSqlQuery queryFrete;
  queryFrete.prepare("SELECT * FROM loja WHERE idLoja = :idLoja");
  queryFrete.bindValue(":idLoja", UserSession::getLoja());

  if (not queryFrete.exec() or not queryFrete.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando parâmetros do frete: " + queryFrete.lastError().text());
    return;
  }

  const double minimoFrete = queryFrete.value("valorMinimoFrete").toDouble();
  const double porcFrete = queryFrete.value("porcentagemFrete").toDouble();

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

  const double frete = ui->checkBoxFreteManual->isChecked() ? ui->doubleSpinBoxFrete->value()
                                                            : qMax(subTotalBruto * porcFrete / 100., minimoFrete);

  double descGlobal = ui->doubleSpinBoxDescontoGlobal->value() / 100.;
  double subTotal = subTotalItens * (1. - descGlobal);

  if (ajusteTotal) {
    subTotal = ui->doubleSpinBoxTotal->value() - frete;
    descGlobal = subTotalItens == 0. ? 0. : 1. - (subTotal / subTotalItens);
  }

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    modelItem.setData(row, "descGlobal", descGlobal * 100.);
    modelItem.setData(row, "total", modelItem.data(row, "parcialDesc").toDouble() * (1 - descGlobal));
  }

  ui->doubleSpinBoxSubTotalBruto->setValue(subTotalBruto);
  ui->doubleSpinBoxSubTotalLiq->setValue(subTotalItens);
  ui->doubleSpinBoxDescontoGlobal->setValue(descGlobal * 100.);
  ui->doubleSpinBoxDescontoGlobal->setPrefix("- R$ " + QString::number(subTotalItens - subTotal, 'f', 2) + " - ");
  ui->doubleSpinBoxFrete->setValue(frete);
  ui->doubleSpinBoxTotal->setValue(subTotal + frete);

  resetarPagamentos();
  montarFluxoCaixa();
}

void Venda::fillTotals() {
  QSqlQuery query;
  query.prepare("SELECT * FROM orcamento WHERE idOrcamento = :idOrcamento");
  query.bindValue(":idOrcamento", ui->lineEditVenda->text());

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando orçamento: " + query.lastError().text());
    return;
  }

  if (not query.first()) {
    query.prepare("SELECT * FROM venda WHERE idVenda = :idVenda");
    query.bindValue(":idVenda", ui->lineEditVenda->text());

    if (not query.exec() or not query.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando venda: " + query.lastError().text());
      return;
    }
  }

  ui->doubleSpinBoxSubTotalBruto->setValue(query.value("subTotalBru").toDouble());
  ui->doubleSpinBoxSubTotalLiq->setValue(query.value("subTotalLiq").toDouble());
  ui->doubleSpinBoxFrete->setValue(query.value("frete").toDouble());
  ui->doubleSpinBoxDescontoGlobal->setValue(query.value("descontoPorc").toDouble());
  ui->doubleSpinBoxTotal->setValue(query.value("total").toDouble());
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
  addMapping(ui->doubleSpinBoxFrete, "frete");
  addMapping(ui->doubleSpinBoxDescontoGlobal, "descontoPorc");
  addMapping(ui->doubleSpinBoxTotal, "total");
  addMapping(ui->spinBoxPrazoEntrega, "prazoEntrega");
  addMapping(ui->textEdit, "observacao");
}

void Venda::on_pushButtonCancelar_clicked() { close(); }

void Venda::on_pushButtonCadastrarPedido_clicked() { update(); }

void Venda::calculoSpinBox1() const {
  const double pgt1 = ui->doubleSpinBoxPgt1->value();
  const double pgt2 = ui->doubleSpinBoxPgt2->value();
  const double pgt3 = ui->doubleSpinBoxPgt3->value();
  const double total = ui->doubleSpinBoxTotalPag->value();
  double restante = total - (pgt1 + pgt2 + pgt3);

  ui->doubleSpinBoxPgt2->setValue(pgt2 + restante);
  ui->doubleSpinBoxPgt2->setEnabled(true);
  ui->comboBoxPgt2->setEnabled(true);

  if (pgt2 == 0. or pgt3 >= 0.) {
    ui->doubleSpinBoxPgt2->setMaximum(restante + pgt2);
    ui->doubleSpinBoxPgt2->setValue(restante + pgt2);
    ui->doubleSpinBoxPgt3->setMaximum(pgt3);
    restante = 0;
  } else if (pgt3 == 0.) {
    ui->doubleSpinBoxPgt3->setMaximum(restante + pgt3);
    ui->doubleSpinBoxPgt3->setValue(restante + pgt3);
    ui->doubleSpinBoxPgt2->setMaximum(pgt2);
    restante = 0;
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
  if (text == "Escolha uma opção!") {
    return;
  }

  if (text == "Cartão de crédito" or text == "Cheque" or text == "Boleto") {
    ui->comboBoxPgt1Parc->setEnabled(true);
  } else {
    ui->comboBoxPgt1Parc->setDisabled(true);
  }

  ui->dateEditPgt1->setEnabled(true);

  montarFluxoCaixa();
}

void Venda::on_comboBoxPgt2_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") {
    return;
  }

  if (text == "Cartão de crédito" or text == "Cheque" or text == "Boleto") {
    ui->comboBoxPgt2Parc->setEnabled(true);
  } else {
    ui->comboBoxPgt2Parc->setDisabled(true);
  }

  ui->dateEditPgt2->setEnabled(true);

  montarFluxoCaixa();
}

void Venda::on_comboBoxPgt3_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") {
    return;
  }

  if (text == "Cartão de crédito" or text == "Cheque" or text == "Boleto") {
    ui->comboBoxPgt3Parc->setEnabled(true);
  } else {
    ui->comboBoxPgt3Parc->setDisabled(true);
  }

  ui->dateEditPgt3->setEnabled(true);

  montarFluxoCaixa();
}

bool Venda::savingProcedures(const int row) {
  setData(row, "idEnderecoEntrega", ui->itemBoxEndereco->value());
  setData(row, "idEnderecoFaturamento", ui->itemBoxEnderecoFat->value());
  setData(row, "status", "PENDENTE");
  setData(row, "data", ui->dateTimeEdit->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
  setData(row, "dataOrc", ui->dateTimeEditOrc->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
  setData(row, "prazoEntrega", QDate::currentDate().addDays(ui->spinBoxPrazoEntrega->value()));
  setData(row, "observacao", ui->textEdit->toPlainText());

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados na tabela venda: " + model.lastError().text());
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

  query.prepare("DELETE FROM orcamento_has_produto WHERE idOrcamento = :idVenda");
  query.bindValue(":idVenda", ui->lineEditVenda->text());

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro deletando itens no orçamento_has_produto: " + query.lastError().text());
    return false;
  }

  query.prepare("DELETE FROM orcamento WHERE idOrcamento = :idVenda");
  query.bindValue(":idVenda", ui->lineEditVenda->text());

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro deletando orçamento: " + query.lastError().text());
    return false;
  }

  query.prepare("INSERT INTO conta_a_receber (dataEmissao, idVenda) VALUES (:dataEmissao, :idVenda)");
  query.bindValue(":dataEmissao", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
  query.bindValue(":idVenda", ui->lineEditVenda->text());

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro inserindo dados em conta_a_receber: " + query.lastError().text());
    return false;
  }

  return true;
}

void Venda::registerMode() {
  ui->framePagamentos->show();
  ui->pushButtonImprimir->hide();
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
  ui->pushButtonImprimir->show();
  ui->pushButtonCadastrarPedido->hide();
  ui->pushButtonVoltar->hide();
  ui->doubleSpinBoxDescontoGlobal->setReadOnly(true);
  ui->doubleSpinBoxDescontoGlobal->setFrame(false);
  ui->doubleSpinBoxDescontoGlobal->setButtonSymbols(QDoubleSpinBox::NoButtons);
  ui->doubleSpinBoxTotal->setReadOnly(true);
  ui->doubleSpinBoxTotal->setFrame(false);
  ui->doubleSpinBoxTotal->setButtonSymbols(QDoubleSpinBox::NoButtons);
  ui->checkBoxFreteManual->hide();
}

bool Venda::viewRegister(const QModelIndex index) {
  if (not RegisterDialog::viewRegister(index)) {
    return false;
  }

  QString idCliente = ui->itemBoxCliente->value().toString();

  ui->itemBoxEndereco->searchDialog()->setFilter("idCliente = " + idCliente + " AND desativado = FALSE");
  ui->itemBoxEnderecoFat->searchDialog()->setFilter("idCliente = " + idCliente + " AND desativado = FALSE");

  modelItem.setFilter("idVenda = '" + ui->lineEditVenda->text() + "'");

  if (not modelItem.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela venda_has_produto: " + modelItem.lastError().text());
    return false;
  }

  modelFluxoCaixa.setFilter("idVenda = '" + ui->lineEditVenda->text() + "'");

  if (not modelFluxoCaixa.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela conta_a_receber_has_pagamento: " + modelFluxoCaixa.lastError().text());
    return false;
  }

  ui->tableFluxoCaixa->resizeColumnsToContents();

  fillTotals();

  ui->tableVenda->resizeColumnsToContents();

  int prazo = QDate::currentDate().daysTo(model.data(mapper.currentIndex(), "prazoEntrega").toDate());
  ui->spinBoxPrazoEntrega->setValue(prazo);

  ui->spinBoxPrazoEntrega->setReadOnly(true);

  ui->itemBoxCliente->setReadOnlyItemBox(true);
  ui->itemBoxEndereco->setReadOnlyItemBox(true);
  ui->itemBoxEnderecoFat->setReadOnlyItemBox(true);
  ui->itemBoxProfissional->setReadOnlyItemBox(true);
  ui->itemBoxVendedor->setReadOnlyItemBox(true);

  return true;
}

void Venda::on_pushButtonVoltar_clicked() {
  Orcamento *orcamento = new Orcamento(parentWidget());
  orcamento->viewRegisterById(ui->lineEditVenda->text());
  orcamento->show();

  isDirty = false;
  model.select();
  close();
}

void Venda::montarFluxoCaixa() {
  if (ui->framePagamentos_2->isHidden()) {
    return;
  }

  modelFluxoCaixa.removeRows(0, modelFluxoCaixa.rowCount());

  int row = 0;

  QList<QComboBox *> comboPgt;
  comboPgt << ui->comboBoxPgt1 << ui->comboBoxPgt2 << ui->comboBoxPgt3;
  QList<QComboBox *> comboParc;
  comboParc << ui->comboBoxPgt1Parc << ui->comboBoxPgt2Parc << ui->comboBoxPgt3Parc;
  QList<QDoubleSpinBox *> spinPgt;
  spinPgt << ui->doubleSpinBoxPgt1 << ui->doubleSpinBoxPgt2 << ui->doubleSpinBoxPgt3;
  QList<QDateEdit *> datePgt;
  datePgt << ui->dateEditPgt1 << ui->dateEditPgt2 << ui->dateEditPgt3;
  QList<QLineEdit *> linePgt;
  linePgt << ui->lineEditPgt1 << ui->lineEditPgt2 << ui->lineEditPgt3;

  for (int i = 0; i < 3; ++i) {
    if (comboPgt.at(i)->currentText() != "Escolha uma opção!") {
      const int parcelas = comboParc.at(i)->currentIndex() + 1;
      const double valor = spinPgt.at(i)->value();
      const int temp2 = comboPgt.at(i)->currentText() == "Cartão de crédito" ? 1 : 0;
      const float temp = static_cast<float>(static_cast<int>(static_cast<float>(valor / parcelas) * 100)) / 100;
      const double resto = static_cast<double>(valor - (temp * parcelas));
      const double parcela = static_cast<double>(temp);

      for (int x = 0, y = parcelas - 1; x < parcelas; ++x, --y) {
        modelFluxoCaixa.insertRow(modelFluxoCaixa.rowCount());
        modelFluxoCaixa.setData(row, "idVenda", ui->lineEditVenda->text());
        modelFluxoCaixa.setData(row, "idLoja", UserSession::getLoja());
        modelFluxoCaixa.setData(row, "tipo", QString::number(i + 1) + ". " + comboPgt.at(i)->currentText());
        modelFluxoCaixa.setData(row, "parcela", parcelas - y);
        modelFluxoCaixa.setData(row, "valor", parcela + (x == 0 ? resto : 0));
        modelFluxoCaixa.setData(row, "data", datePgt.at(i)->date().addMonths(x + temp2));
        modelFluxoCaixa.setData(row, "observacao", linePgt.at(i)->text());

        ++row;
      }
    }
  }

  ui->tableFluxoCaixa->resizeColumnsToContents();
}

void Venda::on_comboBoxPgt1Parc_currentTextChanged(const QString &text) {
  Q_UNUSED(text);

  montarFluxoCaixa();
}

void Venda::on_comboBoxPgt2Parc_currentTextChanged(const QString &text) {
  Q_UNUSED(text);

  montarFluxoCaixa();
}

void Venda::on_comboBoxPgt3Parc_currentTextChanged(const QString &text) {
  Q_UNUSED(text);

  montarFluxoCaixa();
}

void Venda::on_dateEditPgt1_dateChanged(const QDate &) { montarFluxoCaixa(); }

void Venda::on_dateEditPgt2_dateChanged(const QDate &) { montarFluxoCaixa(); }

void Venda::on_dateEditPgt3_dateChanged(const QDate &) { montarFluxoCaixa(); }

void Venda::on_pushButtonLimparPag_clicked() { resetarPagamentos(); }

void Venda::on_doubleSpinBoxTotal_editingFinished() {
  if (modelItem.rowCount() == 0 or ui->doubleSpinBoxSubTotalLiq->value() == 0) {
    calcPrecoGlobalTotal();
    return;
  }

  const double new_total = ui->doubleSpinBoxTotal->value();
  const double frete = ui->doubleSpinBoxFrete->value();
  const double new_subtotal = new_total - frete;

  if (new_subtotal >= ui->doubleSpinBoxSubTotalLiq->value()) {
    ui->doubleSpinBoxDescontoGlobal->setValue(0.);
    calcPrecoGlobalTotal();
  } else {
    calcPrecoGlobalTotal(true);
  }
}

void Venda::on_checkBoxFreteManual_clicked(const bool checked) {
  if (checked == true and UserSession::getTipoUsuario() != "ADMINISTRADOR") {
    ui->checkBoxFreteManual->setChecked(false);
    return;
  }

  ui->doubleSpinBoxFrete->setFrame(checked);
  ui->doubleSpinBoxFrete->setReadOnly(not checked);
  ui->doubleSpinBoxFrete->setButtonSymbols(checked ? QDoubleSpinBox::UpDownArrows : QDoubleSpinBox::NoButtons);

  calcPrecoGlobalTotal();
}

void Venda::on_doubleSpinBoxFrete_editingFinished() { calcPrecoGlobalTotal(); }

void Venda::on_doubleSpinBoxDescontoGlobal_valueChanged(const double) { calcPrecoGlobalTotal(); }

void Venda::on_pushButtonImprimir_clicked() {
  QtRPT *report = new QtRPT(this);
  QFile file(qApp->applicationDirPath() + "/venda.xml");

  if (not file.exists()) {
    QMessageBox::critical(this, "Erro!", "XML da impressão não encontrado!");
    return;
  }

  queryCliente.prepare("SELECT * FROM cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", ui->itemBoxCliente->value());

  if (not queryCliente.exec() or not queryCliente.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando cliente: " + model.lastError().text());
    return;
  }

  queryProfissional.prepare("SELECT * FROM profissional WHERE idProfissional = :idProfissional");
  queryProfissional.bindValue(":idProfissional", ui->itemBoxProfissional->value());

  if (not queryProfissional.exec() or not queryProfissional.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando profissional: " + queryProfissional.lastError().text());
    return;
  }

  queryVendedor.prepare("SELECT * FROM usuario WHERE nome = :nome");
  queryVendedor.bindValue(":nome", ui->itemBoxVendedor->text());

  if (not queryVendedor.exec() or not queryVendedor.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando vendedor: " + queryVendedor.lastError().text());
    return;
  }

  queryEndereco.prepare("SELECT * FROM cliente_has_endereco WHERE idEndereco = :idEndereco");
  queryEndereco.bindValue(":idEndereco", ui->itemBoxEndereco->value());

  if (not queryEndereco.exec() or not queryEndereco.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando endereço: " + queryEndereco.lastError().text());
    return;
  }

  queryLoja.prepare("SELECT * FROM loja WHERE idLoja = :idLoja");
  queryLoja.bindValue(":idLoja", queryVendedor.value("idLoja"));

  if (not queryLoja.exec() or not queryLoja.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando loja: " + queryLoja.lastError().text());
    return;
  }

  queryLojaEnd.prepare("SELECT * FROM loja_has_endereco WHERE idLoja = :idLoja");
  queryLojaEnd.bindValue(":idLoja", queryVendedor.value("idLoja"));

  if (not queryLojaEnd.exec() or not queryLojaEnd.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando loja endereço: " + queryLojaEnd.lastError().text());
    return;
  }

  report->loadReport(file.fileName());
  report->recordCount << ui->tableVenda->model()->rowCount();
  connect(report, &QtRPT::setValue, this, &Venda::setValue);

  QSettings settings("Staccato", "ERP");
  settings.beginGroup("User");
  QString path = settings.value("userFolder").toString();

  QDir dir(path);

  if (not dir.exists()) {
    dir.mkdir(path);
  }

  report->printPDF(path + "/" + ui->lineEditVenda->text() + ".pdf");
}

void Venda::setValue(const int recNo, const QString paramName, QVariant &paramValue, const int reportPage) {
  Q_UNUSED(reportPage);

  QLocale locale;

  if (modelItem.data(recNo, "idProduto") != queryProduto.boundValue(":idProduto")) {
    queryProduto.prepare("SELECT * FROM produto WHERE idProduto = :idProduto");
    queryProduto.bindValue(":idProduto", modelItem.data(recNo, "idProduto"));

    if (not queryProduto.exec() or not queryProduto.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando produto: " + modelItem.lastError().text());
      return;
    }
  }

  // REPORT TITLE
  if (paramName == "Loja") {
    paramValue = queryLoja.value("descricao").toString();
    return;
  }

  if (paramName == "Endereco") {
    paramValue = queryLojaEnd.value("logradouro").toString() + ", " + queryLojaEnd.value("numero").toString() + " - " +
                 queryLojaEnd.value("bairro").toString() + "\n" + queryLojaEnd.value("cidade").toString() + " - " +
                 queryLojaEnd.value("uf").toString() + " - CEP: " + queryLojaEnd.value("cep").toString() + "\n" +
                 queryLoja.value("tel").toString() + " - " + queryLoja.value("tel2").toString();
    return;
  }

  if (paramName == "pedido") {
    paramValue = ui->lineEditVenda->text();
    return;
  }

  if (paramName == "data") {
    paramValue = ui->dateTimeEdit->dateTime().toString("dd-MM-yyyy");
    return;
  }

  if (paramName == "cliente") {
    paramValue = ui->itemBoxCliente->text().left(30);
    return;
  }

  if (paramName == "cpfcnpj") {
    paramValue = queryCliente.value(queryCliente.value("pfpj").toString() == "PF" ? "cpf" : "cnpj");
    return;
  }

  if (paramName == "email") {
    paramValue = queryCliente.value("email");
    return;
  }

  if (paramName == "tel1") {
    paramValue = queryCliente.value("tel");
    return;
  }

  if (paramName == "tel2") {
    paramValue = queryCliente.value("telCel");
    return;
  }

  if (paramName == "endfiscal") {
    QString endereco = ui->itemBoxEndereco->text();

    if (endereco != "Não há/Retira") {
      endereco = endereco.remove(0, endereco.indexOf("-") + 2);
    }

    paramValue = endereco;
    return;
  }

  if (paramName == "cepfiscal") {
    paramValue = queryEndereco.value("cep");
    return;
  }

  if (paramName == "endentrega") {
    QString endereco = ui->itemBoxEndereco->text();

    if (endereco != "Não há/Retira") {
      endereco = endereco.remove(0, endereco.indexOf("-") + 2);
    }

    paramValue = endereco;
    return;
  }

  if (paramName == "cepentrega") {
    paramValue = queryEndereco.value("cep");
    return;
  }

  if (paramName == "profissional") {
    paramValue = ui->itemBoxProfissional->text().isEmpty() ? "Não há" : ui->itemBoxProfissional->text();
    return;
  }

  if (paramName == "telprofissional") {
    paramValue = queryProfissional.value("tel");
    return;
  }

  if (paramName == "emailprofissional") {
    paramValue = queryProfissional.value("email");
    return;
  }

  if (paramName == "vendedor") {
    paramValue = ui->itemBoxVendedor->text();
    return;
  }

  if (paramName == "emailvendedor") {
    paramValue = queryVendedor.value("email");
    return;
  }

  if (paramName == "estoque") {
    return;
  }

  if (paramName == "dataestoque") {
    return;
  }

  // MASTER BAND
  if (paramName == "Marca") {
    paramValue = modelItem.data(recNo, "fornecedor").toString();
    return;
  }

  if (paramName == "Código") {
    paramValue = queryProduto.value("codComercial").toString();
    return;
  }

  if (paramName == "Nome do produto") {
    paramValue = modelItem.data(recNo, "produto").toString();
    return;
  }

  if (paramName == "Ambiente") {
    paramValue = modelItem.data(recNo, "obs").toString();
    return;
  }

  if (paramName == "Preço-R$") {
    double value = modelItem.data(recNo, "prcUnitario").toDouble();
    paramValue = "R$ " + locale.toString(value, 'f', 2);
    return;
  }

  if (paramName == "Quant.") {
    paramValue = modelItem.data(recNo, "quant").toString();
    return;
  }

  if (paramName == "Unid.") {
    paramValue = modelItem.data(recNo, "un").toString();
    return;
  }

  if (paramName == "TotalProd") {
    double parcial = modelItem.data(recNo, "parcial").toDouble();
    paramValue = "R$ " + locale.toString(parcial, 'f', 2);
    return;
  }

  // REPORT SUMMARY

  if (paramName == "Total") {
    double value = ui->doubleSpinBoxSubTotalLiq->value();
    paramValue = locale.toString(value, 'f', 2);
    return;
  }

  if (paramName == "Frete") {
    double value = ui->doubleSpinBoxFrete->value();
    paramValue = locale.toString(value, 'f', 2);
    return;
  }

  if (paramName == "TotalFinal") {
    double value = ui->doubleSpinBoxTotal->value();
    paramValue = locale.toString(value, 'f', 2);
    return;
  }

  if (paramName == "Observacao") {
    paramValue = ui->textEdit->toPlainText();
    return;
  }

  if (paramName == "Disclaimer") {
    paramValue =
        "1- O prazo de entrega deve ser consultado no momento da compra;\n2- Não aceitamos devolução de produtos "
        "calculados com percetual de perda, cortes/rodapés de porcelanatos ou mosaicos especiais;\n3- Produtos com "
        "classificação comercial \"C\" podem apresentar algum tipo de defeito, tendo valor especial por este motivo,  "
        "e devoluções não serão aceitas.";
    return;
  }

  if (paramName == "PrazoEntrega") {
    paramValue = ui->spinBoxPrazoEntrega->text();
    return;
  }

  if (paramName == "FormaPagamento1") {
    QSqlQuery queryPgt1;

    if (not queryPgt1.exec("SELECT data FROM conta_a_receber_has_pagamento WHERE idVenda = '" +
                           ui->lineEditVenda->text() + "' AND tipo LIKE '1%'") or
        not queryPgt1.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando pagamentos 1: " + queryPgt1.lastError().text());
      return;
    }

    QDate data = queryPgt1.value(0).toDate();

    if (not queryPgt1.exec(
          "SELECT tipo, COUNT(valor), valor, data FROM conta_a_receber_has_pagamento WHERE idVenda = '" +
          ui->lineEditVenda->text() + "' AND tipo LIKE '1%';") or
        not queryPgt1.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando pagamentos 1: " + queryPgt1.lastError().text());
      return;
    }

    if (queryPgt1.value(1) == 1) {
      paramValue = queryPgt1.value(0).toString() + " - " + queryPgt1.value(1).toString() + "x de R$ " +
                   locale.toString(queryPgt1.value(2).toDouble(), 'f', 2) + " - pag. em: " +
                   data.toString("dd-MM-yyyy");
      return;
    } else {
      paramValue = queryPgt1.value(0).toString() + " - " + queryPgt1.value(1).toString() + "x de R$ " +
                   locale.toString(queryPgt1.value(2).toDouble(), 'f', 2) + " - 1° pag. em: " +
                   data.toString("dd-MM-yyyy");
      return;
    }
  }

  if (paramName == "FormaPagamento2") {
    QSqlQuery queryPgt2;

    if (not queryPgt2.exec("SELECT data FROM conta_a_receber_has_pagamento WHERE idVenda = '" +
                           ui->lineEditVenda->text() + "' AND tipo LIKE '2%'")) {
      QMessageBox::critical(this, "Erro!", "Erro buscando pagamentos 2: " + queryPgt2.lastError().text());
      return;
    }

    if (not queryPgt2.first()) {
      return;
    }

    QDate data = queryPgt2.value(0).toDate();

    if (not queryPgt2.exec(
          "SELECT tipo, COUNT(valor), valor, data FROM conta_a_receber_has_pagamento WHERE idVenda = '" +
          ui->lineEditVenda->text() + "' AND tipo LIKE '2%';") or
        not queryPgt2.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando pagamentos 2: " + queryPgt2.lastError().text());
      return;
    }

    if (queryPgt2.value(2) == 0) {
      return;
    }

    if (queryPgt2.value(1) == 1) {
      paramValue = queryPgt2.value(0).toString() + " - " + queryPgt2.value(1).toString() + "x de R$ " +
                   locale.toString(queryPgt2.value(2).toDouble(), 'f', 2) + " - pag. em: " +
                   data.toString("dd-MM-yyyy");
      return;
    } else {
      paramValue = queryPgt2.value(0).toString() + " - " + queryPgt2.value(1).toString() + "x de R$ " +
                   locale.toString(queryPgt2.value(2).toDouble(), 'f', 2) + " - 1° pag. em: " +
                   data.toString("dd-MM-yyyy");
      return;
    }
  }

  if (paramName == "FormaPagamento3") {
    QSqlQuery queryPgt3;

    if (not queryPgt3.exec("SELECT data FROM conta_a_receber_has_pagamento WHERE idVenda = '" +
                           ui->lineEditVenda->text() + "' AND tipo LIKE '3%'")) {
      QMessageBox::critical(this, "Erro!", "Erro buscando pagamentos 3: " + queryPgt3.lastError().text());
      return;
    }

    if (not queryPgt3.first()) {
      return;
    }

    QDate data = queryPgt3.value(0).toDate();

    if (not queryPgt3.exec(
          "SELECT tipo, COUNT(valor), valor, data FROM conta_a_receber_has_pagamento WHERE idVenda = '" +
          ui->lineEditVenda->text() + "' AND tipo LIKE '3%';") or
        not queryPgt3.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando pagamentos 3: " + queryPgt3.lastError().text());
      return;
    }

    if (queryPgt3.value(2) == 0) {
      return;
    }

    if (queryPgt3.value(1) == 1) {
      paramValue = queryPgt3.value(0).toString() + " - " + queryPgt3.value(1).toString() + "x de R$ " +
                   locale.toString(queryPgt3.value(2).toDouble(), 'f', 2) + " - pag. em: " +
                   data.toString("dd-MM-yyyy");
      return;
    } else {
      paramValue = queryPgt3.value(0).toString() + " - " + queryPgt3.value(1).toString() + "x de R$ " +
                   locale.toString(queryPgt3.value(2).toDouble(), 'f', 2) + " - 1° pag. em: " +
                   data.toString("dd-MM-yyyy");
      return;
    }
  }
}

void Venda::successMessage() {
  QMessageBox::information(this, "Atenção!", "Venda cadastrada com sucesso!");

  close();
}

void Venda::on_pushButtonGerarExcel_clicked() {
  QFile modelo(QDir::currentPath() + "/modelo.xlsx");

  if (not modelo.exists()) {
    QMessageBox::critical(this, "Erro!", "Não encontrou o modelo do Excel!");
    return;
  }

  if (modelItem.rowCount() > 17) {
    QMessageBox::critical(this, "Erro!", "Mais itens do que cabe no modelo!");
    return;
  }

  QXlsx::Document xlsx("modelo.xlsx");

  QString idVenda = model.data(mapper.currentIndex(), "idVenda").toString();

  QSqlQuery queryVenda;
  queryVenda.prepare("SELECT * FROM venda WHERE idVenda = :idVenda");
  queryVenda.bindValue(":idVenda", idVenda);

  if (not queryVenda.exec() or not queryVenda.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados da venda: " + queryVenda.lastError().text());
    return;
  }

  QSqlQuery queryLoja;
  queryLoja.prepare("SELECT * FROM loja WHERE idLoja = (SELECT idLoja FROM venda WHERE idVenda = :idVenda)");
  queryLoja.bindValue(":idVenda", idVenda);

  if (not queryLoja.exec() or not queryLoja.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados da loja: " + queryLoja.lastError().text());
    return;
  }

  QSqlQuery queryUsuario;
  queryUsuario.prepare(
        "SELECT * FROM usuario WHERE idUsuario = (SELECT idUsuario FROM venda WHERE idVenda = :idVenda)");
  queryUsuario.bindValue(":idVenda", idVenda);

  if (not queryUsuario.exec() or not queryUsuario.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados do usuário: " + queryUsuario.lastError().text());
    return;
  }

  QSqlQuery queryCliente;
  queryCliente.prepare(
        "SELECT * FROM cliente WHERE idCliente = (SELECT idCliente FROM venda WHERE idVenda = :idVenda)");
  queryCliente.bindValue(":idVenda", idVenda);

  if (not queryCliente.exec() or not queryCliente.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados do cliente: " + queryCliente.lastError().text());
    return;
  }

  QSqlQuery queryEndEnt;
  queryEndEnt.prepare("SELECT * FROM cliente_has_endereco WHERE idEndereco = (SELECT idEnderecoEntrega FROM venda "
                      "WHERE idVenda = :idVenda)");
  queryEndEnt.bindValue(":idVenda", idVenda);

  if (not queryEndEnt.exec() or not queryEndEnt.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados do endereço: " + queryEndEnt.lastError().text());
    return;
  }

  QSqlQuery queryEndFat;
  queryEndFat.prepare("SELECT * FROM cliente_has_endereco WHERE idEndereco = (SELECT idEnderecoFaturamento FROM venda "
                      "WHERE idVenda = :idVenda)");
  queryEndFat.bindValue(":idVenda", idVenda);

  if (not queryEndFat.exec() or not queryEndFat.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados do endereço: " + queryEndFat.lastError().text());
    return;
  }

  QSqlQuery queryProf;
  queryProf.prepare(
        "SELECT * FROM profissional WHERE idProfissional = (SELECT idProfissional FROM venda WHERE idVenda = :idVenda)");
  queryProf.bindValue(":idVenda", idVenda);

  if (not queryProf.exec() or not queryProf.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados do profissional: " + queryProf.lastError().text());
    return;
  }

  xlsx.write("D2", queryVenda.value("idVenda"));
  xlsx.write("D3", queryCliente.value("nome_razao"));
  xlsx.write("D4", queryCliente.value("email"));
  xlsx.write("D5", queryEndFat.value("logradouro").toString() + " " + queryEndFat.value("numero").toString() + " - " +
             queryEndFat.value("bairro").toString() + ", " + queryEndFat.value("cidade").toString());
  xlsx.write("D6", queryEndEnt.value("logradouro").toString() + " " + queryEndEnt.value("numero").toString() + " - " +
             queryEndEnt.value("bairro").toString() + ", " + queryEndEnt.value("cidade").toString());
  xlsx.write("D7", queryProf.value("nome_razao").toString());
  xlsx.write("D8", queryUsuario.value("nome").toString());
  xlsx.write("F8", queryUsuario.value("email").toString());
  xlsx.write("M2", queryVenda.value("data").toDateTime().toString("dd/MM/yyyy hh:mm"));
  xlsx.write("M3", queryCliente.value(queryCliente.value("pfpj").toString() == "PF" ? "cpf" : "cnpj").toString());
  xlsx.write("M4", queryCliente.value("tel").toString());
  xlsx.write("M5", queryEndFat.value("cep").toString());
  xlsx.write("M6", queryEndEnt.value("cep").toString());
  xlsx.write("H7", queryProf.value("tel").toString());
  xlsx.write("K7", queryProf.value("email").toString());

  xlsx.write("N29", "R$ " + QString::number(queryVenda.value("subTotalLiq").toDouble(), 'f', 2)); // soma
  xlsx.write("N30", QString::number(queryVenda.value("descontoPorc").toDouble(), 'f', 2) + "%");  // desconto
  xlsx.write("N31", "R$ " + QString::number(queryVenda.value("subTotalLiq").toDouble() -
                                            (ui->doubleSpinBoxDescontoGlobal->value() / 100 *
                                             queryVenda.value("subTotalLiq").toDouble()),
                                            'f', 2));                                       // total
  xlsx.write("N32", "R$ " + QString::number(queryVenda.value("frete").toDouble(), 'f', 2)); // frete
  xlsx.write("N33", "R$ " + QString::number(queryVenda.value("total").toDouble(), 'f', 2)); // total final

  for (int i = 0; i < modelItem.rowCount(); ++i) {
    xlsx.write("A" + QString::number(12 + i), modelItem.data(i, "fornecedor").toString());
    xlsx.write("B" + QString::number(12 + i), modelItem.data(i, "codComercial").toString());
    xlsx.write("C" + QString::number(12 + i), modelItem.data(i, "produto").toString());
    xlsx.write("H" + QString::number(12 + i), modelItem.data(i, "obs").toString());
    xlsx.write("K" + QString::number(12 + i), modelItem.data(i, "prcUnitario").toDouble());
    xlsx.write("L" + QString::number(12 + i), modelItem.data(i, "quant").toDouble());
    xlsx.write("M" + QString::number(12 + i), modelItem.data(i, "un").toString());
    xlsx.write("N" + QString::number(12 + i), "R$ " + QString::number(modelItem.data(i, "total").toDouble(), 'f', 2));
  }

  QSettings settings("Staccato", "ERP");
  settings.beginGroup("User");
  QString path = settings.value("userFolder").toString();

  QDir dir(path);

  if (not dir.exists()) {
    dir.mkdir(path);
  }

  if (not xlsx.saveAs(path + "/" + ui->lineEditVenda->text() + ".xlsx")) {
    QMessageBox::critical(this, "Erro!", "Ocorreu algum erro ao salvar o arquivo.");
    return;
  }

  QMessageBox::information(this, "Ok!", "Arquivo salvo como " + path + "/" + ui->lineEditVenda->text() + ".xlsx");
  QDesktopServices::openUrl(QUrl::fromLocalFile(path + "/" + ui->lineEditVenda->text() + ".xlsx"));
}

void Venda::on_lineEditPgt1_textChanged(const QString &arg1) {
  Q_UNUSED(arg1);

  montarFluxoCaixa();
}

void Venda::on_lineEditPgt2_textChanged(const QString &arg1) {
  Q_UNUSED(arg1);

  montarFluxoCaixa();
}

void Venda::on_lineEditPgt3_textChanged(const QString &arg1) {
  Q_UNUSED(arg1);

  montarFluxoCaixa();
}
