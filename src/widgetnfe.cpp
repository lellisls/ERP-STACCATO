#include <QFileDialog>
#include <QMessageBox>

#include "ui_widgetnfe.h"
#include "widgetnfe.h"
#include "xml_viewer.h"

WidgetNfe::WidgetNfe(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetNfe) {
  ui->setupUi(this);

  connect(ui->widgetEntrada, &WidgetNfeEntrada::errorSignal, this, &WidgetNfe::errorSignal);
  connect(ui->widgetSaida, &WidgetNfeSaida::errorSignal, this, &WidgetNfe::errorSignal);
}

WidgetNfe::~WidgetNfe() { delete ui; }

bool WidgetNfe::updateTables() {
  const QString currentText = ui->tabWidgetNfe->tabText(ui->tabWidgetNfe->currentIndex());

  if (currentText == "Entrada" and not ui->widgetEntrada->updateTables()) return false;
  if (currentText == "Saída" and not ui->widgetSaida->updateTables()) return false;

  return true;
}

void WidgetNfe::on_tabWidgetNfe_currentChanged(const int) { updateTables(); }
