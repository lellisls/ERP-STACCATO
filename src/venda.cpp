#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlRecord>

#include "cadastrocliente.h"
#include "checkboxdelegate.h"
#include "devolucao.h"
#include "doubledelegate.h"
#include "excel.h"
#include "impressao.h"
#include "logindialog.h"
#include "noeditdelegate.h"
#include "orcamento.h"
#include "porcentagemdelegate.h"
#include "reaisdelegate.h"
#include "singleeditdelegate.h"
#include "ui_venda.h"
#include "usersession.h"
#include "venda.h"

Venda::Venda(QWidget *parent) : RegisterDialog("venda", "idVenda", parent), ui(new Ui::Venda) {
  ui->setupUi(this);

  setAttribute(Qt::WA_DeleteOnClose);

  //  for (auto const *line : findChildren<QLineEdit *>()) {
  //    connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);
  //  }

  setupTables();

  ui->itemBoxCliente->setSearchDialog(SearchDialog::cliente(this));
  ui->itemBoxCliente->setRegisterDialog(new CadastroCliente(this));
  ui->itemBoxVendedor->setSearchDialog(SearchDialog::vendedor(this));
  ui->itemBoxProfissional->setSearchDialog(SearchDialog::profissional(this));
  ui->itemBoxEndereco->setSearchDialog(SearchDialog::enderecoCliente(this));
  ui->itemBoxEnderecoFat->setSearchDialog(SearchDialog::enderecoCliente(this));

  ui->dateEditPgt1->setDate(QDate::currentDate());
  ui->dateEditPgt2->setDate(QDate::currentDate());
  ui->dateEditPgt3->setDate(QDate::currentDate());
  ui->dateEditPgt4->setDate(QDate::currentDate());
  ui->dateEditPgt5->setDate(QDate::currentDate());

  setupMapper();
  newRegister();
  makeConnections();

  ui->groupBoxFinanceiro->hide();
  ui->tableFluxoCaixa2->hide();

  show();
}

Venda::~Venda() { delete ui; }

void Venda::makeConnections() {
  connect(ui->checkBoxRep1, &QCheckBox::stateChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->checkBoxRep2, &QCheckBox::stateChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->checkBoxRep3, &QCheckBox::stateChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->checkBoxRep4, &QCheckBox::stateChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->checkBoxRep5, &QCheckBox::stateChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->comboBoxPgt1Parc, &QComboBox::currentTextChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->comboBoxPgt2Parc, &QComboBox::currentTextChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->comboBoxPgt3Parc, &QComboBox::currentTextChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->comboBoxPgt4Parc, &QComboBox::currentTextChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->comboBoxPgt5Parc, &QComboBox::currentTextChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->dateEditPgt1, &QDateEdit::dateChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->dateEditPgt2, &QDateEdit::dateChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->dateEditPgt3, &QDateEdit::dateChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->dateEditPgt4, &QDateEdit::dateChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->dateEditPgt5, &QDateEdit::dateChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->lineEditPgt1, &QLineEdit::textChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->lineEditPgt2, &QLineEdit::textChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->lineEditPgt3, &QLineEdit::textChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->lineEditPgt4, &QLineEdit::textChanged, this, &Venda::montarFluxoCaixa);
  connect(ui->lineEditPgt5, &QLineEdit::textChanged, this, &Venda::montarFluxoCaixa);
}

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
  modelItem.setHeaderData("unCaixa", "Un./Cx.");
  modelItem.setHeaderData("codComercial", "Código");
  modelItem.setHeaderData("formComercial", "Formato");
  modelItem.setHeaderData("parcial", "Subtotal");
  modelItem.setHeaderData("desconto", "Desc. %");
  modelItem.setHeaderData("parcialDesc", "Desc. Parc.");
  modelItem.setHeaderData("descGlobal", "Desc. Glob. %");
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

  modelItem.setFilter("0");

  if (not modelItem.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela venda_has_produto: " + modelItem.lastError().text());
    return;
  }

  ui->tableVenda->setModel(&modelItem);
  ui->tableVenda->hideColumn("entregou");
  ui->tableVenda->hideColumn("descUnitario");
  ui->tableVenda->hideColumn("estoque_promocao");
  ui->tableVenda->hideColumn("idCompra");
  ui->tableVenda->hideColumn("idNfeSaida");
  ui->tableVenda->hideColumn("idVendaProduto");
  ui->tableVenda->hideColumn("selecionado");
  ui->tableVenda->hideColumn("idVenda");
  ui->tableVenda->hideColumn("idLoja");
  ui->tableVenda->hideColumn("idProduto");
  ui->tableVenda->hideColumn("comissao");
  ui->tableVenda->setItemDelegate(new DoubleDelegate(this));
  ui->tableVenda->setItemDelegateForColumn("quant", new DoubleDelegate(this, 4));
  ui->tableVenda->setItemDelegateForColumn("prcUnitario", new ReaisDelegate(this));
  ui->tableVenda->setItemDelegateForColumn("parcial", new ReaisDelegate(this));
  ui->tableVenda->setItemDelegateForColumn("parcial", new ReaisDelegate(this));
  ui->tableVenda->setItemDelegateForColumn("parcialDesc", new ReaisDelegate(this));
  ui->tableVenda->setItemDelegateForColumn("desconto", new PorcentagemDelegate(this));
  ui->tableVenda->setItemDelegateForColumn("descGlobal", new PorcentagemDelegate(this));
  ui->tableVenda->setItemDelegateForColumn("total", new ReaisDelegate(this));

  modelFluxoCaixa.setTable("conta_a_receber_has_pagamento");
  modelFluxoCaixa.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelFluxoCaixa.setHeaderData("tipo", "Tipo");
  modelFluxoCaixa.setHeaderData("parcela", "Parcela");
  modelFluxoCaixa.setHeaderData("valor", "R$");
  modelFluxoCaixa.setHeaderData("dataPagamento", "Data");
  modelFluxoCaixa.setHeaderData("observacao", "Obs.");
  modelFluxoCaixa.setHeaderData("status", "Status");
  modelFluxoCaixa.setHeaderData("representacao", "Representação");

  modelFluxoCaixa.setFilter("0");

  if (not modelFluxoCaixa.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela conta_a_receber_has_pagamento: " + modelFluxoCaixa.lastError().text());
    return;
  }

  ui->tableFluxoCaixa->setModel(&modelFluxoCaixa);
  ui->tableFluxoCaixa->hideColumn("contraParte");
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
  ui->tableFluxoCaixa->hideColumn("comissao");
  ui->tableFluxoCaixa->hideColumn("taxa");
  ui->tableFluxoCaixa->setItemDelegate(new NoEditDelegate(this));
  ui->tableFluxoCaixa->setItemDelegateForColumn("representacao", new CheckBoxDelegate(this, true));
  ui->tableFluxoCaixa->setItemDelegateForColumn("observacao", new SingleEditDelegate(this));
  ui->tableFluxoCaixa->setItemDelegateForColumn("valor", new ReaisDelegate(this));

  modelFluxoCaixa2.setTable("conta_a_receber_has_pagamento");
  modelFluxoCaixa2.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelFluxoCaixa2.setHeaderData("contraParte", "ContraParte");
  modelFluxoCaixa2.setHeaderData("tipo", "Tipo");
  modelFluxoCaixa2.setHeaderData("parcela", "Parcela");
  modelFluxoCaixa2.setHeaderData("valor", "R$");
  modelFluxoCaixa2.setHeaderData("dataPagamento", "Data");
  modelFluxoCaixa2.setHeaderData("observacao", "Obs.");
  modelFluxoCaixa2.setHeaderData("status", "Status");

  modelFluxoCaixa2.setFilter("0");

  if (not modelFluxoCaixa2.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela conta_a_receber_has_pagamento: " + modelFluxoCaixa2.lastError().text());
    return;
  }

  ui->tableFluxoCaixa2->setModel(&modelFluxoCaixa2);
  ui->tableFluxoCaixa2->hideColumn("idPagamento");
  ui->tableFluxoCaixa2->hideColumn("dataEmissao");
  ui->tableFluxoCaixa2->hideColumn("idVenda");
  ui->tableFluxoCaixa2->hideColumn("idLoja");
  ui->tableFluxoCaixa2->hideColumn("representacao");
  ui->tableFluxoCaixa2->hideColumn("dataRealizado");
  ui->tableFluxoCaixa2->hideColumn("valorReal");
  ui->tableFluxoCaixa2->hideColumn("tipoReal");
  ui->tableFluxoCaixa2->hideColumn("parcelaReal");
  ui->tableFluxoCaixa2->hideColumn("contaDestino");
  ui->tableFluxoCaixa2->hideColumn("tipoDet");
  ui->tableFluxoCaixa2->hideColumn("centroCusto");
  ui->tableFluxoCaixa2->hideColumn("grupo");
  ui->tableFluxoCaixa2->hideColumn("subGrupo");
  ui->tableFluxoCaixa2->hideColumn("comissao");
  ui->tableFluxoCaixa2->hideColumn("taxa");
  ui->tableFluxoCaixa2->setItemDelegateForColumn("valor", new ReaisDelegate(this));
}

