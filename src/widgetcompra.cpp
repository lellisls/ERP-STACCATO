#include "widgetcompra.h"
#include "ui_widgetcompra.h"

#include <QMessageBox>

WidgetCompra::WidgetCompra(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompra) { ui->setupUi(this); }

WidgetCompra::~WidgetCompra() { delete ui; }

bool WidgetCompra::updateTables(QString &error) {
  switch (ui->tabWidget->currentIndex()) {
  case 0:
    if (not ui->widgetDevolucao->updateTables(error)) return false;
    break;

  case 1:
    if (not ui->widgetPendentes->updateTables(error)) return false;
    break;

  case 2:
    if (not ui->widgetGerar->updateTables(error)) return false;
    break;

  case 3:
    if (not ui->widgetConfirmar->updateTables(error)) return false;
    break;

  case 4:
    if (not ui->widgetFaturar->updateTables(error)) return false;
    break;
  }

  return true;
}

void WidgetCompra::on_tabWidget_currentChanged(const int &) {
  QString error;

  if (not updateTables(error)) QMessageBox::critical(this, "Erro!", error);
}
