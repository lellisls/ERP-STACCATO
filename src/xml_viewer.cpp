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
  setAttribute(Qt::WA_DeleteOnClose);

  ui->treeView->setModel(&model);
  ui->treeView->setUniformRowHeights(true);
  ui->treeView->setAnimated(true);
  ui->treeView->setEditTriggers(QTreeView::NoEditTriggers);
}

XML_Viewer::~XML_Viewer() { delete ui; }

void XML_Viewer::exibirXML(const QByteArray &fileContent) {
  if (fileContent.isEmpty()) {
    return;
  }

  XML xml(model, fileContent);

  ui->treeView->expandAll();

  show();
}
