#include <QFileDialog>
#include <QMessageBox>

#include "ui_widgetnfe.h"
#include "widgetnfe.h"
#include "xml_viewer.h"

WidgetNfe::WidgetNfe(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetNfe) { ui->setupUi(this); }

WidgetNfe::~WidgetNfe() { delete ui; }

bool WidgetNfe::updateTables(QString &error) {
  switch (ui->tabWidgetNfe->currentIndex()) {
  case 0:
    if (not ui->widgetEntrada->updateTables(error)) return false;
    break;

  case 1:
    if (not ui->widgetSaida->updateTables(error)) return false;
    break;
  }

  return true;
}

void WidgetNfe::on_tabWidgetNfe_currentChanged(const int &) {
  QString error;

  if (not updateTables(error)) QMessageBox::critical(this, "Erro!", error);
}

void WidgetNfe::on_pushButtonExibirXML_clicked() {
  QString xml = QFileDialog::getOpenFileName(this, "Arquivo XML", QDir::currentPath(), "*.xml");

  if (xml.isEmpty()) return;

  QFile file(xml);

  if (not file.open(QFile::ReadOnly)) {
    QMessageBox::critical(this, "Erro!", "Erro abrindo arquivo: " + file.errorString());
    return;
  }

  XML_Viewer *viewer = new XML_Viewer(this);
  viewer->exibirXML(file.readAll());
}
