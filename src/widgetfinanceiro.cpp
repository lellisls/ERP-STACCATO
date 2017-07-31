#include <QDebug>

#include "ui_widgetfinanceiro.h"
#include "widgetfinanceiro.h"

WidgetFinanceiro::WidgetFinanceiro(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetFinanceiro) {
  ui->setupUi(this);

  ui->widgetPagar->setTipo(WidgetPagamento::Tipo::Pagar);
  ui->widgetReceber->setTipo(WidgetPagamento::Tipo::Receber);
  ui->widgetVenda->setFinanceiro();

  connect(ui->widgetFluxoCaixa, &WidgetFluxoCaixa::errorSignal, this, &WidgetFinanceiro::errorSignal);
  connect(ui->widgetPagar, &WidgetPagamento::errorSignal, this, &WidgetFinanceiro::errorSignal);
  connect(ui->widgetReceber, &WidgetPagamento::errorSignal, this, &WidgetFinanceiro::errorSignal);
  connect(ui->widgetVenda, &WidgetVenda::errorSignal, this, &WidgetFinanceiro::errorSignal);
  connect(ui->widgetCompra, &WidgetFinanceiroCompra::errorSignal, this, &WidgetFinanceiro::errorSignal);

  connect(ui->tabWidget, &QTabWidget::currentChanged, this, &WidgetFinanceiro::updateTables);
}

WidgetFinanceiro::~WidgetFinanceiro() { delete ui; }

bool WidgetFinanceiro::updateTables() {
  const QString currentText = ui->tabWidget->tabText(ui->tabWidget->currentIndex());

  if (currentText == "Fluxo de Caixa") return ui->widgetFluxoCaixa->updateTables();
  if (currentText == "Contas a Pagar") return ui->widgetPagar->updateTables();
  if (currentText == "Contas a Receber") return ui->widgetReceber->updateTables();
  if (currentText == "Vendas") return ui->widgetVenda->updateTables();
  if (currentText == "Compras") return ui->widgetCompra->updateTables();

  return true;
}

// TODO: a cada dia colocar em 'maintenance' um job para enviar o relatorio das financas de 3 dias antes

// select cp.dataEmissao, cp.dataRealizado, cp.valorReal, concat(lhc.banco, ' - ', lhc.agencia, ' - ', lhc.conta) AS
// 'Conta', cp.observacao, cp.contraParte, cp.grupo, cp.subGrupo
// from conta_a_pagar_has_pagamento cp
// left join nfe n on cp.nfe = n.idNFe
// left join loja_has_conta lhc on cp.contaDestino = lhc.idConta
// where cp.dataRealizado is not null and cp.valorReal is not null
// order by cp.dataRealizado;

// select cr.dataEmissao, cr.dataRealizado, cr.valorReal, concat(lhc.banco, ' - ', lhc.agencia, ' - ', lhc.conta) AS
// 'Conta', cr.observacao, cr.contraParte, cr.grupo, cr.subGrupo
// from conta_a_receber_has_pagamento cr
// left join loja_has_conta lhc on cr.contaDestino = lhc.idConta
// where cr.valorReal is not null
// order by cr.dataRealizado;
// TODO: poder deixar 'agencia' e 'conta' como nulo nos casos em que nao existem
