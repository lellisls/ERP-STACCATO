#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

#include "doubledelegate.h"
#include "inputdialogfinanceiro.h"
#include "noeditdelegate.h"
#include "porcentagemdelegate.h"
#include "reaisdelegate.h"
#include "singleeditdelegate.h"
#include "ui_inputdialogfinanceiro.h"
#include "usersession.h"

InputDialogFinanceiro::InputDialogFinanceiro(const Type &type, QWidget *parent) : QDialog(parent), type(type), ui(new Ui::InputDialogFinanceiro) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables();

  ui->frameData->hide();
  ui->frameDataPreco->hide();
  ui->frameFrete->hide();
  ui->checkBoxMarcarTodos->hide();
  ui->groupBoxFinanceiro->hide();
  ui->framePgt1->hide();
  ui->framePgt2->hide();
  ui->framePgt3->hide();
  ui->framePgt4->hide();
  ui->framePgt5->hide();
  ui->framePgtTotal->hide();

  ui->dateEditEvento->setDate(QDate::currentDate());
  ui->dateEditProximo->setDate(QDate::currentDate());

  ui->dateEditPgt1->setDate(QDate::currentDate());
  ui->dateEditPgt2->setDate(QDate::currentDate());
  ui->dateEditPgt3->setDate(QDate::currentDate());
  ui->dateEditPgt4->setDate(QDate::currentDate());
  ui->dateEditPgt5->setDate(QDate::currentDate());

  if (type == ConfirmarCompra) {
    ui->frameData->show();
    ui->frameDataPreco->show();
    ui->frameFrete->show();
    ui->checkBoxMarcarTodos->show();
    ui->framePgt1->show();
    ui->framePgt2->show();
    ui->framePgt3->show();
    ui->framePgt4->show();
    ui->framePgt5->show();
    ui->framePgtTotal->show();

    ui->labelEvento->setText("Data confirmação:");
    ui->labelProximoEvento->setText("Data prevista faturamento:");

    showMaximized();
  }

  if (type == Financeiro) {
    ui->frameDataPreco->show();
    ui->groupBoxFinanceiro->show();

    show();
  }
}

InputDialogFinanceiro::~InputDialogFinanceiro() { delete ui; }

QDateTime InputDialogFinanceiro::getDate() const { return ui->dateEditEvento->dateTime(); }

QDateTime InputDialogFinanceiro::getNextDate() const { return ui->dateEditProximo->dateTime(); }

void InputDialogFinanceiro::setupTables() {
  model.setTable("pedido_fornecedor_has_produto");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);
  model.setHeaderData("idVenda", "Código");
  model.setHeaderData("fornecedor", "Fornecedor");
  model.setHeaderData("descricao", "Produto");
  model.setHeaderData("colecao", "Coleção");
  model.setHeaderData("caixas", "Caixas");
  model.setHeaderData("prcUnitario", "$ Unit.");
  model.setHeaderData("quant", "Quant.");
  model.setHeaderData("preco", "Total");
  model.setHeaderData("un", "Un.");
  model.setHeaderData("un2", "Un.2");
  model.setHeaderData("kgcx", "Kg./Cx.");
  model.setHeaderData("formComercial", "Formato");
  model.setHeaderData("codComercial", "Cód. Com.");
  model.setHeaderData("obs", "Obs.");
  model.setHeaderData("aliquotaSt", "Alíquota ST");
  model.setHeaderData("st", "ST");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return;
  }

  ui->table->setModel(&model);
  ui->table->hideColumn("idVendaProduto");
  ui->table->hideColumn("statusFinanceiro");
  ui->table->hideColumn("ordemCompra");
  ui->table->hideColumn("quantConsumida");
  ui->table->hideColumn("idNfe");
  ui->table->hideColumn("idEstoque");
  ui->table->hideColumn("quantUpd");
  ui->table->hideColumn("selecionado");
  ui->table->hideColumn("idPedido");
  ui->table->hideColumn("idProduto");
  ui->table->hideColumn("codBarras");
  ui->table->hideColumn("idCompra");
  ui->table->hideColumn("status");
  ui->table->hideColumn("dataPrevCompra");
  ui->table->hideColumn("dataRealCompra");
  ui->table->hideColumn("dataPrevConf");
  ui->table->hideColumn("dataRealConf");
  ui->table->hideColumn("dataPrevFat");
  ui->table->hideColumn("dataRealFat");
  ui->table->hideColumn("dataPrevColeta");
  ui->table->hideColumn("dataRealColeta");
  ui->table->hideColumn("dataPrevReceb");
  ui->table->hideColumn("dataRealReceb");
  ui->table->hideColumn("dataPrevEnt");
  ui->table->hideColumn("dataRealEnt");
  ui->table->setItemDelegate(new NoEditDelegate(this));
  ui->table->setItemDelegateForColumn("prcUnitario", new ReaisDelegate(this));
  ui->table->setItemDelegateForColumn("preco", new ReaisDelegate(this));
  ui->table->setItemDelegateForColumn("aliquotaSt", new PorcentagemDelegate(this));

  if (type == ConfirmarCompra) ui->table->setItemDelegateForColumn("quant", new SingleEditDelegate(this));

  modelFluxoCaixa.setTable("conta_a_pagar_has_pagamento");
  modelFluxoCaixa.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelFluxoCaixa.setHeaderData("tipo", "Tipo");
  modelFluxoCaixa.setHeaderData("parcela", "Parcela");
  modelFluxoCaixa.setHeaderData("valor", "R$");
  modelFluxoCaixa.setHeaderData("dataPagamento", "Data");
  modelFluxoCaixa.setHeaderData("observacao", "Obs.");
  modelFluxoCaixa.setHeaderData("status", "Status");

  if (not modelFluxoCaixa.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela conta_a_receber_has_pagamento: " + modelFluxoCaixa.lastError().text());
    return;
  }

  ui->tableFluxoCaixa->setModel(&modelFluxoCaixa);
  ui->tableFluxoCaixa->setItemDelegate(new DoubleDelegate(this));
  ui->tableFluxoCaixa->hideColumn("nfe");
  ui->tableFluxoCaixa->hideColumn("contraParte");
  ui->tableFluxoCaixa->hideColumn("idCompra");
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
  ui->tableFluxoCaixa->hideColumn("taxa");
  ui->tableFluxoCaixa->hideColumn("desativado");
}

