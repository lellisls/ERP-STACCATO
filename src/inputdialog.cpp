#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "checkboxdelegate.h"
#include "doubledelegate.h"
#include "inputdialog.h"
#include "ui_inputdialog.h"
#include "usersession.h"

InputDialog::InputDialog(const Type &type, QWidget *parent) : QDialog(parent), type(type), ui(new Ui::InputDialog) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables();

  ui->groupBoxData->hide();
  ui->groupBoxDataPreco->hide();
  ui->framePagamentos->hide();

  ui->dateEditEvento->setDate(QDate::currentDate());
  ui->dateEditProximo->setDate(QDate::currentDate());

  if (type == Carrinho) {
    ui->groupBoxData->show();
    ui->labelEvento->hide();
    ui->dateEditEvento->hide();

    ui->labelProximoEvento->setText("Data prevista compra:");
  }

  if (type == GerarCompra) {
    ui->groupBoxData->show();
    ui->groupBoxDataPreco->show();

    ui->labelEvento->setText("Data compra:");
    ui->labelProximoEvento->setText("Data prevista confirmação:");
  }

  if (type == ConfirmarCompra) {
    ui->groupBoxData->show();
    ui->groupBoxDataPreco->show();
    ui->framePagamentos->show();

    ui->labelEvento->setText("Data confirmação:");
    ui->labelProximoEvento->setText("Data prevista faturamento:");

    // NOTE: make this runtime changeable
    QStringList list{"Escolha uma opção!", "Cartão de débito", "Cartão de crédito", "Cheque",
                     "Dinheiro",           "Boleto",           "Transf. Banc."};

    ui->comboBoxPgt1->insertItems(0, list);
    ui->comboBoxPgt2->insertItems(0, list);
    ui->comboBoxPgt3->insertItems(0, list);

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

    // NOTE: readd this later
    //    ui->tableView->showColumn(model.fieldIndex("selecionado"));

    //    for (int row = 0; row < model.rowCount(); ++row) {
    //      ui->tableView->openPersistentEditor(model.index(row, model.fieldIndex("selecionado")));
    //    }
  }

  if (type == Faturamento) {
    ui->groupBoxData->show();

    ui->labelEvento->setText("Data faturamento:");
    ui->labelProximoEvento->setText("Data prevista coleta:");
  }

  if (type == Coleta) {
    ui->groupBoxData->show();

    ui->labelEvento->setText("Data coleta:");
    ui->labelProximoEvento->setText("Data prevista recebimento:");
  }

  if (type == Recebimento) {
    ui->groupBoxData->show();

    ui->labelEvento->setText("Data do recebimento:");
    ui->labelProximoEvento->setText("Data prevista entrega:");
  }

  if (type == Entrega) {
    ui->groupBoxData->show();
    ui->labelProximoEvento->hide();
    ui->dateEditProximo->hide();

    ui->labelEvento->setText("Data entrega:");
  }

  adjustSize();
  show();
}

InputDialog::~InputDialog() { delete ui; }

QDate InputDialog::getDate() { return ui->dateEditEvento->date(); }

QDate InputDialog::getNextDate() { return ui->dateEditProximo->date(); }

void InputDialog::on_pushButtonSalvar_clicked() {
  if (type == ConfirmarCompra) {
    // NOTE: confirmar apenas os que estiverem marcados

    if (not qFuzzyCompare(ui->doubleSpinBoxPgt1->value() + ui->doubleSpinBoxPgt2->value() +
                          ui->doubleSpinBoxPgt3->value(),
                          ui->doubleSpinBoxTotalPag->value())) {
      QMessageBox::critical(this, "Erro!", "Soma dos pagamentos não é igual ao total! Favor verificar.");
      return;
    }

    if (ui->doubleSpinBoxPgt1->value() > 0 and ui->comboBoxPgt1->currentText() == "Escolha uma opção!") {
      QMessageBox::critical(this, "Erro!", "Por favor escolha a forma de pagamento 1.");
      ui->doubleSpinBoxPgt1->setFocus();
      return;
    }

    if (ui->doubleSpinBoxPgt2->value() > 0 and ui->comboBoxPgt2->currentText() == "Escolha uma opção!") {
      QMessageBox::critical(this, "Erro!", "Por favor escolha a forma de pagamento 2.");
      ui->doubleSpinBoxPgt2->setFocus();
      return;
    }

    if (ui->doubleSpinBoxPgt3->value() > 0 and ui->comboBoxPgt3->currentText() == "Escolha uma opção!") {
      QMessageBox::critical(this, "Erro!", "Por favor escolha a forma de pagamento 3.");
      ui->doubleSpinBoxPgt3->setFocus();
      return;
    }
  }

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados na tabela: " + model.lastError().text());
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  if (type == ConfirmarCompra) {
    if (not modelFluxoCaixa.submitAll()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando dados do pagamento: " + modelFluxoCaixa.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }
  }

  QSqlQuery("COMMIT").exec();

  QDialog::accept();
  close();
}

void InputDialog::on_dateEditEvento_dateChanged(const QDate &date) {
  if (ui->dateEditProximo->date() < date) ui->dateEditProximo->setDate(date);
}

void InputDialog::setFilter(const QStringList &ids) {
  if (ids.isEmpty() or ids.first().isEmpty()) {
    model.setFilter("idPedido = 0");
    QMessageBox::critical(this, "Erro!", "ids vazio!");
    return;
  }

  const QString filter =
      type == ConfirmarCompra ? "idCompra = " + ids.first() : "idPedido = " + ids.join(" OR idPedido = ");

  model.setFilter(filter);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return;
  }

  ui->table->resizeColumnsToContents();

  for (int row = 0; row < model.rowCount(); ++row) {
    ui->table->openPersistentEditor(row, "selecionado");
  }

  QMessageBox::information(this, "Aviso!", "Ajustar preço e quantidade se necessário.");
}

