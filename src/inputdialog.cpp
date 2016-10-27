#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "doubledelegate.h"
#include "inputdialog.h"
#include "noeditdelegate.h"
#include "reaisdelegate.h"
#include "singleeditdelegate.h"
#include "ui_inputdialog.h"
#include "usersession.h"

InputDialog::InputDialog(const Type &type, QWidget *parent) : QDialog(parent), type(type), ui(new Ui::InputDialog) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables();

  ui->frameData->hide();
  ui->frameDataPreco->hide();
  ui->framePagamentos->hide();
  ui->frameFrete->hide();
  ui->checkBoxMarcarTodos->hide();
  ui->groupBoxFinanceiro->hide();
  ui->framePgt1->hide();
  ui->framePgt2->hide();
  ui->framePgt3->hide();
  ui->framePgtTotal->hide();

  ui->dateEditEvento->setDate(QDate::currentDate());
  ui->dateEditProximo->setDate(QDate::currentDate());

  if (type == Carrinho) {
    ui->frameData->show();
    ui->labelEvento->hide();
    ui->dateEditEvento->hide();

    ui->labelProximoEvento->setText("Data prevista compra:");

    connect(&model, &SqlTableModel::dataChanged, this, &InputDialog::updateTableData);
  }

  if (type == GerarCompra) {
    ui->frameData->show();
    ui->frameDataPreco->show();

    ui->labelEvento->setText("Data compra:");
    ui->labelProximoEvento->setText("Data prevista confirmação:");

    connect(&model, &SqlTableModel::dataChanged, this, &InputDialog::updateTableData);
  }

  if (type == ConfirmarCompra or type == Financeiro) {
    ui->dateEditPgt1->setDate(QDate::currentDate());
    ui->dateEditPgt2->setDate(QDate::currentDate());
    ui->dateEditPgt3->setDate(QDate::currentDate());

    connect(ui->comboBoxPgt1Parc, &QComboBox::currentTextChanged, this, &InputDialog::montarFluxoCaixa);
    connect(ui->comboBoxPgt2Parc, &QComboBox::currentTextChanged, this, &InputDialog::montarFluxoCaixa);
    connect(ui->comboBoxPgt3Parc, &QComboBox::currentTextChanged, this, &InputDialog::montarFluxoCaixa);
    connect(ui->dateEditPgt1, &QDateEdit::dateChanged, this, &InputDialog::montarFluxoCaixa);
    connect(ui->dateEditPgt2, &QDateEdit::dateChanged, this, &InputDialog::montarFluxoCaixa);
    connect(ui->dateEditPgt3, &QDateEdit::dateChanged, this, &InputDialog::montarFluxoCaixa);
    connect(ui->lineEditPgt1, &QLineEdit::textChanged, this, &InputDialog::montarFluxoCaixa);
    connect(ui->lineEditPgt2, &QLineEdit::textChanged, this, &InputDialog::montarFluxoCaixa);
    connect(ui->lineEditPgt3, &QLineEdit::textChanged, this, &InputDialog::montarFluxoCaixa);
    connect(ui->comboBoxData1, &QComboBox::currentTextChanged, this, &InputDialog::montarFluxoCaixa);
    connect(ui->comboBoxData2, &QComboBox::currentTextChanged, this, &InputDialog::montarFluxoCaixa);
    connect(ui->comboBoxData3, &QComboBox::currentTextChanged, this, &InputDialog::montarFluxoCaixa);
  }

  if (type == ConfirmarCompra) {
    ui->frameData->show();
    ui->frameDataPreco->show();
    ui->framePagamentos->show();
    ui->frameFrete->show();
    ui->checkBoxMarcarTodos->show();
    ui->framePgt1->show();
    ui->framePgt2->show();
    ui->framePgt3->show();
    ui->framePgtTotal->show();

    ui->labelEvento->setText("Data confirmação:");
    ui->labelProximoEvento->setText("Data prevista faturamento:");

    ui->frameTotal->hide();

    connect(&model, &SqlTableModel::dataChanged, this, &InputDialog::updateTableData);
    connect(ui->table->selectionModel(), &QItemSelectionModel::selectionChanged, this, &InputDialog::calcularTotal);
    connect(ui->doubleSpinBoxTotal, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            &InputDialog::resetarPagamentos);
    connect(ui->doubleSpinBoxTotalPag, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            &InputDialog::resetarPagamentos);
  }

  if (type == Faturamento) {
    ui->frameData->show();
    ui->frameDataPreco->show();
    ui->labelProximoEvento->hide();
    ui->dateEditProximo->hide();

    ui->labelEvento->setText("Data faturamento:");
  }

  if (type == AgendarColeta) {
    ui->frameData->show();
    ui->labelEvento->hide();
    ui->dateEditEvento->hide();
    ui->labelProximoEvento->show();
    ui->dateEditProximo->show();

    ui->labelProximoEvento->setText("Data prevista coleta");
    ui->dateEditProximo->setDate(QDate::currentDate().addDays(8));
  }

  if (type == Coleta) {
    ui->frameData->show();

    ui->labelEvento->setText("Data coleta:");
    ui->labelProximoEvento->setText("Data prevista recebimento:");
    // TODO: colocar dias a mais por padrao?
  }

  if (type == Recebimento) {
    ui->frameData->show();
    ui->labelProximoEvento->hide();
    ui->dateEditProximo->hide();

    ui->labelEvento->setText("Data do recebimento:");
    //    ui->labelProximoEvento->setText("Data prevista entrega:");
  }

  if (type == AgendarEntrega) {
    ui->frameData->show();
    ui->labelEvento->hide();
    ui->dateEditEvento->hide();
    ui->labelProximoEvento->show();
    ui->dateEditProximo->show();

    ui->labelProximoEvento->setText("Data prevista entrega:");
  }

  if (type == Entrega) {
    ui->frameData->show();
    ui->labelProximoEvento->hide();
    ui->dateEditProximo->hide();

    ui->labelEvento->setText("Data entrega:");
  }

  if (type == Financeiro) {
    ui->framePagamentos->show();
    ui->groupBoxFinanceiro->show();

    ui->dateEditPgt1->setDate(QDate::currentDate());
    ui->dateEditPgt2->setDate(QDate::currentDate());
    ui->dateEditPgt3->setDate(QDate::currentDate());
  }

  connect(&model, &QAbstractItemModel::dataChanged, this, &InputDialog::calcularTotal);

  ui->splitter->setStretchFactor(0, 1);
  ui->splitter->setStretchFactor(1, 0);

  adjustSize();

  (type == GerarCompra or type == ConfirmarCompra) ? showMaximized() : show();
}