void InputDialogFinanceiro::wrapMontarFluxoCaixa() {
  if (isBlockedFluxoCaixa) return;

  isBlockedFluxoCaixa = true;
  montarFluxoCaixa();
  isBlockedFluxoCaixa = false;
}

void InputDialogFinanceiro::montarFluxoCaixa() {
  // TODO: 'ST Loja' tem a data do faturamento, 'ST Fornecedor' segue as datas dos pagamentos

  if (representacao) return;

  modelFluxoCaixa.select();

  if (type == Financeiro) {
    for (int row = 0; row < modelFluxoCaixa.rowCount(); ++row) {
      if (not modelFluxoCaixa.setData(row, "status", "SUBSTITUIDO")) {
        QMessageBox::critical(this, "Erro!", "Erro mudando status para 'SUBSTITUIDO'!");
        return;
      }
    }
  }

  const QList<QComboBox *> comboParc({ui->comboBoxPgt1Parc, ui->comboBoxPgt2Parc, ui->comboBoxPgt3Parc, ui->comboBoxPgt4Parc, ui->comboBoxPgt5Parc});
  const QList<QComboBox *> comboPgt({ui->comboBoxPgt1, ui->comboBoxPgt2, ui->comboBoxPgt3, ui->comboBoxPgt4, ui->comboBoxPgt5});
  const QList<QDateEdit *> datePgt({ui->dateEditPgt1, ui->dateEditPgt2, ui->dateEditPgt3, ui->dateEditPgt4, ui->dateEditPgt5});
  const QList<QComboBox *> datePgt2({ui->comboBoxData1, ui->comboBoxData2, ui->comboBoxData3, ui->comboBoxData4, ui->comboBoxData5});
  const QList<QDoubleSpinBox *> spinPgt({ui->doubleSpinBoxPgt1, ui->doubleSpinBoxPgt2, ui->doubleSpinBoxPgt3, ui->doubleSpinBoxPgt4, ui->doubleSpinBoxPgt5});
  const QList<QLineEdit *> linePgt({ui->lineEditPgt1, ui->lineEditPgt2, ui->lineEditPgt3, ui->lineEditPgt4, ui->lineEditPgt5});

  for (int i = 0; i < 5; ++i) {
    if (comboPgt.at(i)->currentText() != "Escolha uma opção!") {
      const int parcelas = comboParc.at(i)->currentIndex() + 1;
      const double valor = spinPgt.at(i)->value();
      const float temp =
          comboPgt.at(i)->currentText() == "Cartão de crédito" ? static_cast<float>(static_cast<int>(static_cast<float>(valor / parcelas) * 100)) / 100 : qRound(valor / parcelas * 100) / 100.;
      const double resto = static_cast<double>(valor - (temp * parcelas));
      const double parcela = static_cast<double>(temp);

      for (int x = 0, y = parcelas - 1; x < parcelas; ++x, --y) {
        const int row = modelFluxoCaixa.rowCount();
        modelFluxoCaixa.insertRow(row);
        modelFluxoCaixa.setData(row, "contraParte", model.data(0, "fornecedor"));
        modelFluxoCaixa.setData(row, "dataEmissao", ui->dateEditEvento->dateTime());
        modelFluxoCaixa.setData(row, "idCompra", model.data(0, "idCompra"));
        modelFluxoCaixa.setData(row, "idLoja", 1); // Geral
        const QDate dataPgt = (datePgt2.at(i)->currentText() == "Data + 1 Mês"
                                   ? datePgt.at(i)->date().addMonths(x + 1)
                                   : datePgt2.at(i)->currentText() == "Data Mês" ? datePgt.at(i)->date().addMonths(x) : datePgt.at(i)->date().addDays(datePgt2.at(i)->currentText().toInt() * (x + 1)));

        modelFluxoCaixa.setData(row, "dataPagamento", dataPgt);
        modelFluxoCaixa.setData(row, "valor", parcela + (x == 0 ? resto : 0));
        modelFluxoCaixa.setData(row, "tipo", QString::number(i + 1) + ". " + comboPgt.at(i)->currentText());
        modelFluxoCaixa.setData(row, "parcela", parcelas - y);
        modelFluxoCaixa.setData(row, "observacao", linePgt.at(i)->text());
      }
    }
  }

  if (ui->doubleSpinBoxFrete->value() > 0) {
    const int row = modelFluxoCaixa.rowCount();
    modelFluxoCaixa.insertRow(row);
    modelFluxoCaixa.setData(row, "contraParte", ""); // TODO: preencher com 'PROPRIO' do checkbox ou com o que foi digitado ao lado
    modelFluxoCaixa.setData(row, "dataEmissao", ui->dateEditEvento->dateTime());
    modelFluxoCaixa.setData(row, "idCompra", model.data(0, "idCompra"));
    modelFluxoCaixa.setData(row, "idLoja", 1);                           // Geral
    modelFluxoCaixa.setData(row, "dataPagamento", QDate::currentDate()); // TODO: redo this with a editable date
    modelFluxoCaixa.setData(row, "valor", ui->doubleSpinBoxFrete->value());
    modelFluxoCaixa.setData(row, "tipo", "Frete");
    modelFluxoCaixa.setData(row, "parcela", 1);
    modelFluxoCaixa.setData(row, "observacao", "");
  }

  // set st date
  if (not isBlockedStDate) {

    QDate date = modelFluxoCaixa.data(0, "dataPagamento").toDate();

    for (int row = 1; row < modelFluxoCaixa.rowCount(); ++row) {
      const QDate current = modelFluxoCaixa.data(row, "dataPagamento").toDate();

      if (current < date) date = current;
    }

    ui->dateEditPgtSt->setDate(date);
  }

  if (ui->doubleSpinBoxSt->value() > 0) {
    const int row = modelFluxoCaixa.rowCount();
    modelFluxoCaixa.insertRow(row);

    modelFluxoCaixa.setData(row, "contraParte", model.data(0, "fornecedor"));
    modelFluxoCaixa.setData(row, "dataEmissao", ui->dateEditEvento->dateTime());
    modelFluxoCaixa.setData(row, "idCompra", model.data(0, "idCompra"));
    modelFluxoCaixa.setData(row, "idLoja", 1); // Geral
    modelFluxoCaixa.setData(row, "dataPagamento", ui->dateEditPgtSt->date());
    modelFluxoCaixa.setData(row, "valor", ui->doubleSpinBoxSt->value());
    modelFluxoCaixa.setData(row, "tipo", "ST Fornecedor");
    modelFluxoCaixa.setData(row, "parcela", 1);
    modelFluxoCaixa.setData(row, "observacao", "");
  }

  ui->tableFluxoCaixa->resizeColumnsToContents();
}