void Venda::resetarPagamentos() {
  ui->doubleSpinBoxTotalPag->setValue(ui->doubleSpinBoxTotal->value());
  ui->doubleSpinBoxPgt1->setMaximum(ui->doubleSpinBoxTotal->value());
  ui->doubleSpinBoxPgt1->setValue(ui->doubleSpinBoxTotal->value());
  ui->doubleSpinBoxPgt2->setValue(0);
  ui->doubleSpinBoxPgt3->setValue(0);
  ui->doubleSpinBoxPgt4->setValue(0);
  ui->doubleSpinBoxPgt5->setValue(0);
  ui->doubleSpinBoxPgt2->setMaximum(0);
  ui->doubleSpinBoxPgt3->setMaximum(0);
  ui->doubleSpinBoxPgt4->setMaximum(0);
  ui->doubleSpinBoxPgt5->setMaximum(0);
  ui->comboBoxPgt1->setCurrentIndex(0);
  ui->comboBoxPgt2->setCurrentIndex(0);
  ui->comboBoxPgt3->setCurrentIndex(0);
  ui->comboBoxPgt4->setCurrentIndex(0);
  ui->comboBoxPgt5->setCurrentIndex(0);

  ui->comboBoxPgt1Parc->clear();
  ui->comboBoxPgt2Parc->clear();
  ui->comboBoxPgt3Parc->clear();
  ui->comboBoxPgt4Parc->clear();
  ui->comboBoxPgt5Parc->clear();

  ui->comboBoxPgt1Parc->setCurrentIndex(0);
  ui->comboBoxPgt2Parc->setCurrentIndex(0);
  ui->comboBoxPgt3Parc->setCurrentIndex(0);
  ui->comboBoxPgt4Parc->setCurrentIndex(0);
  ui->comboBoxPgt5Parc->setCurrentIndex(0);

  ui->comboBoxPgt2->setDisabled(true);
  ui->comboBoxPgt3->setDisabled(true);
  ui->comboBoxPgt4->setDisabled(true);
  ui->comboBoxPgt5->setDisabled(true);
  ui->comboBoxPgt1Parc->setDisabled(true);
  ui->comboBoxPgt2Parc->setDisabled(true);
  ui->comboBoxPgt3Parc->setDisabled(true);
  ui->comboBoxPgt4Parc->setDisabled(true);
  ui->comboBoxPgt5Parc->setDisabled(true);
  ui->dateEditPgt2->setDisabled(true);
  ui->dateEditPgt3->setDisabled(true);
  ui->dateEditPgt4->setDisabled(true);
  ui->dateEditPgt5->setDisabled(true);

  ui->doubleSpinBoxPgt2->setDisabled(true);
  ui->doubleSpinBoxPgt3->setDisabled(true);
  ui->doubleSpinBoxPgt4->setDisabled(true);
  ui->doubleSpinBoxPgt5->setDisabled(true);

  ui->dateEditPgt1->setDate(QDate::currentDate());
  ui->dateEditPgt2->setDate(QDate::currentDate());
  ui->dateEditPgt3->setDate(QDate::currentDate());
  ui->dateEditPgt4->setDate(QDate::currentDate());
  ui->dateEditPgt5->setDate(QDate::currentDate());

  ui->lineEditPgt1->clear();
  ui->lineEditPgt2->clear();
  ui->lineEditPgt3->clear();
  ui->lineEditPgt4->clear();
  ui->lineEditPgt5->clear();

  ui->lineEditPgt1->setReadOnly(false);
  ui->lineEditPgt2->setReadOnly(false);
  ui->lineEditPgt3->setReadOnly(false);
  ui->lineEditPgt4->setReadOnly(false);
  ui->lineEditPgt5->setReadOnly(false);

  ui->doubleSpinBoxPgt1->setReadOnly(false);
  ui->doubleSpinBoxPgt2->setReadOnly(false);
  ui->doubleSpinBoxPgt3->setReadOnly(false);
  ui->doubleSpinBoxPgt4->setReadOnly(false);
  ui->doubleSpinBoxPgt5->setReadOnly(false);

  ui->checkBoxRep1->setChecked(data("representacao").toBool());
  ui->checkBoxRep2->setChecked(data("representacao").toBool());
  ui->checkBoxRep3->setChecked(data("representacao").toBool());
  ui->checkBoxRep4->setChecked(data("representacao").toBool());
  ui->checkBoxRep5->setChecked(data("representacao").toBool());

  montarFluxoCaixa();
}

