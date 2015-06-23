#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlRelationalDelegate>
#include <QTime>
#include <QXmlStreamWriter>
#include <QPrintPreviewDialog>
#include <QWebPage>
#include <QDir>
#include <QTextCodec>
#include <QTextDocument>
#include <QWebFrame>
#include <QFileDialog>

#include "mainwindow.h"
#include "orcamento.h"
#include "ui_venda.h"
#include "venda.h"
#include "usersession.h"
#include "cadastrocliente.h"
#include "cadastrarnfe.h"
#include "endereco.h"
#include "doubledelegate.h"
#include "checkboxdelegate.h"
#include "qtrpt.h"

Venda::Venda(QWidget *parent) : RegisterDialog("Venda", "idVenda", parent), ui(new Ui::Venda) {
  ui->setupUi(this);

  setupTables();

  // TODO: make this runtime changeable
  QStringList list{"Escolha uma opção!", "Cartão de débito", "Cartão de crédito", "Cheque", "Dinheiro", "Boleto"};

  ui->comboBoxPgt1->insertItems(0, list);
  ui->comboBoxPgt2->insertItems(0, list);
  ui->comboBoxPgt3->insertItems(0, list);

  ui->pushButtonNFe->hide();

  ui->dateEditPgt1->setDate(QDate::currentDate());
  ui->dateEditPgt2->setDate(QDate::currentDate());
  ui->dateEditPgt3->setDate(QDate::currentDate());

  SearchDialog *sdEndereco = SearchDialog::enderecoCliente(ui->itemBoxEndereco);
  ui->itemBoxEndereco->setSearchDialog(sdEndereco);

  SearchDialog *sdEnderecoFat = SearchDialog::enderecoCliente(ui->itemBoxEnderecoFat);
  ui->itemBoxEnderecoFat->setSearchDialog(sdEnderecoFat);

  setupMapper();
  newRegister();

  foreach (const QLineEdit *line, findChildren<QLineEdit *>()) {
    connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);
  }

  showMaximized();
  ui->tableVenda->resizeColumnsToContents();
}

Venda::~Venda() { delete ui; }

void Venda::setupTables() {
  modelItem.setTable("Venda_has_Produto");
  modelItem.setHeaderData(modelItem.fieldIndex("selecionado"), Qt::Horizontal, "");
  modelItem.setHeaderData(modelItem.fieldIndex("fornecedor"), Qt::Horizontal, "Fornecedor");
  modelItem.setHeaderData(modelItem.fieldIndex("produto"), Qt::Horizontal, "Produto");
  modelItem.setHeaderData(modelItem.fieldIndex("obs"), Qt::Horizontal, "Obs.");
  modelItem.setHeaderData(modelItem.fieldIndex("prcUnitario"), Qt::Horizontal, "Preço/Un");
  modelItem.setHeaderData(modelItem.fieldIndex("caixas"), Qt::Horizontal, "Caixas");
  modelItem.setHeaderData(modelItem.fieldIndex("qte"), Qt::Horizontal, "Qte.");
  modelItem.setHeaderData(modelItem.fieldIndex("un"), Qt::Horizontal, "Un.");
  modelItem.setHeaderData(modelItem.fieldIndex("unCaixa"), Qt::Horizontal, "Un/Caixa");
  modelItem.setHeaderData(modelItem.fieldIndex("parcial"), Qt::Horizontal, "Parcial");
  modelItem.setHeaderData(modelItem.fieldIndex("desconto"), Qt::Horizontal, "Desconto");
  modelItem.setHeaderData(modelItem.fieldIndex("parcialDesc"), Qt::Horizontal, "Desc. Parc.");
  modelItem.setHeaderData(modelItem.fieldIndex("descGlobal"), Qt::Horizontal, "Desc. Glob.");
  modelItem.setHeaderData(modelItem.fieldIndex("total"), Qt::Horizontal, "Total");
  modelItem.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelItem.select()) {
    qDebug() << "erro modelItem: " << modelItem.lastError();
    return;
  }

  modelFluxoCaixa.setTable("Venda_has_Pagamento");
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

  DoubleDelegate *doubleDelegate = new DoubleDelegate(this);
  ui->tableFluxoCaixa->setItemDelegate(doubleDelegate);
  ui->tableVenda->setItemDelegate(doubleDelegate);

  ui->tableVenda->setItemDelegateForColumn(modelItem.fieldIndex("selecionado"), new CheckBoxDelegate(ui->tableVenda));
}