void InputDialogFinanceiro::calcularTotal() {
  double total = 0;
  double st = 0;

  const auto list = ui->table->selectionModel()->selectedRows();

  for (auto const &item : list) {
    const double preco = model.data(item.row(), "preco").toDouble();
    const QString tipoSt = model.data(item.row(), "st").toString();
    const double aliquota = model.data(item.row(), "aliquotaSt").toDouble();

    const bool isSt = (tipoSt == "ST Fornecedor" or tipoSt == "ST Loja");

    st += isSt ? preco * aliquota / 100 : 0;
    total += preco;
  }

  total -= ui->doubleSpinBoxAdicionais->value();

  ui->doubleSpinBoxSt->setValue(st);
  ui->doubleSpinBoxTotalPag->setValue(total);
}

void InputDialogFinanceiro::updateTableData(const QModelIndex &topLeft) {
  const QString header = model.headerData(topLeft.column(), Qt::Horizontal).toString();
  const int row = topLeft.row();

  if (header == "Quant." or header == "$ Unit.") {
    const double preco = model.data(row, "quant").toDouble() * model.data(row, "prcUnitario").toDouble();
    model.setData(row, "preco", preco);
  }

  if (header == "Total") {
    const double preco = model.data(row, "preco").toDouble() / model.data(row, "quant").toDouble();
    model.setData(row, "prcUnitario", preco);
  }

  calcularTotal();
}

