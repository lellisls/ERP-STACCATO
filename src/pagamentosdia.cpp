#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

#include "pagamentosdia.h"
#include "reaisdelegate.h"
#include "ui_pagamentosdia.h"

PagamentosDia::PagamentosDia(QWidget *parent) : QDialog(parent), ui(new Ui::PagamentosDia) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables();
}

PagamentosDia::~PagamentosDia() { delete ui; }

void PagamentosDia::setupTables() {
  model.setTable("view_fluxo_caixa");

  ui->tableView->setModel(&model);

  // TODO: usar outra coluna no lugar de idCompra?
  // TODO: renomear/esconder colunas de data
  // TODO: colocar delegates

  ui->tableView->hideColumn("contaDestino");
  ui->tableView->hideColumn("idConta");
  ui->tableView->hideColumn("Data Pag");
  ui->tableView->hideColumn("Data");

  ui->tableView->setItemDelegateForColumn("R$", new ReaisDelegate(this));
}

bool PagamentosDia::setFilter(const QDate &date, const QString &idConta) {
  const QString filtroConta = idConta.isEmpty() ? "" : "AND idConta = " + idConta;

  model.setFilter("`Data` = '" + date.toString("yyyy-MM-dd") + "' AND (status = 'PAGO' OR status = 'RECEBIDO') " + filtroConta);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
    return false;
  }

  setWindowTitle(date.toString("dd-MM-yyyy"));

  return true;
}
