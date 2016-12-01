#include <QDate>
#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QMessageBox>
#include <QProgressDialog>
#include <QTextStream>
#include <QUrl>

#include "ui_xml_viewer.h"
#include "usersession.h"
#include "xml.h"
#include "xml_viewer.h"

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
  if (fileContent.isEmpty()) return;

  this->fileContent = fileContent;

  XML xml(fileContent);
  xml.montarArvore(model);

  ui->treeView->expandAll();

  show();
}

bool XML_Viewer::imprimirDanfe() {
  const QString dirEntrada = UserSession::settings("User/pastaEntACBr").toString();
  const QString dirSaida = UserSession::settings("User/pastaSaiACBr").toString();
  const QString dirXml = UserSession::settings("User/pastaXmlACBr").toString();

  QFile fileXml(dirXml + "/temp-nfe.xml");

  if (not fileXml.open(QFile::WriteOnly)) {
    QMessageBox::critical(this, "Erro!", "Não foi possível salvar o XML, favor verificar se as pastas "
                                         "estão corretamente configuradas.");
    return false;
  }

  QTextStream streamXml(&fileXml);

  streamXml << fileContent;

  streamXml.flush();
  fileXml.close();

  QFile fileGerar(dirEntrada + "/gerarDanfe.txt");

  if (not fileGerar.open(QFile::WriteOnly)) {
    QMessageBox::critical(this, "Erro!", "Não foi possível enviar o pedido para o ACBr, favor verificar se as pastas "
                                         "estão corretamente configuradas.");
    return false;
  }

  QTextStream streamGerar(&fileGerar);

  streamGerar << "NFe.ImprimirDanfePDF(\"" + dirXml + "/temp-nfe.xml\")" << endl;

  streamGerar.flush();
  fileGerar.close();

  //

  QFile fileResposta(dirSaida + "/gerarDanfe-resp.txt");

  QProgressDialog *progressDialog = new QProgressDialog(this);
  progressDialog->reset();
  progressDialog->setCancelButton(0);
  progressDialog->setLabelText("Esperando ACBr...");
  progressDialog->setWindowTitle("ERP Staccato");
  progressDialog->setWindowModality(Qt::WindowModal);
  progressDialog->setMaximum(0);
  progressDialog->setMinimum(0);
  progressDialog->show();

  const QTime wait = QTime::currentTime().addSecs(10);

  while (QTime::currentTime() < wait) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    if (fileResposta.exists()) break;
  }

  progressDialog->cancel();

  if (not fileResposta.exists()) {
    QMessageBox::critical(this, "Erro!", "ACBr não respondeu, verificar se ele está aberto e funcionando!");

    if (fileGerar.exists()) fileGerar.remove();

    return false;
  }

  if (not fileResposta.open(QFile::ReadOnly)) {
    QMessageBox::critical(this, "Erro!", "Erro lendo arquivo: " + fileResposta.errorString());
    return false;
  }

  QTextStream ts(&fileResposta);

  QString resposta = ts.readAll();
  fileResposta.remove();

  if (not resposta.contains("OK")) {
    QMessageBox::critical(this, "Erro!", "Resposta do ACBr: " + resposta);
    return false;
  }

  //

  resposta = resposta.remove(0, 23);
  resposta.chop(2);

  if (not QDesktopServices::openUrl(QUrl::fromLocalFile(resposta))) {
    QMessageBox::critical(this, "Erro!", "Erro abrindo PDF!");
  }

  return true;
}

void XML_Viewer::on_pushButtonDanfe_clicked() {
  if (not imprimirDanfe()) {
    const QString dirEntrada = UserSession::settings("User/pastaEntACBr").toString();
    const QString dirSaida = UserSession::settings("User/pastaSaiACBr").toString();

    QFile fileGerar(dirEntrada + "/gerarDanfe.txt");

    if (fileGerar.exists()) fileGerar.remove();

    QFile fileResposta(dirSaida + "/gerarDanfe-resp.txt");

    if (fileResposta.exists()) fileResposta.remove();
  }
}