void Venda::prepararVenda(const QString &idOrcamento) {
  m_idOrcamento = idOrcamento;
  isUpdate = true;

  QSqlQuery queryOrc;
  queryOrc.prepare("SELECT idUsuario, idOrcamento, idLoja, idUsuarioIndicou, idCliente, idEnderecoEntrega, "
                   "idEnderecoFaturamento, idProfissional, data, subTotalBru, subTotalLiq, frete, descontoPorc, "
                   "descontoReais, total, status, observacao, prazoEntrega, representacao FROM orcamento WHERE "
                   "idOrcamento = :idOrcamento");
  queryOrc.bindValue(":idOrcamento", idOrcamento);

  if (not queryOrc.exec() or not queryOrc.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando orçamento: " + queryOrc.lastError().text());
    return;
  }

  //

  QSqlQuery queryPag;
  queryPag.prepare("SELECT pagamento FROM view_pagamento_loja WHERE idLoja = :idLoja");
  queryPag.bindValue(":idLoja", queryOrc.value("idLoja"));

  if (not queryPag.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo formas de pagamentos: " + queryPag.lastError().text());
    return;
  }

  QStringList list{"Escolha uma opção!"};

  while (queryPag.next()) list << queryPag.value("pagamento").toString();

  ui->comboBoxPgt1->insertItems(0, list);
  ui->comboBoxPgt2->insertItems(0, list);
  ui->comboBoxPgt3->insertItems(0, list);
  ui->comboBoxPgt4->insertItems(0, list);
  ui->comboBoxPgt5->insertItems(0, list);

  ui->comboBoxPgt1->addItem("Conta Cliente");

  //

  ui->itemBoxVendedor->setValue(queryOrc.value("idUsuario"));

  QSqlQuery queryProdutos;
  queryProdutos.prepare("SELECT * FROM orcamento_has_produto WHERE idOrcamento = :idOrcamento");
  queryProdutos.bindValue(":idOrcamento", idOrcamento);

  if (not queryProdutos.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando produtos: " + queryProdutos.lastError().text());
    return;
  }

  while (queryProdutos.next()) {
    const int rowItem = modelItem.rowCount();
    modelItem.insertRow(rowItem);

    for (int column = 0, columnCount = queryProdutos.record().count(); column < columnCount; ++column) {
      const QString field = queryProdutos.record().fieldName(column);

      if (modelItem.fieldIndex(field) == -1) continue;
      if (not modelItem.setData(rowItem, field, queryProdutos.value(field))) return;
    }

    if (not modelItem.setData(rowItem, "status", "PENDENTE")) return;
  }

  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

  const int row = model.rowCount();
  model.insertRow(row);
  mapper.toLast();

  if (not model.setData(row, "idOrcamento", queryOrc.value("idOrcamento"))) return;
  if (not model.setData(row, "idLoja", queryOrc.value("idLoja"))) return;
  if (not model.setData(row, "idUsuario", queryOrc.value("idUsuario"))) return;
  if (not model.setData(row, "idUsuarioIndicou", queryOrc.value("idUsuarioIndicou"))) return;
  if (not model.setData(row, "idCliente", queryOrc.value("idCliente"))) return;
  if (not model.setData(row, "idEnderecoEntrega", queryOrc.value("idEnderecoEntrega"))) return;
  if (not model.setData(row, "idProfissional", queryOrc.value("idProfissional"))) return;
  if (not model.setData(row, "dataOrc", queryOrc.value("data"))) return;
  if (not model.setData(row, "subTotalBru", queryOrc.value("subTotalBru"))) return;
  if (not model.setData(row, "subTotalLiq", queryOrc.value("subTotalLiq"))) return;
  if (not model.setData(row, "frete", queryOrc.value("frete"))) return;
  if (not model.setData(row, "descontoPorc", queryOrc.value("descontoPorc"))) return;
  if (not model.setData(row, "descontoReais", queryOrc.value("descontoReais"))) return;
  if (not model.setData(row, "total", queryOrc.value("total"))) return;
  if (not model.setData(row, "status", queryOrc.value("status"))) return;
  if (not model.setData(row, "observacao", queryOrc.value("observacao"))) return;
  if (not model.setData(row, "prazoEntrega", queryOrc.value("prazoEntrega"))) return;
  if (not model.setData(row, "representacao", queryOrc.value("representacao"))) return;

  ui->itemBoxEndereco->searchDialog()->setFilter("idCliente = " + queryOrc.value("idCliente").toString() +
                                                 " AND desativado = FALSE");

  ui->itemBoxEnderecoFat->searchDialog()->setFilter("idCliente = " + queryOrc.value("idCliente").toString() +
                                                    " AND desativado = FALSE");

  ui->itemBoxCliente->setValue(queryOrc.value("idCliente"));
  ui->itemBoxProfissional->setValue(queryOrc.value("idProfissional"));
  ui->itemBoxEndereco->setValue(queryOrc.value("idEnderecoEntrega"));

  ui->tableVenda->resizeColumnsToContents();

  QSqlQuery queryFrete;
  queryFrete.prepare("SELECT valorMinimoFrete, porcentagemFrete FROM loja WHERE idLoja = :idLoja");
  queryFrete.bindValue(":idLoja", UserSession::fromLoja("usuario.idLoja", ui->itemBoxVendedor->text()));

  if (not queryFrete.exec() or not queryFrete.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando parâmetros do frete: " + queryFrete.lastError().text());
    return;
  }

  minimoFrete = queryFrete.value("valorMinimoFrete").toDouble();
  porcFrete = queryFrete.value("porcentagemFrete").toDouble();

  if (data("representacao").toBool()) {
    ui->checkBoxRep1->setChecked(true);
    ui->checkBoxRep2->setChecked(true);
    ui->checkBoxRep3->setChecked(true);
    ui->checkBoxRep4->setChecked(true);
    ui->checkBoxRep5->setChecked(true);
  } else {
    ui->pushButtonFreteLoja->hide();
    ui->pushButtonPgtLoja->hide();
    ui->checkBoxRep1->hide();
    ui->checkBoxRep2->hide();
    ui->checkBoxRep3->hide();
    ui->checkBoxRep4->hide();
    ui->checkBoxRep5->hide();
    ui->tableFluxoCaixa->hideColumn("representacao");
  }

  QSqlQuery query;
  query.prepare("SELECT credito FROM cliente WHERE idCliente = :idCliente");
  query.bindValue(":idCliente", data("idCliente"));

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando crédito cliente: " + query.lastError().text());
    return;
  }

  ui->doubleSpinBoxCreditoTotal->setValue(query.value("credito").toDouble());

  if (query.value("credito").toDouble() == 0.) {
    ui->comboBoxPgt1->removeItem(ui->comboBoxPgt1->findText("Conta Cliente"));
    ui->comboBoxPgt2->removeItem(ui->comboBoxPgt2->findText("Conta Cliente"));
    ui->comboBoxPgt3->removeItem(ui->comboBoxPgt3->findText("Conta Cliente"));
    ui->comboBoxPgt4->removeItem(ui->comboBoxPgt4->findText("Conta Cliente"));
    ui->comboBoxPgt5->removeItem(ui->comboBoxPgt5->findText("Conta Cliente"));
  }

  resetarPagamentos();

  ui->lineEditVenda->setText("Auto gerado");
}

bool Venda::verifyFields() {
  if (ui->comboBoxPgt1->currentText() != "Escolha uma opção!" and ui->lineEditPgt1->text().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Faltou preencher observação do pagamento 1!");
    ui->lineEditPgt1->setFocus();
    return false;
  }

  if (ui->comboBoxPgt2->currentText() != "Escolha uma opção!" and ui->lineEditPgt2->text().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Faltou preencher observação do pagamento 2!");
    ui->lineEditPgt2->setFocus();
    return false;
  }

  if (ui->comboBoxPgt3->currentText() != "Escolha uma opção!" and ui->lineEditPgt3->text().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Faltou preencher observação do pagamento 3!");
    ui->lineEditPgt3->setFocus();
    return false;
  }

  if (ui->comboBoxPgt4->currentText() != "Escolha uma opção!" and ui->lineEditPgt4->text().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Faltou preencher observação do pagamento 4!");
    ui->lineEditPgt4->setFocus();
    return false;
  }

  if (ui->comboBoxPgt5->currentText() != "Escolha uma opção!" and ui->lineEditPgt5->text().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Faltou preencher observação do pagamento 5!");
    ui->lineEditPgt5->setFocus();
    return false;
  }

  if (ui->spinBoxPrazoEntrega->value() == 0) {
    QMessageBox::critical(this, "Erro!", "Por favor preencha o prazo de entrega.");
    ui->spinBoxPrazoEntrega->setFocus();
    return false;
  }

  if (ui->itemBoxEnderecoFat->text().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Deve selecionar um endereço de faturamento!");
    ui->itemBoxEnderecoFat->setFocus();
    return false;
  }

  if (not qFuzzyCompare(ui->doubleSpinBoxPgt1->value() + ui->doubleSpinBoxPgt2->value() +
                            ui->doubleSpinBoxPgt3->value() + ui->doubleSpinBoxPgt4->value() +
                            ui->doubleSpinBoxPgt5->value(),
                        ui->doubleSpinBoxTotal->value())) {
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

  if (ui->doubleSpinBoxPgt4->value() > 0 and ui->comboBoxPgt4->currentText() == "Escolha uma opção!") {
    QMessageBox::critical(this, "Erro!", "Por favor escolha a forma de pagamento 4.");
    ui->doubleSpinBoxPgt4->setFocus();
    return false;
  }

  if (ui->doubleSpinBoxPgt5->value() > 0 and ui->comboBoxPgt5->currentText() == "Escolha uma opção!") {
    QMessageBox::critical(this, "Erro!", "Por favor escolha a forma de pagamento 5.");
    ui->doubleSpinBoxPgt5->setFocus();
    return false;
  }

  if (ui->doubleSpinBoxPgt1->value() == 0 and ui->comboBoxPgt1->currentText() != "Escolha uma opção!") {
    QMessageBox::critical(this, "Erro!", "Pagamento 1 está com valor 0!");
    ui->doubleSpinBoxPgt1->setFocus();
    return false;
  }

  if (ui->doubleSpinBoxPgt2->value() == 0 and ui->comboBoxPgt2->currentText() != "Escolha uma opção!") {
    QMessageBox::critical(this, "Erro!", "Pagamento 2 está com valor 0!");
    ui->doubleSpinBoxPgt2->setFocus();
    return false;
  }

  if (ui->doubleSpinBoxPgt3->value() == 0 and ui->comboBoxPgt3->currentText() != "Escolha uma opção!") {
    QMessageBox::critical(this, "Erro!", "Pagamento 3 está com valor 0!");
    ui->doubleSpinBoxPgt3->setFocus();
    return false;
  }

  if (ui->doubleSpinBoxPgt4->value() == 0 and ui->comboBoxPgt4->currentText() != "Escolha uma opção!") {
    QMessageBox::critical(this, "Erro!", "Pagamento 4 está com valor 0!");
    ui->doubleSpinBoxPgt4->setFocus();
    return false;
  }

  if (ui->doubleSpinBoxPgt5->value() == 0 and ui->comboBoxPgt5->currentText() != "Escolha uma opção!") {
    QMessageBox::critical(this, "Erro!", "Pagamento 5 está com valor 0!");
    ui->doubleSpinBoxPgt5->setFocus();
    return false;
  }

  return true;
}

void Venda::calcPrecoGlobalTotal() {
  double subTotalItens = 0.;
  double subTotalBruto = 0.;

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    const double itemBruto = modelItem.data(row, "parcial").toDouble();
    const double stItem = modelItem.data(row, "parcialDesc").toDouble();
    subTotalBruto += itemBruto;
    subTotalItens += stItem;
  }

  const double descGlobal = ui->doubleSpinBoxDescontoGlobal->value() / 100.;
  const double subTotal = subTotalItens * (1. - descGlobal);

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    modelItem.setData(row, "descGlobal", descGlobal * 100.);
    modelItem.setData(row, "total", modelItem.data(row, "parcialDesc").toDouble() * (1 - descGlobal));
  }

  ui->doubleSpinBoxSubTotalBruto->setValue(subTotalBruto);
  ui->doubleSpinBoxSubTotalLiq->setValue(subTotalItens);
  ui->doubleSpinBoxSubTotalDesc->setValue(subTotalItens - ui->doubleSpinBoxDescontoGlobalReais->value());
  if (not isBlockedReais) ui->doubleSpinBoxDescontoGlobalReais->setValue(subTotalItens - subTotal);
  if (not isBlockedTotal) ui->doubleSpinBoxTotal->setValue(subTotal + ui->doubleSpinBoxFrete->value());

  resetarPagamentos();
}