void InputDialogFinanceiro::resetarPagamentos() {
  ui->doubleSpinBoxPgt1->setMaximum(ui->doubleSpinBoxTotalPag->value());
  ui->doubleSpinBoxPgt1->setValue(ui->doubleSpinBoxTotalPag->value());
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

  ui->comboBoxPgt1Parc->setCurrentIndex(0);
  ui->comboBoxPgt2Parc->setCurrentIndex(0);
  ui->comboBoxPgt3Parc->setCurrentIndex(0);
  ui->comboBoxPgt4Parc->setCurrentIndex(0);
  ui->comboBoxPgt5Parc->setCurrentIndex(0);

  ui->comboBoxPgt2->setDisabled(true);
  ui->comboBoxPgt3->setDisabled(true);
  ui->comboBoxPgt4->setDisabled(true);
  ui->comboBoxPgt5->setDisabled(true);
  ui->comboBoxPgt2Parc->setDisabled(true);
  ui->comboBoxPgt3Parc->setDisabled(true);
  ui->comboBoxPgt4Parc->setDisabled(true);
  ui->comboBoxPgt5Parc->setDisabled(true);
  ui->dateEditPgt2->setDisabled(true);
  ui->dateEditPgt3->setDisabled(true);
  ui->dateEditPgt4->setDisabled(true);
  ui->dateEditPgt5->setDisabled(true);

  ui->comboBoxData2->setDisabled(true);
  ui->comboBoxData3->setDisabled(true);
  ui->comboBoxData4->setDisabled(true);
  ui->comboBoxData5->setDisabled(true);

  ui->doubleSpinBoxPgt2->setDisabled(true);
  ui->doubleSpinBoxPgt3->setDisabled(true);
  ui->doubleSpinBoxPgt4->setDisabled(true);
  ui->doubleSpinBoxPgt5->setDisabled(true);

  wrapMontarFluxoCaixa();
}

