#include <QSqlRecord>
#include <QSqlError>
#include <QMessageBox>
#include <QSqlQuery>

#include "venda.h"
#include "ui_venda.h"
#include "orcamento.h"
#include "usersession.h"
#include "cadastrarnfe.h"
#include "doubledelegate.h"
#include "checkboxdelegate.h"
#include "qtrpt.h"
#include "cadastrocliente.h"

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
  QStringList list{"Escolha uma opção!", "Cartão de débito", "Cartão de crédito", "Cheque", "Dinheiro", "Boleto"};

  ui->comboBoxPgt1->insertItems(0, list);
  ui->comboBoxPgt2->insertItems(0, list);
  ui->comboBoxPgt3->insertItems(0, list);

  ui->dateEditPgt1->setDate(QDate::currentDate());
  ui->dateEditPgt2->setDate(QDate::currentDate());
  ui->dateEditPgt3->setDate(QDate::currentDate());

  setupMapper();
  newRegister();

  foreach (const QLineEdit *line, findChildren<QLineEdit *>()) {
    connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);
  }

  showMaximized();
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
  modelItem.setHeaderData(modelItem.fieldIndex("qte"), Qt::Horizontal, "Quant.");
  modelItem.setHeaderData(modelItem.fieldIndex("un"), Qt::Horizontal, "Un.");
  modelItem.setHeaderData(modelItem.fieldIndex("unCaixa"), Qt::Horizontal, "Un./Caixa");
  modelItem.setHeaderData(modelItem.fieldIndex("codComercial"), Qt::Horizontal, "Cód. Com.");
  modelItem.setHeaderData(modelItem.fieldIndex("formComercial"), Qt::Horizontal, "Form. Com.");
  modelItem.setHeaderData(modelItem.fieldIndex("parcial"), Qt::Horizontal, "Parcial");
  modelItem.setHeaderData(modelItem.fieldIndex("desconto"), Qt::Horizontal, "Desconto");
  modelItem.setHeaderData(modelItem.fieldIndex("parcialDesc"), Qt::Horizontal, "Desc. Parc.");
  modelItem.setHeaderData(modelItem.fieldIndex("descGlobal"), Qt::Horizontal, "Desc. Glob.");
  modelItem.setHeaderData(modelItem.fieldIndex("total"), Qt::Horizontal, "Total");

  if (not modelItem.select()) {
    qDebug() << "erro modelItem: " << modelItem.lastError();
    return;
  }

  modelFluxoCaixa.setTable("venda_has_pagamento");
  modelFluxoCaixa.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelFluxoCaixa.setHeaderData(modelFluxoCaixa.fieldIndex("tipo"), Qt::Horizontal, "Tipo");
  modelFluxoCaixa.setHeaderData(modelFluxoCaixa.fieldIndex("parcela"), Qt::Horizontal, "Parcela");
  modelFluxoCaixa.setHeaderData(modelFluxoCaixa.fieldIndex("valor"), Qt::Horizontal, "R$");
  modelFluxoCaixa.setHeaderData(modelFluxoCaixa.fieldIndex("data"), Qt::Horizontal, "Data");

  if (not modelFluxoCaixa.select()) {
    qDebug() << "erro modelFluxoCaixa: " << modelFluxoCaixa.lastError();
    return;
  }

  ui->tableFluxoCaixa->setModel(&modelFluxoCaixa);
  ui->tableFluxoCaixa->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableFluxoCaixa->setColumnHidden(modelFluxoCaixa.fieldIndex("idVenda"), true);
  ui->tableFluxoCaixa->setColumnHidden(modelFluxoCaixa.fieldIndex("idLoja"), true);
  ui->tableFluxoCaixa->setColumnHidden(modelFluxoCaixa.fieldIndex("idPagamento"), true);

  ui->tableVenda->setModel(&modelItem);
  ui->tableVenda->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableVenda->setColumnHidden(modelItem.fieldIndex("idVenda"), true);
  ui->tableVenda->setColumnHidden(modelItem.fieldIndex("idLoja"), true);
  ui->tableVenda->setColumnHidden(modelItem.fieldIndex("idProduto"), true);
  ui->tableVenda->setColumnHidden(modelItem.fieldIndex("item"), true);
  ui->tableVenda->setColumnHidden(modelItem.fieldIndex("status"), true);

  ui->tableFluxoCaixa->setItemDelegate(new DoubleDelegate(this));
  ui->tableVenda->setItemDelegate(new DoubleDelegate(this));

  ui->tableVenda->setItemDelegateForColumn(modelItem.fieldIndex("selecionado"), new CheckBoxDelegate(this));
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
    qDebug() << "Erro buscando produtos: " << queryProdutos.lastError();
  }

  QSqlQuery queryOrc;
  queryOrc.prepare("SELECT * FROM orcamento WHERE idOrcamento = :idOrcamento");
  queryOrc.bindValue(":idOrcamento", idOrcamento);

  if (not queryOrc.exec() or not queryOrc.first()) {
    qDebug() << "Erro buscando orcamento: " << queryOrc.lastError();
  }

  QSqlQuery queryCliente;
  queryCliente.prepare("SELECT * FROM cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", queryOrc.value("idCliente"));

  if (not queryCliente.exec() or not queryCliente.first()) {
    qDebug() << "Erro buscando cliente: " << queryCliente.lastError();
    qDebug() << "Não achou venda: " << queryCliente.size();
    qDebug() << "query: " << queryCliente.lastQuery();
    qDebug() << "id: " << queryOrc.value("idCliente");
  }

  QSqlQuery queryEndFat;
  queryEndFat.prepare("SELECT * FROM cliente_has_endereco WHERE idEndereco = :idEndereco");
  queryEndFat.bindValue(":idEndereco", queryOrc.value("idEnderecoFaturamento"));

  if (not queryEndFat.exec() or not queryEndFat.first()) {
    qDebug() << "Erro buscando endereco faturamento: " << queryEndFat.lastError();
    qDebug() << "Não achou venda: " << queryEndFat.size();
    qDebug() << "query: " << queryEndFat.lastQuery();
    qDebug() << "id: " << queryOrc.value("idEnderecoFaturamento");
  }

  QSqlQuery queryEndEnt;
  queryEndEnt.prepare("SELECT * FROM cliente_has_endereco WHERE idEndereco= :idEndereco");
  queryEndEnt.bindValue(":idEndereco", queryOrc.value("idEnderecoEntrega"));

  if (not queryEndEnt.exec() or not queryEndEnt.first()) {
    qDebug() << "Erro buscando endereco entrega: " << queryEndEnt.lastError();
    qDebug() << "Não achou venda: " << queryEndEnt.size();
    qDebug() << "query: " << queryEndEnt.lastQuery();
    qDebug() << "id: " << queryOrc.value("idEnderecoEntrega");
  }

  QSqlQuery queryVendedor;
  queryVendedor.prepare("SELECT * FROM usuario WHERE idUsuario = :idUsuario");
  queryVendedor.bindValue(":idUsuario", queryOrc.value("idUsuario"));

  if (not queryVendedor.exec() or not queryVendedor.first()) {
    qDebug() << "Erro buscando vendedor: " << queryVendedor.lastError();
    qDebug() << "Não achou venda: " << queryVendedor.size();
    qDebug() << "query: " << queryVendedor.lastQuery();
    qDebug() << "id: " << queryOrc.value("idUsuario");
  }

  QSqlQuery queryProfissional;
  queryProfissional.prepare("SELECT * FROM profissional WHERE idProfissional = :idProfissional");
  queryProfissional.bindValue(":idProfissional", queryOrc.value("idProfissional"));

  if (not queryProfissional.exec() or not queryProfissional.first()) {
    qDebug() << "Erro buscando profissional: " << queryProfissional.lastError();
    qDebug() << "Não achou venda: " << queryProfissional.size();
    qDebug() << "query: " << queryProfissional.lastQuery();
    qDebug() << "id: " << queryOrc.value("idProfissional");
  }

  modelItem.setFilter("idVenda = '" + idOrcamento + "'");

  while (queryProdutos.next()) {
    const int rowItem = modelItem.rowCount();
    modelItem.insertRow(rowItem);

    if (not modelItem.setData(modelItem.index(rowItem, modelItem.fieldIndex("idVenda")),
                              queryProdutos.value("idOrcamento"))) {
      qDebug() << "Erro setando idVenda: " << modelItem.lastError();
    }

    for (int field = 1, fieldCount = queryProdutos.record().count(); field < fieldCount; ++field) {
      if (modelItem.fieldIndex(queryProdutos.record().fieldName(field)) != -1 and
          not modelItem.setData(modelItem.index(rowItem, modelItem.fieldIndex(queryProdutos.record().fieldName(field))),
                                queryProdutos.value(queryProdutos.record().fieldName(field)))) {
        qDebug() << "Erro setando itens venda: " << modelItem.lastError();
      }
    }

    modelItem.setData(modelItem.index(rowItem, modelItem.fieldIndex("status")), "PENDENTE");
  }

  if (not model.setData(model.index(row, model.fieldIndex("idVenda")), queryOrc.value("idOrcamento"))) {
    qDebug() << "erro setando idVenda";
  }

  for (int field = 1, columnCount = queryOrc.record().count(); field < columnCount; ++field) {
    if (model.fieldIndex(queryOrc.record().fieldName(field)) != -1 and
        not model.setData(model.index(mapper.currentIndex(), model.fieldIndex(queryOrc.record().fieldName(field))),
                          queryOrc.value(field))) {
      qDebug() << "erro setando dados venda: " << model.lastError();
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

  ui->tableVenda->resizeColumnsToContents();
}

bool Venda::verifyFields(const int row) {
  Q_UNUSED(row);

  if (ui->spinBoxPrazoEntrega->value() == 0) {
    QMessageBox::warning(this, "Aviso!", "Por favor preencha o prazo de entrega.");
    return false;
  }

  if (ui->doubleSpinBoxPgt1->value() + ui->doubleSpinBoxPgt2->value() + ui->doubleSpinBoxPgt3->value() <
      ui->doubleSpinBoxTotalPag->value()) {
    QMessageBox::warning(this, "Aviso!", "Soma dos pagamentos não é igual ao total! Favor verificar.");
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

bool Venda::verifyRequiredField(QLineEdit *line) {
  Q_UNUSED(line);

  return true;
}

QString Venda::requiredStyle() { return QString(); }

void Venda::calcPrecoGlobalTotal(const bool ajusteTotal) {
  double subTotal = 0.;
  double subTotalItens = 0.;
  double subTotalBruto = 0.;
  double minimoFrete = 0., porcFrete = 0.;

  QSqlQuery queryFrete;
  queryFrete.prepare("SELECT * FROM loja WHERE idLoja = :idLoja");
  queryFrete.bindValue(":idLoja", UserSession::getLoja());

  if (not queryFrete.exec() or not queryFrete.first()) {
    qDebug() << "Erro buscando parâmetros do frete: " << queryFrete.lastError();
    return;
  }

  minimoFrete = queryFrete.value("valorMinimoFrete").toDouble();
  porcFrete = queryFrete.value("porcentagemFrete").toDouble();

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    double prcUnItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("prcUnitario"))).toDouble();
    double qteItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("qte"))).toDouble();
    double descItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("desconto"))).toDouble() / 100.;
    double itemBruto = qteItem * prcUnItem;
    subTotalBruto += itemBruto;
    double stItem = itemBruto * (1. - descItem);
    subTotalItens += stItem;
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("parcial")), itemBruto);
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("parcialDesc")), stItem);
  }

  double frete = qMax(subTotalBruto * porcFrete / 100., minimoFrete);

  if (ui->checkBoxFreteManual->isChecked()) {
    frete = ui->doubleSpinBoxFrete->value();
  }

  double descGlobal = ui->doubleSpinBoxDescontoGlobal->value() / 100.0;
  subTotal = subTotalItens * (1.0 - descGlobal);

  if (ajusteTotal) {
    const double Final = ui->doubleSpinBoxTotal->value();
    subTotal = Final - frete;

    if (subTotalItens == 0.) {
      descGlobal = 0;
    } else {
      descGlobal = 1 - (subTotal / subTotalItens);
    }
  }

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("descGlobal")), descGlobal * 100.0);
    double stItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("parcialDesc"))).toDouble();
    double totalItem = stItem * (1 - descGlobal);
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("total")), totalItem);
  }

  // TODO: refatorar para que essa função não seja chamada duas vezes

  ui->doubleSpinBoxSubTotalBruto->setValue(subTotalBruto);
  ui->doubleSpinBoxSubTotalLiq->setValue(subTotalItens);
  ui->doubleSpinBoxDescontoGlobal->setValue(descGlobal * 100);
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
    qDebug() << "Erro buscando orçamento: " << query.lastError();
    qDebug() << "query: " << query.lastQuery();
  }

  if (not query.first()) {
    query.prepare("SELECT * FROM venda WHERE idVenda = :idVenda");
    query.bindValue(":idVenda", ui->lineEditVenda->text());

    if (not query.exec() or not query.first()) {
      qDebug() << "Erro buscando venda: " << query.lastError();
      qDebug() << "Não achou venda: " << query.size();
      qDebug() << "query: " << query.lastQuery();
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
  addMapping(ui->itemBoxCliente, "idCliente", "value");
  addMapping(ui->itemBoxEndereco, "idEnderecoEntrega", "value");
  addMapping(ui->itemBoxEnderecoFat, "idEnderecoFaturamento", "value");
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

void Venda::on_pushButtonNFe_clicked() {
  CadastrarNFe *cadNFe = new CadastrarNFe(ui->lineEditVenda->text(), this);

  QList<int> lista;

  foreach (const QModelIndex index, modelItem.match(modelItem.index(0, 0), Qt::DisplayRole, true, -1)) {
    lista.append(index.row());
  }

  if (lista.size() == 0) {
    QMessageBox::warning(this, "Aviso!", "Nenhum item selecionado!");
    return;
  }

  cadNFe->prepararNFe(lista);
  cadNFe->showMaximized();
}

void Venda::calculoSpinBox1() {
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

void Venda::calculoSpinBox2() {
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

  if (text == "Cheque" or text == "Boleto") {
    ui->dateEditPgt1->setEnabled(true);
  } else {
    ui->dateEditPgt1->setDisabled(true);
  }

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

  if (text == "Cheque" or text == "Boleto") {
    ui->dateEditPgt2->setEnabled(true);
  } else {
    ui->dateEditPgt2->setDisabled(true);
  }

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

  if (text == "Cheque" or text == "Boleto") {
    ui->dateEditPgt3->setEnabled(true);
  } else {
    ui->dateEditPgt3->setDisabled(true);
  }

  montarFluxoCaixa();
}

bool Venda::savingProcedures(int row) {
  setData(row, "idEnderecoEntrega", ui->itemBoxEndereco->value());
  setData(row, "idEnderecoFaturamento", ui->itemBoxEnderecoFat->value());
  setData(row, "status", "ABERTO");
  setData(row, "data", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
  setData(row, "prazoEntrega", ui->spinBoxPrazoEntrega->value());
  setData(row, "observacao", ui->textEdit->toPlainText());

  if (not model.submitAll()) {
    qDebug() << "Erro submetendo model: " << model.lastError();
    return false;
  }

  if (not modelFluxoCaixa.submitAll()) {
    qDebug() << "Erro submetendo modelFluxoCaixa: " << modelFluxoCaixa.lastError();
    return false;
  }

  if (not modelItem.submitAll()) {
    qDebug() << "Erro submetendo modelItem: " << modelItem.lastError();
    return false;
  }

  QSqlQuery query;

  query.prepare("SELECT produto.descricao, produto.estoque, venda_has_produto.idVenda FROM venda_has_produto INNER "
                "JOIN produto ON produto.idProduto = venda_has_produto.idProduto WHERE estoque = 0 AND "
                "venda_has_produto.idVenda = :idVenda");
  query.bindValue(":idVenda", ui->lineEditVenda->text());

  if (not query.exec()) {
    qDebug() << "Erro na verificação de estoque: " << query.lastError();
    return false;
  }

  if (query.size() > 0) {
    query.prepare("INSERT INTO pedido_fornecedor (idPedido, idLoja, idUsuario, idCliente, "
                  "idEnderecoEntrega, idProfissional, data, subTotalBru, subTotalLiq, frete, descontoPorc, "
                  "descontoReais, total, validade, status) SELECT idVenda, idLoja, idUsuario, idCliente, "
                  "idEnderecoEntrega, idProfissional, data, subTotalBru, subTotalLiq, frete, descontoPorc, "
                  "descontoReais, total, validade, status "
                  "FROM venda WHERE idVenda = :idVenda");
    query.bindValue(":idVenda", ui->lineEditVenda->text());

    if (not query.exec()) {
      qDebug() << "Erro na criação do pedido fornecedor: " << query.lastError();
    }

    query.prepare("INSERT INTO pedido_fornecedor_has_Produto (idPedido, idLoja, item, idProduto, fornecedor, produto, "
                  "obs, prcUnitario, caixas, qte, un, unCaixa, codComercial, formComercial, parcial, desconto, "
                  "parcialDesc, descGlobal, total, status) SELECT v.idVenda, v.idLoja, v.item, v.idProduto, "
                  "v.fornecedor, v.produto, v.obs, v.prcUnitario, v.caixas, v.qte, v.un, v.unCaixa, v.codComercial, "
                  "v.formComercial, v.parcial, v.desconto, v.parcialDesc, v.descGlobal, v.total, v.status FROM "
                  "venda_has_produto AS v INNER JOIN produto ON produto.idProduto = v.idProduto WHERE estoque = 0 AND "
                  "v.idVenda = :idVenda");
    query.bindValue(":idVenda", ui->lineEditVenda->text());

    if (not query.exec()) {
      qDebug() << "Erro na inserção de produtos em pedido_fornecedor_has_produto: " << query.lastError();
      return false;
    }
  }

  query.prepare("DELETE FROM orcamento_has_produto WHERE idOrcamento = :idVenda");
  query.bindValue(":idVenda", ui->lineEditVenda->text());

  if (not query.exec()) {
    qDebug() << "Erro deletando itens no orcamento_has_produto: " << query.lastError();
    return false;
  }

  query.prepare("DELETE FROM orcamento WHERE idOrcamento = :idVenda");
  query.bindValue(":idVenda", ui->lineEditVenda->text());

  if (not query.exec()) {
    qDebug() << "Erro deletando Orcamento: " << query.lastError();
    return false;
  }

  query.prepare("INSERT INTO conta_a_receber (dataEmissao, idVenda) VALUES (:dataEmissao, :idVenda)");
  query.bindValue(":dataEmissao", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
  query.bindValue(":idVenda", ui->lineEditVenda->text());

  if (not query.exec()) {
    qDebug() << "Erro inserindo conta_a_receber: " << query.lastError();
    return false;
  }

  return true;
}

void Venda::registerMode() {
  ui->framePagamentos->show();
  ui->pushButtonNFe->hide();
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
  ui->pushButtonNFe->show();
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

  ui->lineEditVenda->setText(data(primaryKey).toString());

  modelItem.setFilter("idVenda = '" + ui->lineEditVenda->text() + "'");

  if (not modelItem.select()) {
    qDebug() << "erro modelItem: " << modelItem.lastError();
    return false;
  }

  modelFluxoCaixa.setFilter("idVenda = '" + ui->lineEditVenda->text() + "'");

  if (not modelFluxoCaixa.select()) {
    qDebug() << "erro modelFluxoCaixa: " << modelFluxoCaixa.lastError();
    return false;
  }

  ui->tableFluxoCaixa->resizeColumnsToContents();

  for (int i = 0; i < modelItem.rowCount(); ++i) {
    ui->tableVenda->openPersistentEditor(modelItem.index(i, 0));
  }

  fillTotals();

  QSqlQuery query;
  query.prepare("SELECT * FROM orcamento WHERE idOrcamento = :idOrcamento");
  query.bindValue(":idOrcamento", ui->lineEditVenda->text());

  if (not query.exec()) {
    qDebug() << "Erro buscando orçamento: " << query.lastError();
    qDebug() << "query: " << query.lastQuery();
  }

  if (not query.first()) {
    query.prepare("SELECT * FROM venda WHERE idVenda = :idVenda");
    query.bindValue(":idVenda", ui->lineEditVenda->text());

    if (not query.exec() or not query.first()) {
      qDebug() << "Erro buscando venda: " << query.lastError();
      qDebug() << "Não achou venda: " << query.size();
      qDebug() << "query: " << query.lastQuery();
    }
  }

  ui->lineEditVenda->setText(query.value("idVenda").toString());
  ui->itemBoxVendedor->setValue(query.value("idUsuario"));
  ui->itemBoxCliente->setValue(query.value("idCliente"));
  ui->dateTimeEdit->setDateTime(query.value("data").toDateTime());
  ui->itemBoxProfissional->setValue(query.value("idProfissional"));
  ui->itemBoxEndereco->setValue(query.value("idEnderecoEntrega"));

  calcPrecoGlobalTotal();

  ui->tableVenda->resizeColumnsToContents();

  return true;
}

void Venda::on_pushButtonVoltar_clicked() {
  Orcamento *orcamento = new Orcamento(parentWidget());
  orcamento->viewRegisterById(ui->lineEditVenda->text());
  orcamento->showMaximized();

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

  if (ui->comboBoxPgt1->currentText() != "Escolha uma opção!") {
    const int parcelas = ui->comboBoxPgt1Parc->currentIndex() + 1;
    const double valor = ui->doubleSpinBoxPgt1->value();

    const float temp = static_cast<float>(static_cast<int>(static_cast<float>(valor / parcelas) * 100)) / 100;
    const double resto = static_cast<double>(valor - (temp * parcelas));
    const double parcela = static_cast<double>(temp);

    for (int i = 0, z = parcelas - 1; i < parcelas; ++i, --z) {
      modelFluxoCaixa.insertRow(modelFluxoCaixa.rowCount());
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("idVenda")),
                              ui->lineEditVenda->text());
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("idLoja")), UserSession::getLoja());
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("tipo")),
                              "1. " + ui->comboBoxPgt1->currentText());
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("parcela")), parcelas - z);

      if (i == 0) {
        modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("valor")), parcela + resto);
      } else {
        modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("valor")), parcela);
      }

      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("data")),
                              ui->dateEditPgt1->date().addMonths(i));
      ++row;
    }
  }

  if (ui->comboBoxPgt2->currentText() != "Escolha uma opção!") {
    const int parcelas = ui->comboBoxPgt2Parc->currentIndex() + 1;
    const double valor = ui->doubleSpinBoxPgt2->value();

    const float temp = static_cast<float>(static_cast<int>(static_cast<float>(valor / parcelas) * 100)) / 100;
    const double resto = static_cast<double>(valor - (temp * parcelas));
    const double parcela = static_cast<double>(temp);

    for (int i = 0, z = parcelas - 1; i < parcelas; ++i, --z) {
      modelFluxoCaixa.insertRow(modelFluxoCaixa.rowCount());
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("idVenda")),
                              ui->lineEditVenda->text());
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("idLoja")), UserSession::getLoja());
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("tipo")),
                              "2. " + ui->comboBoxPgt2->currentText());
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("parcela")), parcelas - z);

      if (i == 0) {
        modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("valor")), parcela + resto);
      } else {
        modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("valor")), parcela);
      }

      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("data")),
                              ui->dateEditPgt2->date().addMonths(i));
      ++row;
    }
  }

  if (ui->comboBoxPgt3->currentText() != "Escolha uma opção!") {
    const int parcelas = ui->comboBoxPgt3Parc->currentIndex() + 1;
    const double valor = ui->doubleSpinBoxPgt3->value();

    const float temp = static_cast<float>(static_cast<int>(static_cast<float>(valor / parcelas) * 100)) / 100;
    const double resto = static_cast<double>(valor - (temp * parcelas));
    const double parcela = static_cast<double>(temp);

    for (int i = 0, z = parcelas - 1; i < parcelas; ++i, --z) {
      modelFluxoCaixa.insertRow(modelFluxoCaixa.rowCount());
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("idVenda")),
                              ui->lineEditVenda->text());
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("idLoja")), UserSession::getLoja());
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("tipo")),
                              "3. " + ui->comboBoxPgt3->currentText());
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("parcela")), parcelas - z);

      if (i == 0) {
        modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("valor")), parcela + resto);
      } else {
        modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("valor")), parcela);
      }

      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("data")),
                              ui->dateEditPgt3->date().addMonths(i));
      ++row;
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

  if (checked) {
    ui->doubleSpinBoxFrete->setButtonSymbols(QDoubleSpinBox::UpDownArrows);
  } else {
    ui->doubleSpinBoxFrete->setButtonSymbols(QDoubleSpinBox::NoButtons);
  }

  calcPrecoGlobalTotal();
}

