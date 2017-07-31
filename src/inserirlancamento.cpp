#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

#include "checkboxdelegate.h"
#include "comboboxdelegate.h"
#include "dateformatdelegate.h"
#include "doubledelegate.h"
#include "inserirlancamento.h"
#include "itembox.h"
#include "itemboxdelegate.h"
#include "lineeditdelegate.h"
#include "reaisdelegate.h"
#include "ui_inserirlancamento.h"

InserirLancamento::InserirLancamento(const Tipo tipo, QWidget *parent) : QDialog(parent), tipo(tipo), ui(new Ui::InserirLancamento) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables();
}

InserirLancamento::~InserirLancamento() { delete ui; }

void InserirLancamento::setupTables() {
  model.setTable(tipo == Pagar ? "conta_a_pagar_has_pagamento" : "conta_a_receber_has_pagamento");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  model.setHeaderData("dataEmissao", "Data Emissão");
  model.setHeaderData("contraParte", "ContraParte");
  model.setHeaderData("idLoja", "Centro Custo");
  model.setHeaderData("valor", "R$");
  model.setHeaderData("tipo", "Tipo");
  model.setHeaderData("dataPagamento", "Vencimento");
  model.setHeaderData("observacao", "Obs.");
  model.setHeaderData("grupo", "Grupo");
  model.setHeaderData("subGrupo", "SubGrupo");

  model.setFilter("0");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela conta_a_receber_has_pagamento: " + model.lastError().text());
  }

  ui->table->setModel(&model);
  ui->table->setItemDelegate(new DoubleDelegate(this));
  ui->table->setItemDelegateForColumn("valor", new ReaisDelegate(this, 2));
  ui->table->setItemDelegateForColumn("tipo", new ComboBoxDelegate(ComboBoxDelegate::Pagamento, this));
  ui->table->setItemDelegateForColumn("status", new ComboBoxDelegate(ComboBoxDelegate::StatusReceber, this));
  ui->table->setItemDelegateForColumn("contaDestino", new ComboBoxDelegate(ComboBoxDelegate::Conta, this));
  ui->table->setItemDelegateForColumn("representacao", new CheckBoxDelegate(this, true));
  ui->table->setItemDelegateForColumn("idLoja", new ItemBoxDelegate(ItemBoxDelegate::Loja, false, this));
  ui->table->setItemDelegateForColumn("grupo", new ComboBoxDelegate(ComboBoxDelegate::Grupo, this));
  ui->table->setItemDelegateForColumn("contraParte", new LineEditDelegate(LineEditDelegate::ContraPartePagar, this));
  ui->table->setItemDelegateForColumn("dataPagamento", new DateFormatDelegate(this));
  // TODO: colocar lineEditDelegate para subgrupo
  ui->table->hideColumn("nfe");
  ui->table->hideColumn("taxa");
  ui->table->hideColumn("parcela");
  ui->table->hideColumn("status");
  ui->table->hideColumn("representacao");
  ui->table->hideColumn("dataRealizado");
  ui->table->hideColumn("valorReal");
  ui->table->hideColumn("tipoReal");
  ui->table->hideColumn("parcelaReal");
  ui->table->hideColumn("contaDestino");
  ui->table->hideColumn("tipoDet");
  ui->table->hideColumn("centroCusto");
  ui->table->hideColumn("comissao");
  ui->table->hideColumn("idPagamento");
  ui->table->hideColumn("idCompra");
  ui->table->hideColumn("idVenda");
  ui->table->hideColumn("created");
  ui->table->hideColumn("lastUpdated");
}

void InserirLancamento::on_pushButtonCriarLancamento_clicked() {
  const int row = model.rowCount();
  model.insertRow(row);

  model.setData(row, "status", "PENDENTE");
  model.setData(row, "dataEmissao", QDate::currentDate());

  for (int row = 0; row < model.rowCount(); ++row) {
    ui->table->openPersistentEditor(row, "idLoja");
    ui->table->openPersistentEditor(row, "tipo");
    ui->table->openPersistentEditor(row, "grupo");
  }
}

void InserirLancamento::on_pushButtonSalvar_clicked() {
  if (not verifyFields()) return;

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados: " + model.lastError().text());
    return;
  }

  QMessageBox::information(this, "Aviso!", "Lançamento salvo com sucesso!");
  close();
}

bool InserirLancamento::verifyFields() {
  for (int row = 0; row < model.rowCount(); ++row) {
    if (model.data(row, "dataEmissao").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "Faltou preencher 'Data Emissão' na linha: " + QString::number(row + 1));
      return false;
    }

    if (model.data(row, "idLoja").toInt() == 0) {
      QMessageBox::critical(this, "Erro!", "Faltou preencher 'Centro Custo' na linha: " + QString::number(row + 1));
      return false;
    }

    if (model.data(row, "contraParte").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "Faltou preencher 'ContraParte' na linha: " + QString::number(row + 1));
      return false;
    }

    if (model.data(row, "valor").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "Faltou preencher 'R$' na linha: " + QString::number(row + 1));
      return false;
    }

    if (model.data(row, "tipo").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "Faltou preencher 'Tipo' na linha: " + QString::number(row + 1));
      return false;
    }

    if (model.data(row, "dataPagamento").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "Faltou preencher 'Vencimento' na linha: " + QString::number(row + 1));
      return false;
    }

    if (model.data(row, "grupo").toString().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "Faltou preencher 'Grupo' na linha: " + QString::number(row + 1));
      return false;
    }
  }

  return true;
}