bool InputDialogFinanceiro::setFilter(const QString &idCompra) {
  if (idCompra.isEmpty()) {
    model.setFilter("idPedido = 0");
    QMessageBox::critical(this, "Erro!", "IdCompra vazio!");
    return false;
  }

  if (type == ConfirmarCompra) model.setFilter("idCompra = " + idCompra + " AND status = 'EM COMPRA'");
  if (type == Financeiro) model.setFilter("idCompra = " + idCompra);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return false;
  }

  ui->table->resizeColumnsToContents();

  if (type == ConfirmarCompra or type == Financeiro) {
    modelFluxoCaixa.setFilter(type == ConfirmarCompra ? "0" : "idCompra = " + idCompra);

    if (not modelFluxoCaixa.select()) {
      QMessageBox::critical(this, "Erro!", "Erro lendo tabela conta_a_pagar_has_pagamento: " + modelFluxoCaixa.lastError().text());
      return false;
    }

    calcularTotal();

    ui->checkBoxMarcarTodos->setChecked(true);

    //

    QSqlQuery queryPag;
    queryPag.prepare("SELECT pagamento FROM view_pagamento_loja WHERE idLoja = :idLoja");
    queryPag.bindValue(":idLoja", UserSession::idLoja());

    if (not queryPag.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro lendo formas de pagamentos: " + queryPag.lastError().text());
      return false;
    }

    QStringList list{"Escolha uma opção!"};

    while (queryPag.next()) list << queryPag.value("pagamento").toString();

    ui->comboBoxPgt1->insertItems(0, list);
    ui->comboBoxPgt2->insertItems(0, list);
    ui->comboBoxPgt3->insertItems(0, list);
    ui->comboBoxPgt4->insertItems(0, list);
    ui->comboBoxPgt5->insertItems(0, list);

    ui->tableFluxoCaixa->resizeColumnsToContents();
  }

  if (type == Financeiro) ui->comboBoxFinanceiro->setCurrentText(model.data(0, "statusFinanceiro").toString());

  QSqlQuery query;
  query.prepare("SELECT v.representacao FROM pedido_fornecedor_has_produto pf LEFT JOIN venda v ON pf.idVenda = "
                "v.idVenda WHERE idCompra = :idCompra");
  query.bindValue(":idCompra", idCompra);

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando se é representacao: " + query.lastError().text());
    return false;
  }

  representacao = query.value("representacao").toBool();

  if (representacao) {
    ui->framePagamentos->hide();
    ui->frameFrete->hide();
  }

  setWindowTitle("OC: " + model.data(0, "ordemCompra").toString());

  // connects
  connect(ui->comboBoxPgt1Parc, &QComboBox::currentTextChanged, this, &InputDialogFinanceiro::wrapMontarFluxoCaixa);
  connect(ui->comboBoxPgt2Parc, &QComboBox::currentTextChanged, this, &InputDialogFinanceiro::wrapMontarFluxoCaixa);
  connect(ui->comboBoxPgt3Parc, &QComboBox::currentTextChanged, this, &InputDialogFinanceiro::wrapMontarFluxoCaixa);
  connect(ui->comboBoxPgt4Parc, &QComboBox::currentTextChanged, this, &InputDialogFinanceiro::wrapMontarFluxoCaixa);
  connect(ui->comboBoxPgt5Parc, &QComboBox::currentTextChanged, this, &InputDialogFinanceiro::wrapMontarFluxoCaixa);
  connect(ui->dateEditPgt1, &QDateEdit::dateChanged, this, &InputDialogFinanceiro::wrapMontarFluxoCaixa);
  connect(ui->dateEditPgt2, &QDateEdit::dateChanged, this, &InputDialogFinanceiro::wrapMontarFluxoCaixa);
  connect(ui->dateEditPgt3, &QDateEdit::dateChanged, this, &InputDialogFinanceiro::wrapMontarFluxoCaixa);
  connect(ui->dateEditPgt4, &QDateEdit::dateChanged, this, &InputDialogFinanceiro::wrapMontarFluxoCaixa);
  connect(ui->dateEditPgt5, &QDateEdit::dateChanged, this, &InputDialogFinanceiro::wrapMontarFluxoCaixa);
  connect(ui->lineEditPgt1, &QLineEdit::textChanged, this, &InputDialogFinanceiro::wrapMontarFluxoCaixa);
  connect(ui->lineEditPgt2, &QLineEdit::textChanged, this, &InputDialogFinanceiro::wrapMontarFluxoCaixa);
  connect(ui->lineEditPgt3, &QLineEdit::textChanged, this, &InputDialogFinanceiro::wrapMontarFluxoCaixa);
  connect(ui->lineEditPgt4, &QLineEdit::textChanged, this, &InputDialogFinanceiro::wrapMontarFluxoCaixa);
  connect(ui->lineEditPgt5, &QLineEdit::textChanged, this, &InputDialogFinanceiro::wrapMontarFluxoCaixa);
  connect(ui->comboBoxData1, &QComboBox::currentTextChanged, this, &InputDialogFinanceiro::wrapMontarFluxoCaixa);
  connect(ui->comboBoxData2, &QComboBox::currentTextChanged, this, &InputDialogFinanceiro::wrapMontarFluxoCaixa);
  connect(ui->comboBoxData3, &QComboBox::currentTextChanged, this, &InputDialogFinanceiro::wrapMontarFluxoCaixa);
  connect(ui->comboBoxData4, &QComboBox::currentTextChanged, this, &InputDialogFinanceiro::wrapMontarFluxoCaixa);
  connect(ui->comboBoxData5, &QComboBox::currentTextChanged, this, &InputDialogFinanceiro::wrapMontarFluxoCaixa);

  if (type == ConfirmarCompra) {
    connect(&model, &SqlTableModel::dataChanged, this, &InputDialogFinanceiro::updateTableData);
    connect(ui->table->selectionModel(), &QItemSelectionModel::selectionChanged, this, &InputDialogFinanceiro::calcularTotal);
    connect(ui->doubleSpinBoxTotalPag, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &InputDialogFinanceiro::resetarPagamentos);
  }

  //

  calcularTotal();

  return true;
}