void InputDialog::setFilter(const QString &id) {
  if (id.isEmpty()) {
    model.setFilter("idPedido = 0");
    QMessageBox::critical(this, "Erro!", "ids vazio!");
    return;
  }

  model.setFilter("idCompra = " + id);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return;
  }

  ui->table->resizeColumnsToContents();

  for (int row = 0; row < model.rowCount(); ++row) {
    ui->table->openPersistentEditor(row, "selecionado");
  }

  if (type == ConfirmarCompra) {
    modelFluxoCaixa.setFilter("idCompra = " + id);

    if (not modelFluxoCaixa.select()) {
      QMessageBox::critical(this, "Erro!",
                            "Erro lendo tabela conta_a_pagar_has_pagamento: " + modelFluxoCaixa.lastError().text());
      return;
    }

    //--------------------------------
    double preco = 0;

    for (int row = 0; row < model.rowCount(); ++row) {
      preco += model.data(row, "preco").toDouble();
    }

    ui->doubleSpinBoxTotalPag->setValue(preco);

    resetarPagamentos();

    //--------------------------------
  }

  QMessageBox::information(this, "Aviso!", "Ajustar preço e quantidade se necessário.");
}

void InputDialog::setupTables() {
  model.setTable("pedido_fornecedor_has_produto");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);
  model.setHeaderData("selecionado", "");
  model.setHeaderData("fornecedor", "Fornecedor");
  model.setHeaderData("descricao", "Produto");
  model.setHeaderData("colecao", "Coleção");
  model.setHeaderData("caixas", "Caixas");
  model.setHeaderData("quant", "Quant.");
  model.setHeaderData("un", "Un.");
  model.setHeaderData("un2", "Un.2");
  model.setHeaderData("kgcx", "Kg./Cx.");
  model.setHeaderData("formComercial", "Formato");
  model.setHeaderData("codComercial", "Código");
  model.setHeaderData("preco", "Preço");
  model.setHeaderData("obs", "Obs.");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return;
  }

  ui->table->setModel(&model);
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
  ui->table->setItemDelegateForColumn("selecionado", new CheckBoxDelegate(this));

  if (type == ConfirmarCompra) {
    modelFluxoCaixa.setTable("conta_a_pagar_has_pagamento");
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
    ui->tableFluxoCaixa->setItemDelegate(new DoubleDelegate(this));
    ui->tableFluxoCaixa->setItemDelegateForColumn(modelFluxoCaixa.fieldIndex("representacao"),
                                                  new CheckBoxDelegate(this));
  }
}

void InputDialog::on_doubleSpinBoxPgt1_editingFinished() {
  calculoSpinBox1();
  montarFluxoCaixa();
}

void InputDialog::on_doubleSpinBoxPgt2_editingFinished() {
  calculoSpinBox2();
  montarFluxoCaixa();
}

void InputDialog::on_doubleSpinBoxPgt3_editingFinished() { montarFluxoCaixa(); }

void InputDialog::on_comboBoxPgt1_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") return;

  ui->comboBoxPgt1Parc->setEnabled((text == "Cartão de crédito" or text == "Cheque" or text == "Boleto") ? true
                                                                                                         : false);

  ui->dateEditPgt1->setEnabled(true);

  montarFluxoCaixa();
}

void InputDialog::on_comboBoxPgt2_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") return;

  ui->comboBoxPgt2Parc->setEnabled((text == "Cartão de crédito" or text == "Cheque" or text == "Boleto") ? true
                                                                                                         : false);

  ui->dateEditPgt2->setEnabled(true);

  montarFluxoCaixa();
}

void InputDialog::on_comboBoxPgt3_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") return;

  ui->comboBoxPgt3Parc->setEnabled((text == "Cartão de crédito" or text == "Cheque" or text == "Boleto") ? true
                                                                                                         : false);

  ui->dateEditPgt3->setEnabled(true);

  montarFluxoCaixa();
}

void InputDialog::calculoSpinBox1() const {
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

void InputDialog::calculoSpinBox2() const {
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

void InputDialog::montarFluxoCaixa() {
  modelFluxoCaixa.removeRows(0, modelFluxoCaixa.rowCount());

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
        modelFluxoCaixa.setData(row, "idCompra", model.data(0, "idCompra"));
        modelFluxoCaixa.setData(row, "idLoja", UserSession::idLoja());
        modelFluxoCaixa.setData(row, "dataPagamento", datePgt.at(i)->date().addMonths(x + temp2));
        modelFluxoCaixa.setData(row, "valor", parcela + (x == 0 ? resto : 0));
        modelFluxoCaixa.setData(row, "tipo", QString::number(i + 1) + ". " + comboPgt.at(i)->currentText());
        modelFluxoCaixa.setData(row, "parcela", parcelas - y);
        modelFluxoCaixa.setData(row, "observacao", linePgt.at(i)->text());
      }
    }
  }

  for (int row = 0; row < modelFluxoCaixa.rowCount(); ++row) {
    ui->tableFluxoCaixa->openPersistentEditor(row, "representacao");
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

  ui->doubleSpinBoxPgt2->setDisabled(true);
  ui->doubleSpinBoxPgt3->setDisabled(true);

  montarFluxoCaixa();
}

void InputDialog::on_pushButtonLimparPag_clicked() { resetarPagamentos(); }