void Venda::clearFields() {}

void Venda::setupMapper() {
  addMapping(ui->dateTimeEdit, "data");
  addMapping(ui->dateTimeEditOrc, "dataOrc");
  addMapping(ui->doubleSpinBoxSubTotalBruto, "subTotalBru");
  addMapping(ui->doubleSpinBoxSubTotalLiq, "subTotalLiq");
  addMapping(ui->doubleSpinBoxDescontoGlobal, "descontoPorc");
  addMapping(ui->doubleSpinBoxDescontoGlobalReais, "descontoReais");
  addMapping(ui->doubleSpinBoxFrete, "frete");
  addMapping(ui->doubleSpinBoxTotal, "total");
  addMapping(ui->itemBoxCliente, "idCliente", "value");
  addMapping(ui->itemBoxEndereco, "idEnderecoEntrega", "value");
  addMapping(ui->itemBoxEnderecoFat, "idEnderecoFaturamento", "value");
  addMapping(ui->itemBoxProfissional, "idProfissional", "value");
  addMapping(ui->itemBoxVendedor, "idUsuario", "value");
  addMapping(ui->lineEditIdOrcamento, "idOrcamento");
  addMapping(ui->lineEditVenda, "idVenda");
  addMapping(ui->spinBoxPrazoEntrega, "prazoEntrega");
  addMapping(ui->plainTextEdit, "observacao");
}

void Venda::on_pushButtonCadastrarPedido_clicked() { update(); }

void Venda::calculoSpinBox1() {
  const double pgt1 = ui->doubleSpinBoxPgt1->value();
  const double pgt2 = ui->doubleSpinBoxPgt2->value();
  const double pgt3 = ui->doubleSpinBoxPgt3->value();
  const double pgt4 = ui->doubleSpinBoxPgt4->value();
  const double pgt5 = ui->doubleSpinBoxPgt5->value();
  const double total = ui->doubleSpinBoxTotalPag->value();
  const double restante = total - (pgt1 + pgt2 + pgt3 + pgt4 + pgt5);

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

  montarFluxoCaixa();
}

void Venda::on_doubleSpinBoxPgt1_valueChanged(double) { calculoSpinBox1(); }

void Venda::calculoSpinBox2() {
  const double pgt1 = ui->doubleSpinBoxPgt1->value();
  const double pgt2 = ui->doubleSpinBoxPgt2->value();
  const double pgt3 = ui->doubleSpinBoxPgt3->value();
  const double pgt4 = ui->doubleSpinBoxPgt4->value();
  const double pgt5 = ui->doubleSpinBoxPgt5->value();
  const double total = ui->doubleSpinBoxTotalPag->value();
  const double restante = total - (pgt1 + pgt2 + pgt3 + pgt4 + pgt5);

  if (restante == 0.) return;

  ui->doubleSpinBoxPgt3->setValue(pgt3 + restante);
  ui->doubleSpinBoxPgt3->setEnabled(true);
  ui->comboBoxPgt3->setEnabled(true);

  if (pgt3 == 0. or pgt4 >= 0.) {
    ui->doubleSpinBoxPgt3->setMaximum(restante + pgt3);
    ui->doubleSpinBoxPgt3->setValue(restante + pgt3);
    ui->doubleSpinBoxPgt4->setMaximum(pgt4);

    return;
  }

  if (pgt4 == 0.) {
    ui->doubleSpinBoxPgt4->setMaximum(restante + pgt4);
    ui->doubleSpinBoxPgt4->setValue(restante + pgt4);
    ui->doubleSpinBoxPgt3->setMaximum(pgt3);

    return;
  }

  //  ui->doubleSpinBoxPgt3->setMaximum(restante + pgt3);
  //  ui->doubleSpinBoxPgt3->setValue(restante + pgt3);

  //  ui->doubleSpinBoxPgt3->setEnabled(true);
  //  ui->comboBoxPgt3->setEnabled(true);

  montarFluxoCaixa();
}

void Venda::calculoSpinBox3() {
  const double pgt1 = ui->doubleSpinBoxPgt1->value();
  const double pgt2 = ui->doubleSpinBoxPgt2->value();
  const double pgt3 = ui->doubleSpinBoxPgt3->value();
  const double pgt4 = ui->doubleSpinBoxPgt4->value();
  const double pgt5 = ui->doubleSpinBoxPgt5->value();
  const double total = ui->doubleSpinBoxTotalPag->value();
  const double restante = total - (pgt1 + pgt2 + pgt3 + pgt4 + pgt5);

  if (restante == 0.) return;

  ui->doubleSpinBoxPgt4->setValue(pgt4 + restante);
  ui->doubleSpinBoxPgt4->setEnabled(true);
  ui->comboBoxPgt4->setEnabled(true);

  if (pgt4 == 0. or pgt5 >= 0.) {
    ui->doubleSpinBoxPgt4->setMaximum(restante + pgt4);
    ui->doubleSpinBoxPgt4->setValue(restante + pgt4);
    ui->doubleSpinBoxPgt5->setMaximum(pgt5);

    return;
  }

  if (pgt5 == 0.) {
    ui->doubleSpinBoxPgt5->setMaximum(restante + pgt5);
    ui->doubleSpinBoxPgt5->setValue(restante + pgt5);
    ui->doubleSpinBoxPgt4->setMaximum(pgt4);

    return;
  }

  //  ui->doubleSpinBoxPgt3->setMaximum(restante + pgt3);
  //  ui->doubleSpinBoxPgt3->setValue(restante + pgt3);

  //  ui->doubleSpinBoxPgt3->setEnabled(true);
  //  ui->comboBoxPgt3->setEnabled(true);

  montarFluxoCaixa();
}

void Venda::calculoSpinBox4() {
  const double pgt1 = ui->doubleSpinBoxPgt1->value();
  const double pgt2 = ui->doubleSpinBoxPgt2->value();
  const double pgt3 = ui->doubleSpinBoxPgt3->value();
  const double pgt4 = ui->doubleSpinBoxPgt4->value();
  const double pgt5 = ui->doubleSpinBoxPgt5->value();
  const double total = ui->doubleSpinBoxTotalPag->value();
  const double restante = total - (pgt1 + pgt2 + pgt3 + pgt4 + pgt5);

  if (restante == 0.) return;

  ui->doubleSpinBoxPgt5->setMaximum(restante + pgt5);
  ui->doubleSpinBoxPgt5->setValue(restante + pgt5);

  ui->doubleSpinBoxPgt5->setEnabled(true);
  ui->comboBoxPgt5->setEnabled(true);

  montarFluxoCaixa();
}

void Venda::on_doubleSpinBoxPgt2_valueChanged(double) { calculoSpinBox2(); }

void Venda::on_doubleSpinBoxPgt3_valueChanged(double) { calculoSpinBox3(); }

void Venda::on_doubleSpinBoxPgt4_valueChanged(double) { calculoSpinBox4(); }

void Venda::on_doubleSpinBoxPgt5_valueChanged(double) { montarFluxoCaixa(); }