void InputDialogFinanceiro::on_pushButtonSalvar_clicked() {
  if (type == ConfirmarCompra) {
    if (not verifyFields()) return;

    if (not cadastrar()) {
      if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
      return;
    }

    QDialog::accept();
    close();
  }

  //  if (type == ConfirmarCompra and not verifyFields()) return;

  if (type == Financeiro) {
    QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
    QSqlQuery("START TRANSACTION").exec();

    if (not cadastrar()) {
      QSqlQuery("ROLLBACK").exec();
      if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
      return;
    }

    QSqlQuery("COMMIT").exec();
    QMessageBox::information(this, "Aviso!", "Dados salvos com sucesso!");
    QDialog::accept();
    close();
  }
}

bool InputDialogFinanceiro::verifyFields() {
  if (ui->table->selectionModel()->selectedRows().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return false;
  }

  if (not representacao) {
    if (not qFuzzyCompare(ui->doubleSpinBoxPgt1->value() + ui->doubleSpinBoxPgt2->value() + ui->doubleSpinBoxPgt3->value() + ui->doubleSpinBoxPgt4->value() + ui->doubleSpinBoxPgt5->value(),
                          ui->doubleSpinBoxTotalPag->value())) {
      QMessageBox::critical(this, "Erro!", "Soma dos pagamentos não é igual ao total! Favor verificar.");
      return false;
    }

    if (ui->doubleSpinBoxPgt1->value() > 0 and ui->comboBoxPgt1->currentText() == "Escolha uma opção!") {
      QMessageBox::critical(this, "Erro!", "Por favor escolha a forma de pagamento 1.");
      ui->comboBoxPgt1->setFocus();
      return false;
    }

    if (ui->doubleSpinBoxPgt2->value() > 0 and ui->comboBoxPgt2->currentText() == "Escolha uma opção!") {
      QMessageBox::critical(this, "Erro!", "Por favor escolha a forma de pagamento 2.");
      ui->comboBoxPgt2->setFocus();
      return false;
    }

    if (ui->doubleSpinBoxPgt3->value() > 0 and ui->comboBoxPgt3->currentText() == "Escolha uma opção!") {
      QMessageBox::critical(this, "Erro!", "Por favor escolha a forma de pagamento 3.");
      ui->comboBoxPgt3->setFocus();
      return false;
    }

    if (ui->doubleSpinBoxPgt4->value() > 0 and ui->comboBoxPgt4->currentText() == "Escolha uma opção!") {
      QMessageBox::critical(this, "Erro!", "Por favor escolha a forma de pagamento 4.");
      ui->comboBoxPgt4->setFocus();
      return false;
    }

    if (ui->doubleSpinBoxPgt5->value() > 0 and ui->comboBoxPgt5->currentText() == "Escolha uma opção!") {
      QMessageBox::critical(this, "Erro!", "Por favor escolha a forma de pagamento 5.");
      ui->comboBoxPgt5->setFocus();
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
  }

  return true;
}

bool InputDialogFinanceiro::cadastrar() {
  if (type == ConfirmarCompra) {
    const auto list = ui->table->selectionModel()->selectedRows();

    for (auto const &item : list) model.setData(item.row(), "selecionado", true);
  }

  if (type == Financeiro) {
    if (not model.setData(0, "statusFinanceiro", ui->comboBoxFinanceiro->currentText())) {
      error = "Erro salvando status na tabela: " + model.lastError().text();
      return false;
    }
  }

  if (not model.submitAll()) {
    error = "Erro salvando dados na tabela: " + model.lastError().text();
    return false;
  }

  if (type == ConfirmarCompra or type == Financeiro) {
    if (not modelFluxoCaixa.submitAll()) {
      error = "Erro salvando dados do pagamento: " + modelFluxoCaixa.lastError().text();
      return false;
    }
  }

  return true;
}

void InputDialogFinanceiro::on_dateEditEvento_dateChanged(const QDate &date) {
  if (ui->dateEditProximo->date() < date) ui->dateEditProximo->setDate(date);
}

void InputDialogFinanceiro::on_checkBoxMarcarTodos_toggled(bool checked) { checked ? ui->table->selectAll() : ui->table->clearSelection(); }

void InputDialogFinanceiro::on_doubleSpinBoxPgt1_valueChanged(double) { calculoSpinBox1(); }

void InputDialogFinanceiro::on_doubleSpinBoxPgt2_valueChanged(double) { calculoSpinBox2(); }

void InputDialogFinanceiro::on_doubleSpinBoxPgt3_valueChanged(double) { calculoSpinBox3(); }

void InputDialogFinanceiro::on_doubleSpinBoxPgt4_valueChanged(double) { calculoSpinBox4(); }

void InputDialogFinanceiro::on_doubleSpinBoxPgt5_valueChanged(double) { wrapMontarFluxoCaixa(); }

void InputDialogFinanceiro::on_pushButtonCorrigirFluxo_clicked() {
  ui->framePgt1->show();
  ui->framePgt2->show();
  ui->framePgt3->show();
  ui->framePgt4->show();
  ui->framePgt5->show();
  ui->framePgtTotal->show();

  //

  if (modelFluxoCaixa.rowCount() > 0) {
    const QDate date = modelFluxoCaixa.data(0, "dataPagamento").toDate();

    ui->dateEditPgt1->setDate(date);
    ui->dateEditPgt2->setDate(date);
    ui->dateEditPgt3->setDate(date);
    ui->dateEditPgt4->setDate(date);
    ui->dateEditPgt5->setDate(date);
  }

  //

  // TODO: alterar para que apenas na tela do financeiro compra a opcao de corrigir fluxo percorra todas as linhas
  // (enquanto na confirmacao de pagamento percorre apenas as linhas selecionadas)

  calcularTotal();
  resetarPagamentos();
}

void InputDialogFinanceiro::on_pushButtonLimparPag_clicked() { resetarPagamentos(); }

void InputDialogFinanceiro::on_doubleSpinBoxFrete_valueChanged(double) {
  calcularTotal();
  wrapMontarFluxoCaixa();
}

void InputDialogFinanceiro::on_doubleSpinBoxAdicionais_valueChanged(const double value) {
  Q_UNUSED(value);
  // TODO: fracionar desconto nos itens
  // TODO: mudanca de selecao deve rodar esse codigo novamente para redistribuir o desconto

  /*

  QSqlQuery query;

  QString queryString;

  for (auto index : ui->table->selectionModel()->selectedRows()) {
    queryString += queryString.isEmpty() ? "idPedido = " + model.data(index.row(), "idPedido").toString()
                                         : " OR idPedido = " + model.data(index.row(), "idPedido").toString();
  }

  queryString.prepend("SELECT SUM(preco) AS total FROM pedido_fornecedor_has_produto WHERE ");

  if (not query.exec(queryString) or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando total: " + query.lastError().text());
    return;
  }

  const double total = query.value("total").toDouble();
  //  const double total = ui->doubleSpinBoxTotalPag->value();

  QVector<int> indexs;

  for (auto const index : ui->table->selectionModel()->selectedRows()) {
    indexs << index.row();
  }

  model.select();

  for (auto row : indexs) {
    ui->table->selectionModel()->select(model.index(row, 0),
                                        QItemSelectionModel::Rows | QItemSelectionModel::SelectCurrent);
  }

  // TODO: percorrer apenas as linhas selecionadas?

  for (auto const index : ui->table->selectionModel()->selectedRows()) {
    //  for (int row = 0; row < model.rowCount(); ++row) {
    const double totalProd = QString::number(model.data(index.row(), "preco").toDouble(), 'f', 2).toDouble();
    const double valorProporcional = totalProd - (totalProd / total * value);

    model.setData(index.row(), "preco", valorProporcional);

    model.setData(index.row(), "prcUnitario", valorProporcional / model.data(index.row(), "quant").toDouble());
  }
  // TODO: lidar com divisao por zero/nan

  */

  calcularTotal();
}

void InputDialogFinanceiro::calculoSpinBox1() {
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
  ui->comboBoxData2->setEnabled(true);
  ui->dateEditPgt2->setEnabled(true);

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

  wrapMontarFluxoCaixa();
}

void InputDialogFinanceiro::calculoSpinBox2() {
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
  ui->comboBoxData3->setEnabled(true);
  ui->dateEditPgt3->setEnabled(true);

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

  wrapMontarFluxoCaixa();
}

void InputDialogFinanceiro::calculoSpinBox3() {
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
  ui->comboBoxData4->setEnabled(true);
  ui->dateEditPgt4->setEnabled(true);

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

  wrapMontarFluxoCaixa();
}

void InputDialogFinanceiro::calculoSpinBox4() {
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
  ui->comboBoxData5->setEnabled(true);
  ui->dateEditPgt5->setEnabled(true);

  wrapMontarFluxoCaixa();
}

void InputDialogFinanceiro::on_comboBoxPgt1_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") return;

  QSqlQuery query;
  query.prepare("SELECT parcelas FROM forma_pagamento WHERE pagamento = :pagamento");
  query.bindValue(":pagamento", ui->comboBoxPgt1->currentText());

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo formas de pagamentos: " + query.lastError().text());
    return;
  }

  const int parcelas = query.value("parcelas").toInt();

  ui->comboBoxPgt1Parc->clear();

  for (int i = 0; i < parcelas; ++i) ui->comboBoxPgt1Parc->addItem(QString::number(i + 1) + "x");

  ui->comboBoxPgt1Parc->setEnabled(true);
  ui->dateEditPgt1->setEnabled(true);

  wrapMontarFluxoCaixa();
}