InputDialog::~InputDialog() { delete ui; }

QDate InputDialog::getDate() { return ui->dateEditEvento->date(); }

QDate InputDialog::getNextDate() { return ui->dateEditProximo->date(); }

bool InputDialog::cadastrar() {
  if (type == ConfirmarCompra) {
    if (not verifyFields()) return false;

    const auto list = ui->table->selectionModel()->selectedRows();

    for (auto const &item : list) model.setData(item.row(), "selecionado", true);
  }

  if (type == Financeiro) {
    if (not model.setData(0, "statusFinanceiro", ui->comboBoxFinanceiro->currentText())) {
      QMessageBox::critical(this, "Erro!", "Erro salvando status na tabela: " + model.lastError().text());
      return false;
    }
  }

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados na tabela: " + model.lastError().text());
    return false;
  }

  if (type == ConfirmarCompra or type == Financeiro) {
    if (not modelFluxoCaixa.submitAll()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando dados do pagamento: " + modelFluxoCaixa.lastError().text());
      return false;
    }
  }

  return true;
}

void InputDialog::on_pushButtonSalvar_clicked() {
  if (not cadastrar()) return;

  QDialog::accept();
  close();
}

bool InputDialog::verifyFields() {
  if (ui->table->selectionModel()->selectedRows().size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
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

  return true;
}

void InputDialog::on_dateEditEvento_dateChanged(const QDate &date) {
  if (ui->dateEditProximo->date() < date) ui->dateEditProximo->setDate(date);
}

bool InputDialog::setFilter(const QStringList &ids) {
  if (ids.isEmpty()) {
    model.setFilter("0");
    QMessageBox::critical(this, "Erro!", "IdsCompra vazio!");
    return false;
  }

  QString filter;

  if (type == GerarCompra) filter = "idPedido = " + ids.join(" OR idPedido = ");
  if (type == Faturamento) filter = "idCompra = " + ids.join(" OR idCompra = ");

  if (filter.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Filtro vazio!");
    return false;
  }

  model.setFilter(filter);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return false;
  }

  ui->table->resizeColumnsToContents();

  double total = 0;

  for (int row = 0; row < model.rowCount(); ++row) total += model.data(row, "preco").toDouble();

  ui->doubleSpinBoxTotal->setValue(total);

  if (type == GerarCompra or type == ConfirmarCompra) {
    QMessageBox::information(this, "Aviso!", "Ajustar preço e quantidade se necessário.");
  }

  return true;
}

