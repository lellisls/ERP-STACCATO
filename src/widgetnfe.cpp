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

void WidgetNfe::on_pushButtonExibirXML_clicked() {
  const QString xml = QFileDialog::getOpenFileName(this, "Arquivo XML", QDir::currentPath(), "*.xml");

  if (xml.isEmpty()) return;

  QFile file(xml);

  if (not file.open(QFile::ReadOnly)) {
    QMessageBox::critical(this, "Erro!", "Erro abrindo arquivo: " + file.errorString());
    return;
  }

  XML_Viewer *viewer = new XML_Viewer(this);
  viewer->exibirXML(file.readAll());
}