void InputDialogFinanceiro::on_comboBoxPgt2_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") return;

  QSqlQuery query;
  query.prepare("SELECT parcelas FROM forma_pagamento WHERE pagamento = :pagamento");
  query.bindValue(":pagamento", ui->comboBoxPgt2->currentText());

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo formas de pagamentos: " + query.lastError().text());
    return;
  }

  const int parcelas = query.value("parcelas").toInt();

  ui->comboBoxPgt2Parc->clear();

  for (int i = 0; i < parcelas; ++i) ui->comboBoxPgt2Parc->addItem(QString::number(i + 1) + "x");

  ui->comboBoxPgt2Parc->setEnabled(true);
  ui->dateEditPgt2->setEnabled(true);

  wrapMontarFluxoCaixa();
}

void InputDialogFinanceiro::on_comboBoxPgt3_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") return;

  QSqlQuery query;
  query.prepare("SELECT parcelas FROM forma_pagamento WHERE pagamento = :pagamento");
  query.bindValue(":pagamento", ui->comboBoxPgt3->currentText());

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo formas de pagamentos: " + query.lastError().text());
    return;
  }

  const int parcelas = query.value("parcelas").toInt();

  ui->comboBoxPgt3Parc->clear();

  for (int i = 0; i < parcelas; ++i) ui->comboBoxPgt3Parc->addItem(QString::number(i + 1) + "x");

  ui->comboBoxPgt3Parc->setEnabled(true);
  ui->dateEditPgt3->setEnabled(true);

  wrapMontarFluxoCaixa();
}

