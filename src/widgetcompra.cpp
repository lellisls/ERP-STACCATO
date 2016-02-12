#include "ui_widgetcompra.h"
#include "widgetcompra.h"

WidgetCompra::WidgetCompra(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompra) { ui->setupUi(this); }

WidgetCompra::~WidgetCompra() { delete ui; }

QString WidgetCompra::updateTables() {
  switch (ui->tabWidget->currentIndex()) {
    case 0:
      ui->widgetPendentes->updateTables();

    case 1:
      ui->widgetGerar->updateTables();

    case 2:
      ui->widgetConfirmar->updateTables();

    case 3:
      ui->widgetFaturar->updateTables();
  }

  return QString();
}

void WidgetCompra::on_tabWidget_currentChanged(const int &) { updateTables(); }