void Venda::on_comboBoxPgt1_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") return;

  if (text == "Conta Cliente") {
    ui->doubleSpinBoxPgt1->setMaximum(ui->doubleSpinBoxCreditoTotal->value());
    ui->comboBoxPgt1Parc->clear();
    ui->comboBoxPgt1Parc->addItem("1x");
    montarFluxoCaixa();
    return;
  }

  QSqlQuery query;
  query.prepare("SELECT parcelas FROM forma_pagamento WHERE pagamento = :pagamento");
  query.bindValue(":pagamento", ui->comboBoxPgt1->currentText());

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo formas de pagamentos: " + query.lastError().text());
  }

  const int parcelas = query.value("parcelas").toInt();

  ui->comboBoxPgt1Parc->clear();

  for (int i = 0; i < parcelas; ++i) ui->comboBoxPgt1Parc->addItem(QString::number(i + 1) + "x");

  ui->comboBoxPgt1Parc->setEnabled(true);

  ui->dateEditPgt1->setEnabled(true);

  montarFluxoCaixa();
}

void Venda::on_comboBoxPgt2_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") return;

  if (text == "Conta Cliente") {
    ui->doubleSpinBoxPgt2->setMaximum(ui->doubleSpinBoxCreditoTotal->value());
    ui->comboBoxPgt2Parc->clear();
    ui->comboBoxPgt2Parc->addItem("1x");
    montarFluxoCaixa();
    return;
  }

  QSqlQuery query;
  query.prepare("SELECT parcelas FROM forma_pagamento WHERE pagamento = :pagamento");
  query.bindValue(":pagamento", ui->comboBoxPgt2->currentText());

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo formas de pagamentos: " + query.lastError().text());
  }

  const int parcelas = query.value("parcelas").toInt();

  ui->comboBoxPgt2Parc->clear();

  for (int i = 0; i < parcelas; ++i) ui->comboBoxPgt2Parc->addItem(QString::number(i + 1) + "x");

  ui->comboBoxPgt2Parc->setEnabled(true);

  ui->dateEditPgt2->setEnabled(true);

  montarFluxoCaixa();
}

void Venda::on_comboBoxPgt3_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") return;

  if (text == "Conta Cliente") {
    ui->doubleSpinBoxPgt3->setMaximum(ui->doubleSpinBoxCreditoTotal->value());
    ui->comboBoxPgt3Parc->clear();
    ui->comboBoxPgt3Parc->addItem("1x");
    montarFluxoCaixa();
    return;
  }

  QSqlQuery query;
  query.prepare("SELECT parcelas FROM forma_pagamento WHERE pagamento = :pagamento");
  query.bindValue(":pagamento", ui->comboBoxPgt3->currentText());

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo formas de pagamentos: " + query.lastError().text());
  }

  const int parcelas = query.value("parcelas").toInt();

  ui->comboBoxPgt3Parc->clear();

  for (int i = 0; i < parcelas; ++i) ui->comboBoxPgt3Parc->addItem(QString::number(i + 1) + "x");

  ui->comboBoxPgt3Parc->setEnabled(true);

  ui->dateEditPgt3->setEnabled(true);

  montarFluxoCaixa();
}

void Venda::on_comboBoxPgt4_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") return;

  if (text == "Conta Cliente") {
    ui->doubleSpinBoxPgt4->setMaximum(ui->doubleSpinBoxCreditoTotal->value());
    ui->comboBoxPgt4Parc->clear();
    ui->comboBoxPgt4Parc->addItem("1x");
    montarFluxoCaixa();
    return;
  }

  QSqlQuery query;
  query.prepare("SELECT parcelas FROM forma_pagamento WHERE pagamento = :pagamento");
  query.bindValue(":pagamento", ui->comboBoxPgt4->currentText());

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo formas de pagamentos: " + query.lastError().text());
  }

  const int parcelas = query.value("parcelas").toInt();

  ui->comboBoxPgt4Parc->clear();

  for (int i = 0; i < parcelas; ++i) ui->comboBoxPgt4Parc->addItem(QString::number(i + 1) + "x");

  ui->comboBoxPgt4Parc->setEnabled(true);

  ui->dateEditPgt4->setEnabled(true);

  montarFluxoCaixa();
}

void Venda::on_comboBoxPgt5_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") return;

  if (text == "Conta Cliente") {
    ui->doubleSpinBoxPgt5->setMaximum(ui->doubleSpinBoxCreditoTotal->value());
    ui->comboBoxPgt5Parc->clear();
    ui->comboBoxPgt5Parc->addItem("1x");
    montarFluxoCaixa();
    return;
  }

  QSqlQuery query;
  query.prepare("SELECT parcelas FROM forma_pagamento WHERE pagamento = :pagamento");
  query.bindValue(":pagamento", ui->comboBoxPgt5->currentText());

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo formas de pagamentos: " + query.lastError().text());
  }

  const int parcelas = query.value("parcelas").toInt();

  ui->comboBoxPgt5Parc->clear();

  for (int i = 0; i < parcelas; ++i) ui->comboBoxPgt5Parc->addItem(QString::number(i + 1) + "x");

  ui->comboBoxPgt5Parc->setEnabled(true);

  ui->dateEditPgt5->setEnabled(true);

  montarFluxoCaixa();
}

bool Venda::savingProcedures() {
  if (not setData("idVenda", ui->lineEditVenda->text())) return false;
  if (not setData("data", ui->dateTimeEdit->dateTime())) return false;
  if (not setData("dataOrc", ui->dateTimeEditOrc->dateTime())) return false;
  if (not setData("descontoPorc", ui->doubleSpinBoxDescontoGlobal->value())) return false;
  if (not setData("descontoReais", ui->doubleSpinBoxDescontoGlobalReais->value())) return false;
  if (not setData("frete", ui->doubleSpinBoxFrete->value())) return false;
  if (not setData("idCliente", ui->itemBoxCliente->value())) return false;
  if (not setData("idEnderecoEntrega", ui->itemBoxEndereco->value())) return false;
  if (not setData("idEnderecoFaturamento", ui->itemBoxEnderecoFat->value())) return false;
  if (not setData("idLoja", UserSession::fromLoja("usuario.idLoja", ui->itemBoxVendedor->text()))) return false;
  if (not setData("idOrcamento", m_idOrcamento)) return false;
  if (not setData("idProfissional", ui->itemBoxProfissional->value())) return false;
  if (not setData("idUsuario", ui->itemBoxVendedor->value())) return false;
  if (not setData("observacao", ui->plainTextEdit->toPlainText())) return false;
  if (not setData("prazoEntrega", ui->spinBoxPrazoEntrega->value())) return false;
  if (not setData("status", "PENDENTE")) return false;
  if (not setData("subTotalBru", ui->doubleSpinBoxSubTotalBruto->value())) return false;
  if (not setData("subTotalLiq", ui->doubleSpinBoxSubTotalLiq->value())) return false;
  if (not setData("total", ui->doubleSpinBoxTotal->value())) return false;
  if (not setData("representacao", ui->lineEditVenda->text().endsWith("R") ? 1 : 0)) return false;

  return true;
}