void InputDialogFinanceiro::on_comboBoxPgt4_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") return;

  QSqlQuery query;
  query.prepare("SELECT parcelas FROM forma_pagamento WHERE pagamento = :pagamento");
  query.bindValue(":pagamento", ui->comboBoxPgt4->currentText());

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo formas de pagamentos: " + query.lastError().text());
    return;
  }

  const int parcelas = query.value("parcelas").toInt();

  ui->comboBoxPgt4Parc->clear();

  for (int i = 0; i < parcelas; ++i) ui->comboBoxPgt4Parc->addItem(QString::number(i + 1) + "x");

  ui->comboBoxPgt4Parc->setEnabled(true);
  ui->dateEditPgt4->setEnabled(true);

  wrapMontarFluxoCaixa();
}

void InputDialogFinanceiro::on_comboBoxPgt5_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") return;

  QSqlQuery query;
  query.prepare("SELECT parcelas FROM forma_pagamento WHERE pagamento = :pagamento");
  query.bindValue(":pagamento", ui->comboBoxPgt5->currentText());

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo formas de pagamentos: " + query.lastError().text());
    return;
  }

  const int parcelas = query.value("parcelas").toInt();

  ui->comboBoxPgt5Parc->clear();

  for (int i = 0; i < parcelas; ++i) ui->comboBoxPgt5Parc->addItem(QString::number(i + 1) + "x");

  ui->comboBoxPgt5Parc->setEnabled(true);
  ui->dateEditPgt5->setEnabled(true);

  wrapMontarFluxoCaixa();
}

void InputDialogFinanceiro::on_dateEditPgtSt_userDateChanged(const QDate &) {
  isBlockedStDate = true;
  wrapMontarFluxoCaixa();
  isBlockedStDate = false;
}

// TODO: 1quando for confirmacao de representacao perguntar qual o id para colocar na observacao das comissoes (codigo
// que vem do fornecedor)
// TODO: quando for representacao mostrar fluxo de comissao
// TODO: colocar formas de pagamento 4 e 5
// TODO: colocar possibilidade de ajustar valor total para as compras (contabilizar quanto de ajuste foi feito)