bool InputDialog::setFilter(const QString &id) {
  if (id.isEmpty()) {
    model.setFilter("idPedido = 0");
    QMessageBox::critical(this, "Erro!", "IdCompra vazio!");
    return false;
  }

  model.setFilter("idCompra = " + id + " AND status = 'EM COMPRA'");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return false;
  }

  ui->table->resizeColumnsToContents();

  double total = 0;

  for (int row = 0; row < model.rowCount(); ++row) total += model.data(row, "preco").toDouble();

  ui->doubleSpinBoxTotal->setValue(total);

  if (type == ConfirmarCompra or type == Financeiro) {
    if (type == Financeiro) model.setFilter("idCompra = " + id);

    modelFluxoCaixa.setFilter(type == ConfirmarCompra ? "0" : "idCompra = " + id);

    if (not modelFluxoCaixa.select()) {
      QMessageBox::critical(this, "Erro!",
                            "Erro lendo tabela conta_a_pagar_has_pagamento: " + modelFluxoCaixa.lastError().text());
      return false;
    }

    calcularTotal();

    //--------------------------------

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

    ui->tableFluxoCaixa->resizeColumnsToContents();

    montarFluxoCaixa();
  }

  if (type != Faturamento) QMessageBox::information(this, "Aviso!", "Ajustar preço e quantidade se necessário.");

  setWindowTitle("Compra: " + id);

  return true;
}

void InputDialog::setupTables() {
  model.setTable("pedido_fornecedor_has_produto");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);
  //  model.setHeaderData("selecionado", "");
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

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return;
  }

  ui->table->setModel(&model);
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

  if (type == GerarCompra or type == ConfirmarCompra) {
    ui->table->setItemDelegateForColumn("quant", new SingleEditDelegate(this));
    ui->table->setItemDelegateForColumn("prcUnitario", new ReaisDelegate(this));
    ui->table->setItemDelegateForColumn("preco", new ReaisDelegate(this));
  }

  if (type == ConfirmarCompra or type == Financeiro) {
    modelFluxoCaixa.setTable("conta_a_pagar_has_pagamento");
    modelFluxoCaixa.setEditStrategy(QSqlTableModel::OnManualSubmit);
    modelFluxoCaixa.setHeaderData("tipo", "Tipo");
    modelFluxoCaixa.setHeaderData("parcela", "Parcela");
    modelFluxoCaixa.setHeaderData("valor", "R$");
    modelFluxoCaixa.setHeaderData("dataPagamento", "Data");
    modelFluxoCaixa.setHeaderData("observacao", "Obs.");
    modelFluxoCaixa.setHeaderData("status", "Status");

    if (not modelFluxoCaixa.select()) {
      QMessageBox::critical(this, "Erro!",
                            "Erro lendo tabela conta_a_receber_has_pagamento: " + modelFluxoCaixa.lastError().text());
      return;
    }

    ui->tableFluxoCaixa->setModel(&modelFluxoCaixa);
    ui->tableFluxoCaixa->setItemDelegate(new DoubleDelegate(this));
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
  }
}