void Venda::on_doubleSpinBoxFrete_editingFinished() { calcPrecoGlobalTotal(); }

void Venda::on_doubleSpinBoxDescontoGlobal_valueChanged(const double) { calcPrecoGlobalTotal(); }

void Venda::on_pushButtonImprimir_clicked() {
  QtRPT *report = new QtRPT(this);
  QFile file(qApp->applicationDirPath() + "/venda.xml");

  if (not file.exists()) {
    QMessageBox::warning(this, "Aviso!", "XML da impressão não encontrado!");
    return;
  }

  report->loadReport(file.fileName());
  report->recordCount << ui->tableVenda->model()->rowCount();
  connect(report, &QtRPT::setValue, this, &Venda::setValue);
  report->printExec();
}

void Venda::setValue(const int recNo, const QString paramName, QVariant &paramValue, const int reportPage) {
  Q_UNUSED(reportPage);
  QLocale locale;

  QSqlQuery queryCliente;
  queryCliente.prepare("SELECT * FROM cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", ui->itemBoxCliente->value());

  if (not queryCliente.exec() or not queryCliente.first()) {
    qDebug() << "Erro buscando cliente: " << model.fieldIndex("idCliente") << " - " << model.lastError();
  }

  QSqlQuery queryProduto;
  queryProduto.prepare("SELECT * FROM produto WHERE idProduto = :idProduto");
  queryProduto.bindValue(":idProduto", modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("idProduto"))));

  if (not queryProduto.exec() or not queryProduto.first()) {
    qDebug() << "Erro buscando produto: " << modelItem.fieldIndex("idProduto") << " - " << modelItem.lastError();
  }

  QSqlQuery queryProfissional;
  queryProfissional.prepare("SELECT * FROM profissional WHERE idProfissional = :idProfissional");
  queryProfissional.bindValue(":idProfissional", ui->itemBoxProfissional->value());

  if (not queryProfissional.exec() or not queryProfissional.first()) {
    qDebug() << "Erro buscando profissional: " << queryProfissional.lastError();
  }

  QSqlQuery queryVendedor;
  queryVendedor.prepare("SELECT * FROM usuario WHERE idUsuario = :idUsuario");
  queryVendedor.bindValue(":idUsuario", ui->itemBoxVendedor->value());

  if (not queryVendedor.exec() or not queryVendedor.first()) {
    qDebug() << "Erro buscando vendedor: " << queryVendedor.lastError();
  }

  QSqlQuery queryEndereco;
  queryEndereco.prepare("SELECT * FROM cliente_has_endereco WHERE idEndereco = :idEndereco");
  queryEndereco.bindValue(":idEndereco", ui->itemBoxEndereco->value());

  if (not queryEndereco.exec() or not queryEndereco.first()) {
    qDebug() << "Erro buscando endereco: " << queryEndereco.lastError();
  }

  // REPORT TITLE
  if (paramName == "pedido") {
    paramValue = ui->lineEditVenda->text();
  }

  if (paramName == "data") {
    paramValue = ui->dateTimeEdit->dateTime().toString("hh:mm dd-MM-yyyy");
  }

  if (paramName == "cliente") {
    paramValue = ui->itemBoxCliente->text();
  }

  if (paramName == "cpfcnpj") {
    paramValue = queryCliente.value("pfpj").toString() == "PF" ? queryCliente.value("cpf") : queryCliente.value("cnpj");
  }

  if (paramName == "email") {
    paramValue = queryCliente.value("email");
  }

  if (paramName == "tel1") {
    paramValue = queryCliente.value("tel");
  }

  if (paramName == "tel2") {
    paramValue = queryCliente.value("telCel");
  }

  if (paramName == "endfiscal") {
    QString endereco = ui->itemBoxEndereco->text();

    if (endereco != "Não há") {
      endereco = endereco.remove(0, endereco.indexOf("-") + 2);
    }

    paramValue = endereco;
  }

  if (paramName == "cepfiscal") {
    paramValue = queryEndereco.value("cep");
  }

  if (paramName == "endentrega") {
    QString endereco = ui->itemBoxEndereco->text();

    if (endereco != "Não há") {
      endereco = endereco.remove(0, endereco.indexOf("-") + 2);
    }

    paramValue = endereco;
  }

  if (paramName == "cepentrega") {
    paramValue = queryEndereco.value("cep");
  }

  if (paramName == "profissional") {
    if (ui->itemBoxProfissional->text().isEmpty()) {
      paramValue = "Não há";
    } else {
      paramValue = ui->itemBoxProfissional->text();
    }
  }

  if (paramName == "telprofissional") {
    paramValue = queryProfissional.value("tel");
  }

  if (paramName == "emailprofissional") {
    paramValue = queryProfissional.value("email");
  }

  if (paramName == "vendedor") {
    paramValue = ui->itemBoxVendedor->text();
  }

  if (paramName == "emailvendedor") {
    paramValue = queryVendedor.value("email");
  }

  if (paramName == "estoque") {
  }

  if (paramName == "dataestoque") {
  }

  // MASTER BAND
  if (paramName == "Marca") {
    paramValue = modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("fornecedor"))).toString();
  }

  if (paramName == "Código") {
    paramValue = queryProduto.value("codComercial").toString();
  }

  if (paramName == "Nome do produto") {
    paramValue = modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("produto"))).toString();
  }

  if (paramName == "Ambiente") {
    paramValue = modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("obs"))).toString();
  }

  if (paramName == "Preço-R$") {
    double value = modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("prcUnitario"))).toDouble();
    paramValue = "R$ " + locale.toString(value, 'f', 2);
  }

  if (paramName == "Quant.") {
    paramValue = modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("qte"))).toString();
  }

  if (paramName == "Unid.") {
    paramValue = modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("un"))).toString();
  }

  if (paramName == "TotalProd") {
    double parcial = modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("parcial"))).toDouble();
    paramValue = "R$ " + locale.toString(parcial, 'f', 2);
  }

  // REPORT SUMMARY

  if (paramName == "Total") {
    double value = ui->doubleSpinBoxSubTotalLiq->value();
    paramValue = locale.toString(value, 'f', 2);
  }

  if (paramName == "Frete") {
    double value = ui->doubleSpinBoxFrete->value();
    paramValue = locale.toString(value, 'f', 2);
  }

  if (paramName == "TotalFinal") {
    double value = ui->doubleSpinBoxTotal->value();
    paramValue = locale.toString(value, 'f', 2);
  }

  if (paramName == "Observacao") {
    paramValue = ui->textEdit->toPlainText();
  }

  if (paramName == "PrazoEntrega") {
    paramValue = ui->spinBoxPrazoEntrega->text();
  }

  if (paramName == "FormaPagamento1") {
    QSqlQuery queryPgt1;
    if (not queryPgt1.exec("SELECT tipo, COUNT(valor), valor, data FROM mydb.venda_has_pagamento WHERE idVenda = '" +
                           ui->lineEditVenda->text() + "' AND tipo LIKE '1%';") or
        not queryPgt1.first()) {
      qDebug() << "Erro buscando pagamentos: " << queryPgt1.lastError();
      return;
    }

    if (queryPgt1.value(1) == 1) {
      paramValue = queryPgt1.value(0).toString() + " - " + queryPgt1.value(1).toString() + "x de R$ " +
                   locale.toString(queryPgt1.value(2).toDouble(), 'f', 2) + " - pag. em: " +
                   queryPgt1.value(3).toString();
    } else {
      paramValue = queryPgt1.value(0).toString() + " - " + queryPgt1.value(1).toString() + "x de R$ " +
                   locale.toString(queryPgt1.value(2).toDouble(), 'f', 2) + " - 1° pag. em: " +
                   queryPgt1.value(3).toString();
    }
  }

  if (paramName == "FormaPagamento2") {
    QSqlQuery queryPgt2;
    if (not queryPgt2.exec("SELECT tipo, COUNT(valor), valor, data FROM mydb.venda_has_pagamento WHERE idVenda = '" +
                           ui->lineEditVenda->text() + "' AND tipo LIKE '2%';") or
        not queryPgt2.first()) {
      qDebug() << "Erro buscando pagamentos: " << queryPgt2.lastError();
      return;
    }

    if (queryPgt2.value(2) == 0) {
      return;
    }

    if (queryPgt2.value(1) == 1) {
      paramValue = queryPgt2.value(0).toString() + " - " + queryPgt2.value(1).toString() + "x de R$ " +
                   locale.toString(queryPgt2.value(2).toDouble(), 'f', 2) + " - pag. em: " +
                   queryPgt2.value(3).toString();
    } else {
      paramValue = queryPgt2.value(0).toString() + " - " + queryPgt2.value(1).toString() + "x de R$ " +
                   locale.toString(queryPgt2.value(2).toDouble(), 'f', 2) + " - 1° pag. em: " +
                   queryPgt2.value(3).toString();
    }
  }

  if (paramName == "FormaPagamento3") {
    QSqlQuery queryPgt3;
    if (not queryPgt3.exec("SELECT tipo, COUNT(valor), valor, data FROM mydb.venda_has_pagamento WHERE idVenda = '" +
                           ui->lineEditVenda->text() + "' AND tipo LIKE '3%';") or
        not queryPgt3.first()) {
      qDebug() << "Erro buscando pagamentos: " << queryPgt3.lastError();
      return;
    }

    if (queryPgt3.value(2) == 0) {
      return;
    }

    if (queryPgt3.value(1) == 1) {
      paramValue = queryPgt3.value(0).toString() + " - " + queryPgt3.value(1).toString() + "x de R$ " +
                   locale.toString(queryPgt3.value(2).toDouble(), 'f', 2) + " - pag. em: " +
                   queryPgt3.value(3).toString();
    } else {
      paramValue = queryPgt3.value(0).toString() + " - " + queryPgt3.value(1).toString() + "x de R$ " +
                   locale.toString(queryPgt3.value(2).toDouble(), 'f', 2) + " - 1° pag. em: " +
                   queryPgt3.value(3).toString();
    }
  }
}

void Venda::successMessage() {
  QMessageBox::information(this, "Atenção!", "Venda cadastrada com sucesso!", QMessageBox::Ok, QMessageBox::NoButton);

  close();
}
