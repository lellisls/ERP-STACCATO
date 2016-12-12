#include "widgetcompra.h"
#include "ui_widgetcompra.h"

WidgetCompra::WidgetCompra(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompra) {
  ui->setupUi(this);

  connect(ui->widgetOC, &WidgetCompraOC::errorSignal, this, &WidgetCompra::errorSignal);
  connect(ui->widgetDevolucao, &WidgetCompraDevolucao::errorSignal, this, &WidgetCompra::errorSignal);
  connect(ui->widgetPendentes, &WidgetCompraPendentes::errorSignal, this, &WidgetCompra::errorSignal);
  connect(ui->widgetGerar, &WidgetCompraGerar::errorSignal, this, &WidgetCompra::errorSignal);
  connect(ui->widgetConfirmar, &WidgetCompraConfirmar::errorSignal, this, &WidgetCompra::errorSignal);
  connect(ui->widgetFaturar, &WidgetCompraFaturar::errorSignal, this, &WidgetCompra::errorSignal);
}

WidgetCompra::~WidgetCompra() { delete ui; }

bool WidgetCompra::updateTables() {
  const QString currentText = ui->tabWidget->tabText(ui->tabWidget->currentIndex());

  if (currentText == "Devoluções" and not ui->widgetDevolucao->updateTables()) return false;
  if (currentText == "Pendentes" and not ui->widgetPendentes->updateTables()) return false;
  if (currentText == "Gerar Compra" and not ui->widgetGerar->updateTables()) return false;
  if (currentText == "Confirmar Compra" and not ui->widgetConfirmar->updateTables()) return false;
  if (currentText == "Faturamento" and not ui->widgetFaturar->updateTables()) return false;
  if (currentText == "Compras" and not ui->widgetOC->updateTables()) return false;

  return true;
}

void WidgetCompra::on_tabWidget_currentChanged(const int &) { updateTables(); }
