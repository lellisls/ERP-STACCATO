#include <QFileDialog>
#include <QStyleFactory>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QMessageBox>

#include "xml_viewer.h"
#include "ui_xml_viewer.h"
#include "usersession.h"
#include "xml.h"

XML_Viewer::XML_Viewer(QWidget *parent) : QDialog(parent), ui(new Ui::XML_Viewer) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  ui->treeView->setModel(&model);
  ui->treeView->setUniformRowHeights(true);
  ui->treeView->setAnimated(true);
  ui->treeView->setEditTriggers(QTreeView::NoEditTriggers);
}

XML_Viewer::~XML_Viewer() { delete ui; }

void XML_Viewer::exibirXML(QString fileContent) {
  if (fileContent.isEmpty()) {
    return;
  }

  QDomDocument document;
  QString *error = new QString();

  if (not document.setContent(fileContent, error)) {
    QMessageBox::critical(this, "Erro!", "Erro lendo arquivo: " + *error);
    return;
  }

  QDomElement root = document.firstChildElement();
  QDomNamedNodeMap map = root.attributes();
  QStandardItem *rootItem;

  if (map.size() > 0) {
    QString attributes = root.nodeName() + " ";

    for (int i = 0; i < map.size(); ++i) {
      if (i > 0) {
        attributes += " ";
      }

      attributes += map.item(i).nodeName() + "=\"" + map.item(i).nodeValue() + "\"";
    }

    rootItem = new QStandardItem(attributes);
  } else {
    rootItem = new QStandardItem(root.nodeName());
  }

  model.appendRow(rootItem);

  XML xml;
  xml.readChild(root, rootItem);

  ui->treeView->expandAll();
}
