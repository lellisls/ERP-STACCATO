#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "inserirtransferencia.h"
#include "ui_inserirtransferencia.h"

InserirTransferencia::InserirTransferencia(QWidget *parent) : QDialog(parent), ui(new Ui::InserirTransferencia) {
  ui->setupUi(this);

  setupTables();

  ui->itemBoxDe->setSearchDialog(SearchDialog::conta(this));
  ui->itemBoxPara->setSearchDialog(SearchDialog::conta(this));

  ui->dateEdit->setDate(QDate::currentDate());
}

InserirTransferencia::~InserirTransferencia() { delete ui; }

void InserirTransferencia::on_pushButtonSalvar_clicked() {
  if (not verifyFields()) return;

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not cadastrar()) {
    QSqlQuery("ROLLBACK").exec();
    if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
    return;
  }

  QSqlQuery("COMMIT").exec();

  QMessageBox::information(this, "Aviso!", "Transferência registrada com sucesso!");
  close();
}

bool InserirTransferencia::cadastrar() {
  // lancamento 'de'

  const int rowDe = modelDe.rowCount();
  modelDe.insertRow(rowDe);

  modelDe.setData(rowDe, "status", "PAGO");
  modelDe.setData(rowDe, "dataEmissao", ui->dateEdit->date());
  modelDe.setData(rowDe, "idLoja", "1"); // Geral
  modelDe.setData(rowDe, "contraParte", "TRANSFERÊNCIA PARA " + ui->itemBoxPara->text());
  modelDe.setData(rowDe, "valor", ui->doubleSpinBoxValor->value());
  modelDe.setData(rowDe, "tipo", "1. Transf. Banc.");
  modelDe.setData(rowDe, "dataPagamento", ui->dateEdit->date());
  modelDe.setData(rowDe, "dataRealizado", ui->dateEdit->date());
  modelDe.setData(rowDe, "valorReal", ui->doubleSpinBoxValor->value());
  modelDe.setData(rowDe, "tipoReal", "1. Transf. Banc.");
  modelDe.setData(rowDe, "parcelaReal", "1");
  modelDe.setData(rowDe, "contaDestino", ui->itemBoxDe->getValue());
  modelDe.setData(rowDe, "centroCusto", 1); // Geral
  modelDe.setData(rowDe, "grupo", "Transferência");
  modelDe.setData(rowDe, "observacao", ui->lineEditObservacao->text());

  // lancamento 'para'

  const int rowPara = modelPara.rowCount();
  modelPara.insertRow(rowPara);

  modelPara.setData(rowPara, "status", "RECEBIDO");
  modelPara.setData(rowPara, "dataEmissao", ui->dateEdit->date());
  modelPara.setData(rowPara, "idLoja", "1"); // Geral
  modelPara.setData(rowPara, "contraParte", "TRANSFERÊNCIA DE " + ui->itemBoxDe->text());
  modelPara.setData(rowPara, "valor", ui->doubleSpinBoxValor->value());
  modelPara.setData(rowPara, "tipo", "1. Transf. Banc.");
  modelPara.setData(rowPara, "dataPagamento", ui->dateEdit->date());
  modelPara.setData(rowPara, "dataRealizado", ui->dateEdit->date());
  modelPara.setData(rowPara, "valorReal", ui->doubleSpinBoxValor->value());
  modelPara.setData(rowPara, "tipoReal", "1. Transf. Banc.");
  modelPara.setData(rowPara, "parcelaReal", "1");
  modelPara.setData(rowPara, "contaDestino", ui->itemBoxPara->getValue());
  modelPara.setData(rowPara, "centroCusto", 1); // Geral
  modelPara.setData(rowPara, "grupo", "Transferência");
  modelPara.setData(rowPara, "observacao", ui->lineEditObservacao->text());

  if (not modelDe.submitAll()) {
    error = "Erro salvando 'De': " + modelDe.lastError().text();
    return false;
  }

  if (not modelPara.submitAll()) {
    error = "Erro salvando 'Para': " + modelPara.lastError().text();
    return false;
  }

  return true;
}

void InserirTransferencia::on_pushButtonCancelar_clicked() { close(); }

bool InserirTransferencia::verifyFields() {
  if (ui->itemBoxDe->text().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Conta 'De' não preenchido!");
    return false;
  }

  if (ui->itemBoxPara->text().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Conta 'Para' não preenchido!");
    return false;
  }

  if (ui->doubleSpinBoxValor->value() == 0.) {
    QMessageBox::critical(this, "Erro!", "Valor não preenchido!");
    return false;
  }

  return true;
}

void InserirTransferencia::setupTables() {
  modelDe.setTable("conta_a_pagar_has_pagamento");
  modelDe.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelDe.setFilter("0");

  if (not modelDe.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela 'De': " + modelDe.lastError().text());
  }

  modelPara.setTable("conta_a_receber_has_pagamento");
  modelPara.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelPara.setFilter("0");

  if (not modelPara.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela 'Para': " + modelPara.lastError().text());
  }
}

// TODO: colocar campo para observacao (na tabela já tem campo observacao, apenas preencher)
// TODO: fazer transferencia especial para conta cliente, fazendo a operacao no credito tambem