void InputDialog::on_doubleSpinBoxPgt1_valueChanged(double) {
  calculoSpinBox1();
  montarFluxoCaixa();
}

void InputDialog::on_doubleSpinBoxPgt2_valueChanged(double) {
  calculoSpinBox2();
  montarFluxoCaixa();
}

void InputDialog::on_doubleSpinBoxPgt3_valueChanged(double) { montarFluxoCaixa(); }

void InputDialog::on_comboBoxPgt1_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") return;

  QSqlQuery query;
  query.prepare("SELECT parcelas FROM forma_pagamento WHERE pagamento = :pagamento"); // TODO: use id?
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

void InputDialog::on_comboBoxPgt2_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") return;

  QSqlQuery query;
  query.prepare("SELECT parcelas FROM forma_pagamento WHERE pagamento = :pagamento"); // TODO: use id?
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

void InputDialog::on_comboBoxPgt3_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") return;

  QSqlQuery query;
  query.prepare("SELECT parcelas FROM forma_pagamento WHERE pagamento = :pagamento"); // TODO: use id?
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

void InputDialog::calculoSpinBox1() const {
  const double pgt1 = ui->doubleSpinBoxPgt1->value();
  const double pgt2 = ui->doubleSpinBoxPgt2->value();
  const double pgt3 = ui->doubleSpinBoxPgt3->value();
  const double total = ui->doubleSpinBoxTotalPag->value();
  const double restante = total - (pgt1 + pgt2 + pgt3);

  if (restante == 0.) return;

  ui->doubleSpinBoxPgt2->setValue(pgt2 + restante);
  ui->doubleSpinBoxPgt2->setEnabled(true);
  ui->comboBoxPgt2->setEnabled(true);
  ui->comboBoxData2->setEnabled(true);

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

void InputDialog::calculoSpinBox2() const {
  const double pgt1 = ui->doubleSpinBoxPgt1->value();
  const double pgt2 = ui->doubleSpinBoxPgt2->value();
  const double pgt3 = ui->doubleSpinBoxPgt3->value();
  const double total = ui->doubleSpinBoxTotalPag->value();
  const double restante = total - (pgt1 + pgt2 + pgt3);

  if (restante == 0.) return;

  ui->doubleSpinBoxPgt3->setMaximum(restante + pgt3);
  ui->doubleSpinBoxPgt3->setValue(restante + pgt3);

  ui->doubleSpinBoxPgt3->setEnabled(true);
  ui->comboBoxPgt3->setEnabled(true);
  ui->comboBoxData3->setEnabled(true);
}

void InputDialog::montarFluxoCaixa() {
  modelFluxoCaixa.select();

  if (type == Financeiro) {
    for (int row = 0; row < modelFluxoCaixa.rowCount(); ++row) {
      if (not modelFluxoCaixa.setData(row, "status", "CANCELADO")) {
        QMessageBox::critical(this, "Erro!", "Erro mudando status para 'CANCELADO'!");
        return;
      }
    }
  }

  const QList<QComboBox *> comboParc({ui->comboBoxPgt1Parc, ui->comboBoxPgt2Parc, ui->comboBoxPgt3Parc});
  const QList<QComboBox *> comboPgt({ui->comboBoxPgt1, ui->comboBoxPgt2, ui->comboBoxPgt3});
  const QList<QDateEdit *> datePgt({ui->dateEditPgt1, ui->dateEditPgt2, ui->dateEditPgt3});
  const QList<QComboBox *> datePgt2({ui->comboBoxData1, ui->comboBoxData2, ui->comboBoxData3});
  const QList<QDoubleSpinBox *> spinPgt({ui->doubleSpinBoxPgt1, ui->doubleSpinBoxPgt2, ui->doubleSpinBoxPgt3});
  const QList<QLineEdit *> linePgt({ui->lineEditPgt1, ui->lineEditPgt2, ui->lineEditPgt3});

  for (int i = 0; i < 3; ++i) {
    if (comboPgt.at(i)->currentText() != "Escolha uma opção!") {
      const int parcelas = comboParc.at(i)->currentIndex() + 1;
      const double valor = spinPgt.at(i)->value();
      const float temp = static_cast<float>(static_cast<int>(static_cast<float>(valor / parcelas) * 100)) / 100;
      const double resto = static_cast<double>(valor - (temp * parcelas));
      const double parcela = static_cast<double>(temp);

      for (int x = 0, y = parcelas - 1; x < parcelas; ++x, --y) {
        const int row = modelFluxoCaixa.rowCount();
        modelFluxoCaixa.insertRow(row);
        modelFluxoCaixa.setData(row, "contraParte", model.data(0, "fornecedor"));
        modelFluxoCaixa.setData(row, "dataEmissao", model.data(0, "dataRealConf"));
        modelFluxoCaixa.setData(row, "idCompra", model.data(0, "idCompra"));
        modelFluxoCaixa.setData(row, "idLoja", UserSession::idLoja());
        const QDate dataPgt =
            (datePgt2.at(i)->currentText() == "Data + 1 Mês"
                 ? datePgt.at(i)->date().addMonths(x + 1)
                 : datePgt2.at(i)->currentText() == "Data Mês"
                       ? datePgt.at(i)->date().addMonths(x)
                       : datePgt.at(i)->date().addDays(datePgt2.at(i)->currentText().toInt() * (x + 1)));

        modelFluxoCaixa.setData(row, "dataPagamento", dataPgt);
        modelFluxoCaixa.setData(row, "valor", parcela + (x == 0 ? resto : 0));
        modelFluxoCaixa.setData(row, "tipo", QString::number(i + 1) + ". " + comboPgt.at(i)->currentText());
        modelFluxoCaixa.setData(row, "parcela", parcelas - y);
        modelFluxoCaixa.setData(row, "observacao", linePgt.at(i)->text());
      }
    }
  }

  ui->tableFluxoCaixa->resizeColumnsToContents();
}

void InputDialog::resetarPagamentos() {
  ui->doubleSpinBoxPgt1->setMaximum(ui->doubleSpinBoxTotalPag->value());
  ui->doubleSpinBoxPgt1->setValue(ui->doubleSpinBoxTotalPag->value());
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

  ui->comboBoxData2->setDisabled(true);
  ui->comboBoxData3->setDisabled(true);

  ui->doubleSpinBoxPgt2->setDisabled(true);
  ui->doubleSpinBoxPgt3->setDisabled(true);

  montarFluxoCaixa();
}

void InputDialog::on_pushButtonLimparPag_clicked() { resetarPagamentos(); }

void InputDialog::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void InputDialog::on_tableFluxoCaixa_entered(const QModelIndex &) { ui->tableFluxoCaixa->resizeColumnsToContents(); }

void InputDialog::updateTableData(const QModelIndex &topLeft) {
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

void InputDialog::calcularTotal() {
  double total = 0;

  const auto list = ui->table->selectionModel()->selectedRows();

  for (auto const &item : list) total += model.data(item.row(), "preco").toDouble();

  total -= ui->doubleSpinBoxAdicionais->value();
  total += ui->doubleSpinBoxFrete->value();

  ui->doubleSpinBoxTotalPag->setValue(total);
  ui->doubleSpinBoxTotal->setValue(total);
}

void InputDialog::on_doubleSpinBoxFrete_valueChanged(double) { calcularTotal(); }

void InputDialog::on_doubleSpinBoxAdicionais_valueChanged(double) { calcularTotal(); }

void InputDialog::on_checkBoxMarcarTodos_toggled(bool checked) {
  checked ? ui->table->selectAll() : ui->table->clearSelection();
}

void InputDialog::on_pushButtonCorrigirFluxo_clicked() {
  ui->framePgt1->show();
  ui->framePgt2->show();
  ui->framePgt3->show();
  ui->framePgtTotal->show();

  //

  calcularTotal();
  resetarPagamentos();
}

// TODO: na parte de compra quando for representacao puxar o preco de venda e nao o custo
// TODO: na representacao colocar contas a receber e calculcar comissao 30 dias da data de vencimento do
// boleto/pagamento
// TODO: frete aparece separado no fluxo (criar linha propria)
