#include "widgetcompra.h"
#include "ui_widgetcompra.h"

WidgetCompra::WidgetCompra(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompra) { ui->setupUi(this); }

WidgetCompra::~WidgetCompra() { delete ui; }

bool WidgetCompra::updateTables() {
  connect(ui->widgetDevolucao, &WidgetCompraDevolucao::errorSignal, this, &WidgetCompra::errorSignal);
  connect(ui->widgetPendentes, &WidgetCompraPendentes::errorSignal, this, &WidgetCompra::errorSignal);
  connect(ui->widgetGerar, &WidgetCompraGerar::errorSignal, this, &WidgetCompra::errorSignal);
  connect(ui->widgetConfirmar, &WidgetCompraConfirmar::errorSignal, this, &WidgetCompra::errorSignal);
  connect(ui->widgetFaturar, &WidgetCompraFaturar::errorSignal, this, &WidgetCompra::errorSignal);

  switch (ui->tabWidget->currentIndex()) {
  case 0:
    if (not ui->widgetDevolucao->updateTables()) return false;
    break;

  case 1:
    if (not ui->widgetPendentes->updateTables()) return false;
    break;

  case 2:
    if (not ui->widgetGerar->updateTables()) return false;
    break;

  case 3:
    if (not ui->widgetConfirmar->updateTables()) return false;
    break;

  case 4:
    if (not ui->widgetFaturar->updateTables()) return false;
    break;
  }

  return true;
}

void WidgetCompra::on_tabWidget_currentChanged(const int &) { updateTables(); }
