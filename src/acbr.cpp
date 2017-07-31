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
    QMessageBox::critical(nullptr, "Erro!", "Produto não possui nota!");
    return false;
  }

  QSqlQuery query;
  query.prepare("SELECT xml FROM nfe WHERE idNFe = :idNFe");
  query.bindValue(":idNFe", idNFe);

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(nullptr, "Erro!", "Erro buscando chaveAcesso: " + query.lastError().text());
    return false;
  }

  QString resposta;

  if (not enviarComando(R"(NFE.SaveToFile(xml.xml,")" + query.value("xml").toString() + R"(")", resposta)) return false;

  if (not resposta.contains("OK")) {
    QMessageBox::critical(nullptr, "Erro!", "Erro salvando XML: " + resposta);
    return false;
  }

  if (not enviarComando("NFE.ImprimirDANFEPDF(xml.xml)", resposta)) return false;

  if (not resposta.contains("Arquivo criado em:")) {
    QMessageBox::critical(nullptr, "Erro!", resposta);
    QMessageBox::critical(nullptr, "Erro!", "Verifique se o arquivo não está aberto!");
    return false;
  }

  resposta = resposta.remove("OK: Arquivo criado em: ");

  return abrirPdf(resposta);
}

bool ACBr::gerarDanfe(const QByteArray &fileContent, QString &resposta, const bool openFile) {
  if (not enviarComando(R"(NFE.SaveToFile(xml.xml,")" + fileContent + R"(")", resposta)) return false;

  if (not resposta.contains("OK")) {
    QMessageBox::critical(nullptr, "Erro!", "Erro salvando XML: " + resposta);
    return false;
  }

  if (not enviarComando("NFE.ImprimirDANFEPDF(xml.xml)", resposta)) return false;

  if (not resposta.contains("Arquivo criado em:")) {
    QMessageBox::critical(nullptr, "Erro!", resposta);
    QMessageBox::critical(nullptr, "Erro!", "Verifique se o arquivo não está aberto!");
    return false;
  }

  resposta = resposta.remove("OK: Arquivo criado em: ");

  if (openFile) abrirPdf(resposta);

  // TODO: 1copiar arquivo para pasta predefinida e renomear arquivo para formato 'DANFE_xxx.xxx_idpedido'

  return true;
}

bool ACBr::abrirPdf(QString &resposta) {
  if (not QDesktopServices::openUrl(QUrl::fromLocalFile(resposta))) {
    QMessageBox::critical(nullptr, "Erro!", "Erro abrindo PDF!");
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

  QProgressDialog *progressDialog = new QProgressDialog(nullptr);
  progressDialog->reset();
  progressDialog->setCancelButton(nullptr);
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
