#include <QFileDialog>
#include <QMessageBox>

#include "ui_widgetnfe.h"
#include "widgetnfe.h"
#include "xml_viewer.h"

WidgetNfe::WidgetNfe(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetNfe) { ui->setupUi(this); }

WidgetNfe::~WidgetNfe() { delete ui; }

QString WidgetNfe::updateTables() {
  switch (ui->tabWidgetNfe->currentIndex()) {
    case 0: {
        return ui->widgetEntrada->updateTables();
      }

    case 1: {
        return ui->widgetSaida->updateTables();
      }
  }

  return QString();
}

void WidgetNfe::on_tabWidgetNfe_currentChanged(const int &) { updateTables(); }

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