void Venda::resetarPagamentos() {
  ui->doubleSpinBoxTotalPag->setValue(ui->doubleSpinBoxFinal->value());
  ui->doubleSpinBoxPgt1->setMaximum(ui->doubleSpinBoxFinal->value());
  ui->doubleSpinBoxPgt1->setValue(ui->doubleSpinBoxFinal->value());
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

  // TODO: properly disable/enable fields on payments edit
  //  ui->comboBoxPgt2->setDisabled(true);
  //  ui->comboBoxPgt3->setDisabled(true);
  //  ui->comboBoxPgt1Parc->setDisabled(true);
  //  ui->comboBoxPgt2Parc->setDisabled(true);
  //  ui->comboBoxPgt3Parc->setDisabled(true);
  //  ui->dateEditPgt1->setDisabled(true);
  //  ui->dateEditPgt2->setDisabled(true);
  //  ui->dateEditPgt3->setDisabled(true);

  montarFluxoCaixa();
}

void Venda::fecharOrcamento(const QString &idOrcamento) {
  const int row = model.rowCount();
  model.insertRow(row);
  mapper.toLast();

  idVenda = idOrcamento;

  QSqlQuery produtos;
  produtos.prepare("SELECT * FROM Orcamento_has_Produto WHERE idOrcamento = :idOrcamento");
  produtos.bindValue(":idOrcamento", idOrcamento);

  if (not produtos.exec()) {
    qDebug() << "Erro buscando produtos: " << produtos.lastError();
  }

  modelItem.setFilter("idVenda = '" + idOrcamento + "'");

  while (produtos.next()) {
    const int row = modelItem.rowCount();
    modelItem.insertRow(row);

    for (int field = 0, fieldCount = produtos.record().count(); field < fieldCount; ++field) {
      if (not modelItem.setData(modelItem.index(row, field), produtos.value(field))) {
        qDebug() << "Erro setando itens venda: " << modelItem.lastError();
      }
    }

    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("status")), "PENDENTE");
  }

  ui->tableVenda->resizeColumnsToContents();

  QSqlQuery query;
  query.prepare("SELECT * FROM Orcamento WHERE idOrcamento = :idOrcamento");
  query.bindValue(":idOrcamento", idOrcamento);

  if (not query.exec() or not query.first()) {
    qDebug() << "Erro buscando orcamento: " << query.lastError();
  }

  ui->itemBoxEndereco->searchDialog()->setFilter("idCliente = " + query.value("idCliente").toString() +
                                                 " AND desativado = FALSE");
  ui->itemBoxEnderecoFat->searchDialog()->setFilter("idCliente = " + query.value("idCliente").toString() +
                                                    " AND desativado = FALSE");

  ui->itemBoxEndereco->setValue(query.value("idEnderecoEntrega"));
  ui->itemBoxEnderecoFat->setValue(query.value("idEnderecoEntrega"));

  for (int field = 0, columnCount = model.columnCount(); field < columnCount; ++field) {
    if (not model.setData(model.index(mapper.currentIndex(), field), query.value(field))) {
      qDebug() << "Erro setando dados venda: " << model.lastError();
    }
  }

  fillTotals();
  resetarPagamentos();

  modelFluxoCaixa.setFilter("idVenda = '" + idOrcamento + "'");
}

bool Venda::verifyFields(const int row) {
  Q_UNUSED(row);

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

  if (not ui->itemBoxEnderecoFat->value().isValid()) {
    QMessageBox::warning(this, "Aviso!", "Deve selecionar um endereço de faturamento.");
    return false;
  }

  return true;
}

bool Venda::verifyRequiredField(QLineEdit *line) {
  line->objectName();

  return true;
}

QString Venda::requiredStyle() { return QString(); }