void Venda::registerMode() {
  ui->framePagamentos->show();
  ui->pushButtonGerarExcel->hide();
  ui->pushButtonImprimir->hide();
  ui->pushButtonCancelamento->hide();
  ui->pushButtonDevolucao->hide();
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

bool Venda::viewRegister() {
  ui->doubleSpinBoxDescontoGlobalReais->setMinimum(-9999999);

  modelItem.setFilter("idVenda = '" + model.data(0, "idVenda").toString() + "'");

  if (not modelItem.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela venda_has_produto: " + modelItem.lastError().text());
    return false;
  }

  if (not RegisterDialog::viewRegister()) return false;

  const QString idCliente = ui->itemBoxCliente->value().toString();

  ui->itemBoxEndereco->searchDialog()->setFilter("idCliente = " + idCliente + " AND desativado = FALSE");
  ui->itemBoxEnderecoFat->searchDialog()->setFilter("idCliente = " + idCliente + " AND desativado = FALSE");

  modelFluxoCaixa.setFilter("idVenda = '" + ui->lineEditVenda->text() +
                            "' AND status != 'CANCELADO' AND comissao = FALSE AND taxa = FALSE");

  if (not modelFluxoCaixa.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela conta_a_receber_has_pagamento: " + modelFluxoCaixa.lastError().text());
    return false;
  }

  for (int row = 0; row < modelFluxoCaixa.rowCount(); ++row) {
    ui->tableFluxoCaixa->openPersistentEditor(row, "representacao");
  }

  if (financeiro) {
    modelFluxoCaixa2.setFilter("idVenda = '" + ui->lineEditVenda->text() +
                               "' AND status != 'CANCELADO' AND (comissao = TRUE OR taxa = TRUE)");

    if (not modelFluxoCaixa2.select()) {
      QMessageBox::critical(this, "Erro!",
                            "Erro lendo tabela conta_a_receber_has_pagamento: " + modelFluxoCaixa.lastError().text());
      return false;
    }

    ui->comboBoxFinanceiro->setCurrentText(model.data(0, "statusFinanceiro").toString());
  }

  ui->tableVenda->resizeColumnsToContents();
  ui->tableFluxoCaixa->resizeColumnsToContents();
  ui->tableFluxoCaixa2->resizeColumnsToContents();

  ui->tableFluxoCaixa->setEditTriggers(QAbstractItemView::NoEditTriggers);

  ui->spinBoxPrazoEntrega->setReadOnly(true);

  ui->itemBoxCliente->setReadOnlyItemBox(true);
  ui->itemBoxEndereco->setReadOnlyItemBox(true);
  ui->itemBoxEnderecoFat->setReadOnlyItemBox(true);
  ui->itemBoxProfissional->setReadOnlyItemBox(true);
  ui->itemBoxVendedor->setReadOnlyItemBox(true);

  ui->dateTimeEdit->setReadOnly(true);

  ui->plainTextEdit->setReadOnly(true);
  ui->plainTextEdit->setPlainText(data("observacao").toString());

  ui->pushButtonCancelamento->show();
  ui->pushButtonDevolucao->show();

  if (data("status") == "CANCELADO" or data("status") == "DEVOLUÇÃO") {
    ui->pushButtonCancelamento->hide();
    ui->pushButtonDevolucao->hide();
  }

  const QString tipoUsuario = UserSession::tipoUsuario();

  if (not tipoUsuario.contains("GERENTE") and tipoUsuario != "DIRETOR" and tipoUsuario != "ADMINISTRADOR") {
    ui->pushButtonDevolucao->hide();
    ui->pushButtonCancelamento->hide();
  }

  ui->framePagamentos->adjustSize();

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

  modelFluxoCaixa.select();
  modelFluxoCaixa2.select();

  if (financeiro) {
    for (int row = 0; row < modelFluxoCaixa.rowCount(); ++row) {
      if (not modelFluxoCaixa.setData(row, "status", "SUBSTITUIDO")) {
        QMessageBox::critical(this, "Erro!", "Erro mudando status para 'SUBSTITUIDO'!");
        return;
      }
    }

    for (int row = 0; row < modelFluxoCaixa2.rowCount(); ++row) {
      if (not modelFluxoCaixa2.setData(row, "status", "SUBSTITUIDO")) {
        QMessageBox::critical(this, "Erro!", "Erro mudando status para 'SUBSTITUIDO'!");
        return;
      }
    }
  }

  const QList<QCheckBox *> checkBoxRep(
      {ui->checkBoxRep1, ui->checkBoxRep2, ui->checkBoxRep3, ui->checkBoxRep4, ui->checkBoxRep5});
  const QList<QComboBox *> comboParc(
      {ui->comboBoxPgt1Parc, ui->comboBoxPgt2Parc, ui->comboBoxPgt3Parc, ui->comboBoxPgt4Parc, ui->comboBoxPgt5Parc});
  const QList<QComboBox *> comboPgt(
      {ui->comboBoxPgt1, ui->comboBoxPgt2, ui->comboBoxPgt3, ui->comboBoxPgt4, ui->comboBoxPgt5});
  const QList<QDateEdit *> datePgt(
      {ui->dateEditPgt1, ui->dateEditPgt2, ui->dateEditPgt3, ui->dateEditPgt4, ui->dateEditPgt5});
  const QList<QDoubleSpinBox *> spinPgt({ui->doubleSpinBoxPgt1, ui->doubleSpinBoxPgt2, ui->doubleSpinBoxPgt3,
                                         ui->doubleSpinBoxPgt4, ui->doubleSpinBoxPgt5});
  const QList<QLineEdit *> linePgt(
      {ui->lineEditPgt1, ui->lineEditPgt2, ui->lineEditPgt3, ui->lineEditPgt4, ui->lineEditPgt5});

  for (int i = 0; i < 5; ++i) {
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
        modelFluxoCaixa.setData(row, "contraParte", ui->itemBoxCliente->text());
        modelFluxoCaixa.setData(row, "dataEmissao", QDate::currentDate());
        modelFluxoCaixa.setData(row, "idVenda", ui->lineEditVenda->text());
        modelFluxoCaixa.setData(row, "idLoja", data("idLoja"));
        modelFluxoCaixa.setData(row, "dataPagamento", datePgt.at(i)->date().addMonths(x + temp2));
        modelFluxoCaixa.setData(row, "valor", parcela + (x == 0 ? resto : 0));
        modelFluxoCaixa.setData(row, "tipo", QString::number(i + 1) + ". " + comboPgt.at(i)->currentText());
        modelFluxoCaixa.setData(row, "parcela", parcelas - y);
        modelFluxoCaixa.setData(row, "observacao", linePgt.at(i)->text());
        modelFluxoCaixa.setData(row, "representacao", checkBoxRep.at(i)->isChecked());
      }

      // calculo comissao
      for (int z = 0, total = modelFluxoCaixa.rowCount(); z < total; ++z) {
        if (modelFluxoCaixa.data(z, "representacao").toBool() == false) continue;
        if (not modelFluxoCaixa.data(z, "tipo").toString().contains(QString::number(i + 1))) continue;
        if (modelFluxoCaixa.data(z, "status").toString() == "SUBSTITUIDO") continue;

        const QString fornecedor = modelItem.data(0, "fornecedor").toString();

        QSqlQuery query;
        query.prepare("SELECT comissaoLoja FROM fornecedor WHERE razaoSocial = :razaoSocial");
        query.bindValue(":razaoSocial", fornecedor);

        if (not query.exec() or not query.first()) {
          QMessageBox::critical(this, "Erro!", "Erro buscando comissão: " + query.lastError().text());
          return;
        }

        const double comissao = query.value("comissaoLoja").toDouble() / 100;

        const double valor = modelFluxoCaixa.data(z, "valor").toDouble();

        double valorAjustado = comissao * (valor - (valor / data("total").toDouble() * data("frete").toDouble()));

        if (modelFluxoCaixa.data(0, "observacao").toString() == "FRETE") valorAjustado = valor * comissao;

        const int row = modelFluxoCaixa2.rowCount();
        modelFluxoCaixa2.insertRow(row);

        modelFluxoCaixa2.setData(row, "contraParte", modelItem.data(0, "fornecedor"));
        modelFluxoCaixa2.setData(row, "dataEmissao", QDate::currentDate());
        modelFluxoCaixa2.setData(row, "idVenda", ui->lineEditVenda->text());
        modelFluxoCaixa2.setData(row, "idLoja", data("idLoja"));
        modelFluxoCaixa2.setData(row, "dataPagamento", modelFluxoCaixa.data(z, "dataPagamento").toDate().addMonths(1));
        modelFluxoCaixa2.setData(row, "valor", valorAjustado);
        modelFluxoCaixa2.setData(row, "tipo", QString::number(i + 1) + ". Comissão");
        modelFluxoCaixa2.setData(row, "parcela", modelFluxoCaixa.data(z, "parcela").toString());
        modelFluxoCaixa2.setData(row, "comissao", 1);
      }
      //

      // calculo taxas cartao
      for (int z = 0, total = modelFluxoCaixa.rowCount(); z < total; ++z) {
        if (not modelFluxoCaixa.data(z, "tipo").toString().contains(QString::number(i + 1))) continue;
        if (modelFluxoCaixa.data(z, "status").toString() == "SUBSTITUIDO") continue;
        if (modelFluxoCaixa.data(z, "representacao").toBool()) continue;
        if (modelFluxoCaixa.data(z, "tipo").toString().contains("Conta Cliente")) continue;

        QSqlQuery query;
        query.prepare("SELECT taxa FROM forma_pagamento fp LEFT JOIN forma_pagamento_has_taxa fpt ON fp.idPagamento = "
                      "fpt.idPagamento WHERE pagamento = :pagamento AND parcela = :parcela");
        query.bindValue(":pagamento", comboPgt.at(i)->currentText());
        query.bindValue(":parcela", comboParc.at(i)->currentText().remove("x").toInt());

        if (not query.exec() or not query.first()) {
          QMessageBox::critical(this, "Erro!", "Erro buscando taxa: " + query.lastError().text());
          return;
        }

        if (query.value("taxa").toDouble() == 0.) continue;

        const double taxa = query.value("taxa").toDouble() / 100;

        const int row = modelFluxoCaixa2.rowCount();
        modelFluxoCaixa2.insertRow(row);
        modelFluxoCaixa2.setData(row, "contraParte", "Administradora Cartão");
        modelFluxoCaixa2.setData(row, "dataEmissao", QDate::currentDate());
        modelFluxoCaixa2.setData(row, "idVenda", ui->lineEditVenda->text());
        modelFluxoCaixa2.setData(row, "idLoja", data("idLoja"));
        modelFluxoCaixa2.setData(row, "dataPagamento", modelFluxoCaixa.data(z, "dataPagamento"));
        modelFluxoCaixa2.setData(row, "valor", taxa * modelFluxoCaixa.data(z, "valor").toDouble() * -1);
        modelFluxoCaixa2.setData(row, "tipo", QString::number(i + 1) + ". Taxa Cartão");
        modelFluxoCaixa2.setData(row, "parcela", modelFluxoCaixa.data(z, "parcela").toString());
        modelFluxoCaixa2.setData(row, "taxa", 1);
      }
    }
  }

  for (int row = 0; row < modelFluxoCaixa.rowCount(); ++row) {
    ui->tableFluxoCaixa->openPersistentEditor(row, "representacao");
  }

  ui->tableFluxoCaixa->resizeColumnsToContents();
  ui->tableFluxoCaixa2->resizeColumnsToContents();
}

