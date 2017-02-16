#include <QCoreApplication>
#include <QDesktopServices>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSqlError>
#include <QSqlQuery>
#include <QTcpSocket>
#include <QUrl>

#include "acbr.h"

ACBr::ACBr(QObject *parent) : QObject(parent) {}

bool ACBr::gerarDanfe(const int idNFe) {
  if (idNFe == 0) {
    QMessageBox::critical(0, "Erro!", "Produto não possui nota!");
    return false;
  }

  QSqlQuery query;
  query.prepare("SELECT xml FROM nfe WHERE idNFe = :idNFe");
  query.bindValue(":idNFe", idNFe);

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(0, "Erro!", "Erro buscando chaveAcesso: " + query.lastError().text());
    return false;
  }

  QString resposta;

  if (not enviarComando("NFE.SaveToFile(xml.xml,\"" + query.value("xml").toString() + "\"", resposta)) return false;
  if (not enviarComando("NFE.ImprimirDANFEPDF(xml.xml)", resposta)) return false;
  if (not abrirPdf(resposta)) return false;

  return true;
}

bool ACBr::gerarDanfe(const QByteArray &fileContent, QString &resposta, const bool openFile) {
  if (not enviarComando("NFE.SaveToFile(xml.xml,\"" + fileContent + "\"", resposta)) return false;

  if (not resposta.contains("OK")) {
    QMessageBox::critical(0, "Erro!", "Erro salvando XML: " + resposta);
    return false;
  }

  if (not enviarComando("NFE.ImprimirDANFEPDF(xml.xml)", resposta)) return false;

  if (not resposta.contains("Arquivo criado em:")) {
    QMessageBox::critical(0, "Erro!", resposta);
    QMessageBox::critical(0, "Erro!", "Verifique se o arquivo não está aberto!");
    return false;
  }

  resposta = resposta.remove("OK: Arquivo criado em: ");

  if (openFile) abrirPdf(resposta);

  return true;
}

bool ACBr::abrirPdf(QString &resposta) {
  if (not QDesktopServices::openUrl(QUrl::fromLocalFile(resposta))) {
    QMessageBox::critical(0, "Erro!", "Erro abrindo PDF!");
    return false;
  }

  return true;
}

bool ACBr::enviarComando(const QString &comando, QString &resposta) {
  QTcpSocket socket;

  socket.connectToHost("127.0.0.1", 3434);

  if (not socket.waitForConnected(10)) {
    resposta = "Erro: Não foi possível conectar ao ACBr!";
    return false;
  }

  QProgressDialog *progressDialog = new QProgressDialog(0);
  progressDialog->reset();
  progressDialog->setCancelButton(0);
  progressDialog->setLabelText("Esperando ACBr...");
  progressDialog->setWindowTitle("ERP Staccato");
  progressDialog->setWindowModality(Qt::WindowModal);
  progressDialog->setMaximum(0);
  progressDialog->setMinimum(0);
  progressDialog->show();

  connect(&socket, &QTcpSocket::readyRead, progressDialog, &QProgressDialog::cancel);

  while (not progressDialog->wasCanceled()) QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

  progressDialog->cancel();

  socket.readAll(); // lendo mensagem de boas vindas

  QTextStream stream(&socket);

  stream << comando << endl;

  stream << "\r\n.\r\n";
  stream.flush();

  progressDialog->reset();
  progressDialog->show();

  connect(&socket, &QTcpSocket::readyRead, progressDialog, &QProgressDialog::cancel);

  while (not progressDialog->wasCanceled()) QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

  progressDialog->cancel();

  resposta = QString(socket.readAll()).remove("\u0003");

  return true;
}
