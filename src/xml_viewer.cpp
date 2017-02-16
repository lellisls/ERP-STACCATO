#include "xml_viewer.h"
#include "acbr.h"
#include "ui_xml_viewer.h"
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

void XML_Viewer::exibirXML(const QByteArray &fileContent) {
  if (fileContent.isEmpty()) return;

  this->fileContent = fileContent;

  XML xml(fileContent);
  xml.montarArvore(model);

  ui->treeView->expandAll();

  show();
}

void XML_Viewer::on_pushButtonDanfe_clicked() {
  QString resposta;
  ACBr::gerarDanfe(fileContent, resposta);
}