void Venda::on_pushButtonLimparPag_clicked() { resetarPagamentos(); }

void Venda::on_doubleSpinBoxTotal_valueChanged(const double) {
  if (isBlockedGlobal) return;

  const double liq = ui->doubleSpinBoxSubTotalLiq->value();
  const double frete = ui->doubleSpinBoxFrete->value();
  const double total = ui->doubleSpinBoxTotal->value();
  const double value = 100. * (1. - ((total - frete) / liq));

  if (liq == 0.) return;

  isBlockedTotal = true;
  ui->doubleSpinBoxDescontoGlobal->setValue(value);
  isBlockedTotal = false;
}

void Venda::on_checkBoxFreteManual_clicked(const bool checked) {
  ui->doubleSpinBoxFrete->setFrame(checked);
  ui->doubleSpinBoxFrete->setReadOnly(not checked);
  ui->doubleSpinBoxFrete->setButtonSymbols(checked ? QDoubleSpinBox::UpDownArrows : QDoubleSpinBox::NoButtons);

  ui->doubleSpinBoxFrete->setValue(ui->checkBoxFreteManual->isChecked()
                                       ? ui->doubleSpinBoxFrete->value()
                                       : qMax(ui->doubleSpinBoxSubTotalBruto->value() * porcFrete / 100., minimoFrete));
}

void Venda::on_doubleSpinBoxFrete_valueChanged(const double) {
  if (isBlockedGlobal) return;

  calcPrecoGlobalTotal();
}

void Venda::on_doubleSpinBoxDescontoGlobal_valueChanged(const double) {
  if (isBlockedGlobal) return;

  isBlockedGlobal = true;
  calcPrecoGlobalTotal();
  isBlockedGlobal = false;
}

void Venda::on_doubleSpinBoxDescontoGlobalReais_valueChanged(const double) {
  if (isBlockedGlobal) return;

  const double liq = ui->doubleSpinBoxSubTotalLiq->value();
  const double desc = ui->doubleSpinBoxDescontoGlobalReais->value();

  if (liq == 0.) return;

  isBlockedReais = true;
  ui->doubleSpinBoxDescontoGlobal->setValue(100 * (1 - ((liq - desc) / liq)));
  isBlockedReais = false;
}

void Venda::on_pushButtonImprimir_clicked() {
  Impressao impressao(data("idVenda").toString());
  impressao.print();
}

void Venda::successMessage() {
  QMessageBox::information(this, "Atenção!", isUpdate ? "Cadastro atualizado!" : "Venda cadastrada com sucesso!");
}

void Venda::on_pushButtonGerarExcel_clicked() {
  Excel excel(ui->lineEditVenda->text());
  excel.gerarExcel();
}

bool Venda::atualizarCredito() {
  double creditoRestante = ui->doubleSpinBoxCreditoTotal->value();

  if (ui->comboBoxPgt1->currentText() == "Conta Cliente") creditoRestante -= ui->doubleSpinBoxPgt1->value();
  if (ui->comboBoxPgt2->currentText() == "Conta Cliente") creditoRestante -= ui->doubleSpinBoxPgt2->value();
  if (ui->comboBoxPgt3->currentText() == "Conta Cliente") creditoRestante -= ui->doubleSpinBoxPgt3->value();
  if (ui->comboBoxPgt4->currentText() == "Conta Cliente") creditoRestante -= ui->doubleSpinBoxPgt4->value();
  if (ui->comboBoxPgt5->currentText() == "Conta Cliente") creditoRestante -= ui->doubleSpinBoxPgt5->value();

  if (ui->comboBoxPgt1->currentText() == "Conta Cliente" or ui->comboBoxPgt2->currentText() == "Conta Cliente" or
      ui->comboBoxPgt3->currentText() == "Conta Cliente" or ui->comboBoxPgt4->currentText() == "Conta Cliente" or
      ui->comboBoxPgt5->currentText() == "Conta Cliente") {
    QSqlQuery query;
    query.prepare("UPDATE cliente SET credito = :credito WHERE idCliente = :idCliente");
    query.bindValue(":credito", creditoRestante);
    query.bindValue(":idCliente", data("idCliente"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando crédito do cliente: " + query.lastError().text());
      return false;
    }
  }

  return true;
}

bool Venda::cadastrar() {
  if (not verifyFields()) return false;

  row = isUpdate ? mapper.currentIndex() : model.rowCount();

  if (row == -1) {
    QMessageBox::critical(this, "Erro!", "Erro linha -1 Venda: " + QString::number(isUpdate) + "\nMapper: " +
                                             QString::number(mapper.currentIndex()) + "\nModel: " +
                                             QString::number(model.rowCount()));
    return false;
  }

  if (not generateId()) return false;

  if (not savingProcedures()) return false;

  if (not atualizarCredito()) return false;

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro ao cadastrar: " + model.lastError().text());
    return false;
  }

  primaryId = ui->lineEditVenda->text();

  for (int row = 0; row < modelFluxoCaixa.rowCount(); ++row) {
    if (not modelFluxoCaixa.setData(row, "idVenda", ui->lineEditVenda->text())) return false;
  }

  if (not modelFluxoCaixa.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados na tabela conta_a_receber_has_pagamento: " +
                                             modelFluxoCaixa.lastError().text());
    return false;
  }

  for (int row = 0; row < modelFluxoCaixa2.rowCount(); ++row) {
    if (not modelFluxoCaixa2.setData(row, "idVenda", ui->lineEditVenda->text())) return false;
  }

  if (not modelFluxoCaixa2.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados na tabela conta_a_receber_has_pagamento: " +
                                             modelFluxoCaixa2.lastError().text());
    return false;
  }

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    if (not modelItem.setData(row, "idVenda", ui->lineEditVenda->text())) return false;
  }

  if (not modelItem.submitAll()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro salvando dados na tabela venda_has_produto: " + modelItem.lastError().text());
    return false;
  }

  QSqlQuery query;

  query.prepare("UPDATE orcamento SET status = 'FECHADO' WHERE idOrcamento = :idOrcamento");
  query.bindValue(":idOrcamento", m_idOrcamento);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro marcando orçamento como 'FECHADO': " + query.lastError().text());
    return false;
  }

  return true;
}

bool Venda::save() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not cadastrar()) {
    QSqlQuery("ROLLBACK").exec();
    return false;
  }

  QSqlQuery("COMMIT").exec();

  isDirty = false;

  viewRegisterById(primaryId);

  if (not silent) successMessage();

  return true;
}