void Venda::calcPrecoGlobalTotal(const bool ajusteTotal) {
  double subTotal = 0.0;
  double subTotalItens = 0.0;
  double subTotalBruto = 0.0;
  double minimoFrete = 0, porcFrete = 0;

  QSqlQuery queryFrete;
  queryFrete.prepare("SELECT * FROM Loja WHERE idLoja = :idLoja");
  queryFrete.bindValue(":idLoja", UserSession::getLoja());

  if (not queryFrete.exec() or not queryFrete.first()) {
    qDebug() << "Erro buscando parâmetros do frete: " << queryFrete.lastError();
  } else {
    minimoFrete = queryFrete.value("valorMinimoFrete").toDouble();
    porcFrete = queryFrete.value("porcentagemFrete").toDouble();
  }

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    double prcUnItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("prcUnitario"))).toDouble();
    double qteItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("qte"))).toDouble();
    double descItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("desconto"))).toDouble() / 100.0;
    double itemBruto = qteItem * prcUnItem;
    subTotalBruto += itemBruto;
    double stItem = itemBruto * (1.0 - descItem);
    subTotalItens += stItem;
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("parcial")), itemBruto);
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("parcialDesc")), stItem);
  }

  double frete = qMax(subTotalBruto * porcFrete / 100.0, minimoFrete);

  if (ui->checkBoxFreteManual->isChecked()) {
    frete = ui->doubleSpinBoxFrete->value();
    qDebug() << "novo frete: " << frete;
  }

  double descGlobal = ui->doubleSpinBoxDescontoGlobal->value() / 100.0;
  subTotal = subTotalItens * (1.0 - descGlobal);

  if (ajusteTotal) {
    const double Final = ui->doubleSpinBoxFinal->value();
    subTotal = Final - frete;

    if (subTotalItens == 0.0) {
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

  ui->doubleSpinBoxSubTotalBruto->setValue(subTotalBruto);
  ui->doubleSpinBoxDescontoGlobal->setValue(descGlobal * 100);
  ui->doubleSpinBoxDescontoRS->setValue(subTotalItens - subTotal);
  ui->doubleSpinBoxFrete->setValue(frete);
  ui->doubleSpinBoxTotal->setValue(subTotalItens);
  ui->doubleSpinBoxFinal->setValue(subTotal + frete);

  resetarPagamentos();
  montarFluxoCaixa();
}

void Venda::fillTotals() {
  QSqlQuery query;
  query.prepare("SELECT * FROM Orcamento WHERE idOrcamento = :idOrcamento");
  query.bindValue(":idOrcamento", idVenda);

  if (not query.exec()) {
    qDebug() << "Erro buscando orçamento: " << query.lastError();
    qDebug() << "query: " << query.lastQuery();
  }

  if (not query.first()) {
    query.prepare("SELECT * FROM Venda WHERE idVenda = :idVenda");
    query.bindValue(":idVenda", idVenda);

    if (not query.exec() or not query.first()) {
      qDebug() << "Erro buscando venda: " << query.lastError();
      qDebug() << "Não achou venda: " << query.size();
      qDebug() << "query: " << query.lastQuery();
    }
  }

  ui->doubleSpinBoxSubTotalBruto->setValue(query.value("subTotalBru").toDouble());
  ui->doubleSpinBoxTotal->setValue(query.value("subTotalLiq").toDouble());
  ui->doubleSpinBoxFrete->setValue(query.value("frete").toDouble());
  ui->doubleSpinBoxDescontoGlobal->setValue(query.value("descontoPorc").toDouble());
  ui->doubleSpinBoxDescontoRS->setValue(query.value("descontoReais").toDouble());
  ui->doubleSpinBoxFinal->setValue(query.value("total").toDouble());
}

void Venda::clearFields() { idVenda = QString(); }

void Venda::setupMapper() {
  addMapping(ui->itemBoxEndereco, "idEnderecoEntrega", "value");
  addMapping(ui->itemBoxEnderecoFat, "idEnderecoFaturamento", "value");
  addMapping(ui->doubleSpinBoxSubTotalBruto, "subTotalBru");
  addMapping(ui->doubleSpinBoxTotal, "subTotalLiq");
  addMapping(ui->doubleSpinBoxFrete, "frete");
  addMapping(ui->doubleSpinBoxDescontoGlobal, "descontoPorc");
  addMapping(ui->doubleSpinBoxDescontoRS, "descontoReais");
  addMapping(ui->doubleSpinBoxFinal, "total");
}

void Venda::on_pushButtonCancelar_clicked() { close(); }

void Venda::on_pushButtonCadastrarPedido_clicked() { update(); }

void Venda::on_pushButtonNFe_clicked() {
  CadastrarNFe *cadNFe = new CadastrarNFe(idVenda, this);

  const QModelIndexList list = modelItem.match(modelItem.index(0, 0), Qt::DisplayRole, true, -1);

  if (list.size() == 0) {
    QMessageBox::warning(this, "Aviso!", "Nenhum item selecionado!");
    return;
  }

  QList<int> lista;

  foreach (const QModelIndex index, list) { lista.append(index.row()); }

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

  if (pgt2 == 0.0 or pgt3 >= 0.0) {
    ui->doubleSpinBoxPgt2->setMaximum(restante + pgt2);
    ui->doubleSpinBoxPgt2->setValue(restante + pgt2);
    ui->doubleSpinBoxPgt3->setMaximum(pgt3);
    restante = 0;
  } else if (pgt3 == 0.0) {
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

  query.prepare("SELECT Produto.descricao, Produto.estoque, Venda_has_Produto.idVenda FROM Venda_has_Produto INNER "
                "JOIN Produto ON Produto.idProduto = Venda_has_Produto.idProduto WHERE estoque = 0 AND "
                "Venda_has_Produto.idVenda = :idVenda");
  query.bindValue(":idVenda", idVenda);

  if (not query.exec()) {
    qDebug() << "Erro na verificação de estoque: " << query.lastError();
    return false;
  }

  if (query.size() > 0) {
    query.prepare("INSERT INTO PedidoFornecedor (idPedido, idLoja, idUsuario, idCliente, "
                  "idEnderecoEntrega, idProfissional, data, subTotalBru, subTotalLiq, frete, descontoPorc, "
                  "descontoReais, total, validade, status) SELECT idVenda, idLoja, idUsuario, idCliente, "
                  "idEnderecoEntrega, idProfissional, data, subTotalBru, subTotalLiq, frete, descontoPorc, "
                  "descontoReais, total, validade, status "
                  "FROM Venda WHERE idVenda = :idVenda");
    query.bindValue(":idVenda", idVenda);

    if (not query.exec()) {
      qDebug() << "Erro na criação do pedido fornecedor: " << query.lastError();
    }

    query.prepare("INSERT INTO PedidoFornecedor_has_Produto SELECT Venda_has_Produto.* FROM "
                  "Venda_has_Produto INNER JOIN Produto ON Produto.idProduto = "
                  "Venda_has_Produto.idProduto WHERE estoque = 0 AND Venda_has_Produto.idVenda = :idVenda");
    query.bindValue(":idVenda", idVenda);

    if (not query.exec()) {
      qDebug() << "Erro na inserção de produtos em PedidoFornecedor_has_Produto: " << query.lastError();
      return false;
    }
  }

  query.prepare("DELETE FROM Orcamento_has_Produto WHERE idOrcamento = :idVenda");
  query.bindValue(":idVenda", idVenda);

  if (not query.exec()) {
    qDebug() << "Erro deletando itens no Orcamento_has_Produto: " << query.lastError();
    return false;
  }

  query.prepare("DELETE FROM Orcamento WHERE idOrcamento = :idVenda");
  query.bindValue(":idVenda", idVenda);

  if (not query.exec()) {
    qDebug() << "Erro deletando Orcamento: " << query.lastError();
    return false;
  }

  query.prepare("INSERT INTO ContaAReceber (dataEmissao, idVenda) VALUES (:dataEmissao, :idVenda)");
  query.bindValue(":dataEmissao", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
  query.bindValue(":idVenda", idVenda);

  if (not query.exec()) {
    qDebug() << "Erro inserindo ContaAReceber: " << query.lastError();
    return false;
  }

  return true;
}

void Venda::registerMode() {
  ui->framePagamentos->show();
  ui->pushButtonNFe->hide();
  ui->pushButtonCadastrarPedido->show();
  ui->pushButtonVoltar->show();
  ui->doubleSpinBoxDescontoGlobal->setReadOnly(false);
  ui->doubleSpinBoxDescontoGlobal->setFrame(true);
  ui->doubleSpinBoxDescontoGlobal->setButtonSymbols(QDoubleSpinBox::UpDownArrows);
  ui->doubleSpinBoxFinal->setReadOnly(false);
  ui->doubleSpinBoxFinal->setFrame(true);
  ui->doubleSpinBoxFinal->setButtonSymbols(QDoubleSpinBox::UpDownArrows);
  ui->checkBoxFreteManual->show();
}

void Venda::updateMode() {
  ui->framePagamentos_2->hide();
  ui->pushButtonNFe->show();
  ui->pushButtonCadastrarPedido->hide();
  ui->pushButtonVoltar->hide();
  ui->doubleSpinBoxDescontoGlobal->setReadOnly(true);
  ui->doubleSpinBoxDescontoGlobal->setFrame(false);
  ui->doubleSpinBoxDescontoGlobal->setButtonSymbols(QDoubleSpinBox::NoButtons);
  ui->doubleSpinBoxFinal->setReadOnly(true);
  ui->doubleSpinBoxFinal->setFrame(false);
  ui->doubleSpinBoxFinal->setButtonSymbols(QDoubleSpinBox::NoButtons);
  ui->checkBoxFreteManual->hide();
}

bool Venda::viewRegister(const QModelIndex index) {
  if (not RegisterDialog::viewRegister(index)) {
    return false;
  }

  idVenda = data(primaryKey).toString();

  modelItem.setFilter("idVenda = '" + idVenda + "'");

  if (not modelItem.select()) {
    qDebug() << "erro modelItem: " << modelItem.lastError();
    return false;
  }

  modelFluxoCaixa.setFilter("idVenda = '" + idVenda + "'");

  if (not modelFluxoCaixa.select()) {
    qDebug() << "erro modelFluxoCaixa: " << modelFluxoCaixa.lastError();
    return false;
  }

  ui->tableFluxoCaixa->resizeColumnsToContents();

  for (int i = 0; i < modelItem.rowCount(); ++i) {
    ui->tableVenda->openPersistentEditor(modelItem.index(i, 0));
  }

  fillTotals();

  return true;
}

void Venda::on_pushButtonVoltar_clicked() {
  Orcamento *orcamento = new Orcamento(parentWidget());
  orcamento->viewRegisterById(idVenda);
  orcamento->show();
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
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("idVenda")), idVenda);
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
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("idVenda")), idVenda);
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
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("idVenda")), idVenda);
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

