#include "widgetfinanceiro.h"
#include "ui_widgetfinanceiro.h"

WidgetFinanceiro::WidgetFinanceiro(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetFinanceiro) {
  ui->setupUi(this);

  ui->widgetPagar->setTipo(WidgetPagamento::Pagar);
  ui->widgetReceber->setTipo(WidgetPagamento::Receber);
  ui->widgetVenda->setFinanceiro();

  connect(ui->widgetFluxoCaixa, &WidgetFluxoCaixa::errorSignal, this, &WidgetFinanceiro::errorSignal);
  connect(ui->widgetPagar, &WidgetPagamento::errorSignal, this, &WidgetFinanceiro::errorSignal);
  connect(ui->widgetReceber, &WidgetPagamento::errorSignal, this, &WidgetFinanceiro::errorSignal);
  connect(ui->widgetVenda, &WidgetVenda::errorSignal, this, &WidgetFinanceiro::errorSignal);

  connect(ui->tabWidget, &QTabWidget::currentChanged, this, &WidgetFinanceiro::updateTables);
}

WidgetFinanceiro::~WidgetFinanceiro() { delete ui; }

bool WidgetFinanceiro::updateTables() {
  const QString currentText = ui->tabWidget->tabText(ui->tabWidget->currentIndex());

  if (currentText == "Fluxo de Caixa") return ui->widgetFluxoCaixa->updateTables();
  if (currentText == "Contas a Pagar") return ui->widgetPagar->updateTables();
  if (currentText == "Contas a Receber") return ui->widgetReceber->updateTables();
  if (currentText == "Receber Resumo") return ui->widgetReceberResumo->updateTables();
  if (currentText == "Vendas") return ui->widgetVenda->updateTables();
  if (currentText == "Compras") return ui->widgetCompra->updateTables();

  return true;
}
