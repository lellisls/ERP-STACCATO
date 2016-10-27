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
  switch (ui->tabWidget->currentIndex()) {
  case 0:
    return ui->widgetFluxoCaixa->updateTables();
  case 1:
    return ui->widgetPagar->updateTables();
  case 2:
    return ui->widgetReceber->updateTables();
  case 3:
    return ui->widgetReceberResumo->updateTables();
  case 4:
    return ui->widgetVenda->updateTables();
  case 5:
    return ui->widgetCompra->updateTables();
  default:
    return true;
  }
}