bool Venda::cancelamento() {
  bool ok = true;

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    if (modelItem.data(row, "status").toString() != "PENDENTE") {
      ok = false;
      break;
    }
  }

  if (not ok) {
    QMessageBox::critical(this, "Erro!", "Um ou mais produtos não estão pendentes!");
    return false;
  }

  QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Tem certeza que deseja cancelar?",
                     QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Cancelar venda");
  msgBox.setButtonText(QMessageBox::No, "Voltar");

  if (msgBox.exec() != QMessageBox::Yes) return false;

  const QString idOrcamento = data("idOrcamento").toString();

  QMessageBox::information(this, "Aviso!", idOrcamento.isEmpty() ? "Não havia Orçamento associado para ativar!"
                                                                 : "Orçamento " + idOrcamento + " reativado!");

  QSqlQuery query;

  if (not idOrcamento.isEmpty()) {
    query.prepare("UPDATE orcamento SET status = 'ATIVO' WHERE idOrcamento = :idOrcamento");
    query.bindValue(":idOrcamento", idOrcamento);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro reativando orçamento: " + query.lastError().text());
      return false;
    }
  }

  //---------------------------

  query.prepare("UPDATE venda SET status = 'CANCELADO' WHERE idVenda = :idVenda");
  query.bindValue(":idVenda", ui->lineEditVenda->text());

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro marcando venda como cancelada: " + query.lastError().text());
    return false;
  }

  //---------------------------

  query.prepare("UPDATE venda_has_produto SET status = 'CANCELADO' WHERE idVenda = :idVenda");
  query.bindValue(":idVenda", ui->lineEditVenda->text());

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro marcando produtos da venda como cancelados: " + query.lastError().text());
    return false;
  }

  //---------------------------
  for (int row = 0; row < modelFluxoCaixa.rowCount(); ++row) {
    if (modelFluxoCaixa.data(row, "tipo").toString().contains("Conta Cliente")) {
      if (modelFluxoCaixa.data(row, "status").toString() == "CANCELADO") continue;
      const double credito = modelFluxoCaixa.data(row, "valor").toDouble();

      query.prepare("UPDATE cliente SET credito = credito + :valor WHERE idCliente = :idCliente");
      query.bindValue(":valor", credito);
      query.bindValue(":idCliente", model.data(0, "idCliente"));

      if (not query.exec()) {
        QMessageBox::critical(this, "Erro!", "Erro voltando credito do cliente: " + query.lastError().text());
        return false;
      }
    }
  }

  query.prepare("UPDATE conta_a_receber_has_pagamento SET status = 'CANCELADO' WHERE idVenda = :idVenda");
  query.bindValue(":idVenda", ui->lineEditVenda->text());

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro marcando contas como canceladas: " + query.lastError().text());
    return false;
  }

  //---------------------------

  return true;
}

void Venda::on_pushButtonCancelamento_clicked() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not cancelamento()) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  QMessageBox::information(this, "Aviso!", "Venda cancelada!");
  close();
}

bool Venda::generateId() {
  QString id = UserSession::fromLoja("sigla", ui->itemBoxVendedor->text()) + "-" + QDate::currentDate().toString("yy") +
               UserSession::fromLoja("loja.idLoja", ui->itemBoxVendedor->text());

  QSqlQuery query;
  query.prepare("SELECT MAX(idVenda) AS idVenda FROM venda WHERE idVenda LIKE :id");
  query.bindValue(":id", id + "%");

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro na query: " + query.lastError().text());
    return false;
  }

  const int last = query.first() ? query.value("idVenda").toString().remove(id).left(3).toInt() : 0;

  id += QString("%1").arg(last + 1, 3, 10, QChar('0'));

  query.prepare("SELECT representacao FROM orcamento WHERE idOrcamento = :idOrcamento");
  query.bindValue(":idOrcamento", m_idOrcamento);

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro na query: " + query.lastError().text());
    return false;
  }

  if (id.size() != 11) {
    QMessageBox::critical(this, "Erro!", "Ocorreu algum erro ao gerar id: " + id);
    return false;
  }

  id += query.value("representacao").toBool() ? "R" : "";

  ui->lineEditVenda->setText(id);

  return true;
}

void Venda::on_pushButtonDevolucao_clicked() {
  // NOTE: criar separado devolucoes totais ou parciais
  Devolucao *devolucao = new Devolucao(data("idVenda").toString(), this);
  devolucao->show();
}

void Venda::on_tableFluxoCaixa_entered(const QModelIndex &) { ui->tableFluxoCaixa->resizeColumnsToContents(); }

void Venda::on_tableFluxoCaixa2_entered(const QModelIndex &) { ui->tableFluxoCaixa2->resizeColumnsToContents(); }

void Venda::on_dateTimeEdit_dateTimeChanged(const QDateTime &) { resetarPagamentos(); }

void Venda::on_pushButtonFreteLoja_clicked() {
  resetarPagamentos();

  ui->checkBoxRep1->setChecked(false);
  ui->lineEditPgt1->setText("Frete");
  ui->lineEditPgt1->setReadOnly(true);

  ui->doubleSpinBoxPgt1->setValue(ui->doubleSpinBoxFrete->value());
  ui->doubleSpinBoxPgt1->setReadOnly(true);
}

void Venda::on_pushButtonPgtLoja_clicked() {
  LoginDialog dialog(LoginDialog::Autorizacao, this);

  if (dialog.exec() == QDialog::Rejected) return;

  resetarPagamentos();

  ui->checkBoxRep1->setChecked(false);
  ui->checkBoxRep2->setChecked(false);
  ui->checkBoxRep3->setChecked(false);
  ui->checkBoxRep4->setChecked(false);
  ui->checkBoxRep5->setChecked(false);
}

void Venda::setFinanceiro() {
  ui->groupBoxFinanceiro->show();
  ui->tableFluxoCaixa2->show();

  if (UserSession::tipoUsuario() != "ADMINISTRADOR" and UserSession::tipoUsuario() != "GERENTE DEPARTAMENTO") {
    ui->pushButtonCorrigirFluxo->hide();
  }

  ui->frameButtons->hide();
  financeiro = true;
}

bool Venda::financeiroSalvar() {
  if (not model.setData(mapper.currentIndex(), "statusFinanceiro", ui->comboBoxFinanceiro->currentText())) {
    QMessageBox::critical(this, "Erro!", "Erro salvando status financeiro: " + model.lastError().text());
    return false;
  }

  if (not model.setData(mapper.currentIndex(), "dataFinanceiro", QDateTime::currentDateTime())) {
    QMessageBox::critical(this, "Erro!", "Erro salvando status financeiro: " + model.lastError().text());
    return false;
  }

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados: " + model.lastError().text());
    return false;
  }

  if (not modelFluxoCaixa.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando pagamentos: " + modelFluxoCaixa.lastError().text());
    return false;
  }

  if (not modelFluxoCaixa2.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando taxa/comissão: " + modelFluxoCaixa2.lastError().text());
    return false;
  }

  return true;
}

void Venda::on_pushButtonFinanceiroSalvar_clicked() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not financeiroSalvar()) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  QMessageBox::information(this, "Aviso!", "Dados salvos com sucesso!");
  close();
}

void Venda::on_pushButtonCorrigirFluxo_clicked() {
  QStringList list{"Escolha uma opção!"};

  QSqlQuery queryPag;
  queryPag.prepare("SELECT pagamento FROM view_pagamento_loja WHERE idLoja = :idLoja");
  queryPag.bindValue(":idLoja", data("idLoja"));

  if (not queryPag.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo formas de pagamentos: " + queryPag.lastError().text());
    return;
  }

  while (queryPag.next()) list << queryPag.value("pagamento").toString();

  ui->comboBoxPgt1->insertItems(0, list);
  ui->comboBoxPgt2->insertItems(0, list);
  ui->comboBoxPgt3->insertItems(0, list);
  ui->comboBoxPgt4->insertItems(0, list);
  ui->comboBoxPgt5->insertItems(0, list);

  ui->comboBoxPgt1->addItem("Conta Cliente");

  //

  ui->framePagamentos_2->show();
  resetarPagamentos();
}

// NOTE: prazoEntrega por produto
// NOTE: bloquear desconto maximo por classe de funcionario