void Venda::on_doubleSpinBoxFinal_editingFinished() {
  if (modelItem.rowCount() == 0 or ui->doubleSpinBoxTotal->value() == 0) {
    calcPrecoGlobalTotal();
    return;
  }

  const double new_total = ui->doubleSpinBoxFinal->value();
  const double frete = ui->doubleSpinBoxFrete->value();
  const double new_subtotal = new_total - frete;

  if (new_subtotal >= ui->doubleSpinBoxTotal->value()) {
    ui->doubleSpinBoxDescontoGlobal->setValue(0.0);
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
  report->loadReport("C:/ERP/venda.xml");
  report->recordCount << ui->tableVenda->model()->rowCount();
  QObject::connect(report, &QtRPT::setValue, this, &Venda::setValue);
  report->printExec();
}

void Venda::setValue(const int recNo, const QString paramName, QVariant &paramValue, const int reportPage) {
  Q_UNUSED(reportPage);
  QLocale locale;

  QSqlQuery queryCliente;
  queryCliente.prepare("SELECT * FROM Cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", model.data(model.index(0, model.fieldIndex("idCliente"))));

  if (not queryCliente.exec() or not queryCliente.first()) {
    qDebug() << "Erro buscando cliente: " << model.fieldIndex("idCliente") << " - " << model.lastError();
  }

  QSqlQuery queryProduto;
  queryProduto.prepare("SELECT * FROM Produto WHERE idProduto = :idProduto");
  queryProduto.bindValue(":idProduto", modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("idProduto"))));

  if (not queryProduto.exec() or not queryProduto.first()) {
    qDebug() << "Erro buscando produto: " << modelItem.fieldIndex("idProduto") << " - " << modelItem.lastError();
  }

  if (paramName == "cliente") {
    paramValue = queryCliente.value("nome_razao").toString();
  }

  if (paramName == "data") {
    paramValue = model.data(model.index(0, model.fieldIndex("data"))).toDateTime().toString("hh:mm dd-MM-yyyy");
  }

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
    paramValue = locale.toString(value, 'f', 2);
  }

  if (paramName == "Quant.") {
    paramValue = modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("qte"))).toString();
  }

  if (paramName == "Unid.") {
    paramValue = modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("un"))).toString();
  }

  if (paramName == "Total") {
    double value = modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("total"))).toDouble();
    paramValue = locale.toString(value, 'f', 2);
  }
}

void Venda::successMessage() {
  QMessageBox::information(this, "Atenção!", "Venda cadastrada com sucesso!", QMessageBox::Ok, QMessageBox::NoButton);

  close();
}
