#include <QFileDialog>

#include "sendmail.h"
#include "smtp.h"
#include "ui_sendmail.h"
#include "usersession.h"

SendMail::SendMail(QWidget *parent, const QString &text, const QString &arquivo)
  : QDialog(parent), ui(new Ui::SendMail) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);
  setAttribute(Qt::WA_DeleteOnClose);

  ui->textEdit->setText(text);

  files.append(arquivo);

  ui->lineEditAnexo->setText(arquivo);

  ui->lineEditServidor->setText(UserSession::settings("User/servidorSMTP").toString());
  ui->lineEditPorta->setText(UserSession::settings("User/portaSMTP").toString());
  ui->lineEditEmail->setText(UserSession::settings("User/emailCompra").toString());
  ui->lineEditPasswd->setText(UserSession::settings("User/emailSenha").toString());

  progress = new QProgressDialog("Enviando...", "Cancelar", 0, 0, this);
  progress->setCancelButton(0);
  progress->reset();
}

SendMail::~SendMail() { delete ui; }

void SendMail::on_pushButtonBuscar_clicked() {
  files.clear();

  files = QFileDialog::getOpenFileNames(this, "Abrir anexos", QDir::homePath());

  QString fileListString;

  for (const auto file : files) {
    fileListString.append("\"" + QFileInfo(file).fileName() + "\" ");
  }

  ui->lineEditAnexo->setText(fileListString);
}

void SendMail::on_pushButtonEnviar_clicked() {
    // TODO: verificar campos obrigatorios

  progress->show();

  UserSession::setSettings("User/servidorSMTP", ui->lineEditServidor->text());
  UserSession::setSettings("User/portaSMTP", ui->lineEditPorta->text());
  UserSession::setSettings("User/emailCompra", ui->lineEditEmail->text());
  UserSession::setSettings("User/emailSenha", ui->lineEditPasswd->text());

  Smtp *smtp = new Smtp(ui->lineEditEmail->text(), ui->lineEditPasswd->text(), ui->lineEditServidor->text(),
                        ui->lineEditPorta->text().toInt());
  connect(smtp, &Smtp::status, this, &SendMail::mailSent);

  smtp->sendMail(ui->lineEditEmail->text(), ui->lineEditDest->text(), ui->lineEditCopia->text(), ui->lineEditTitulo->text(),
                 ui->textEdit->toPlainText(), files);
}

void SendMail::mailSent(const QString &status) {
  progress->cancel();
  status == "Message sent" ? successStatus() : failureStatus(status);
}

void SendMail::successStatus() {
  QMessageBox::information(0, tr("Qt Simple SMTP client"), tr("Mensagem enviada!"));

  QDialog::accept();
}

void SendMail::failureStatus(const QString &status) {
  QMessageBox::critical(0, tr("Qt Simple SMTP client"), "Ocorreu erro: " + status);
}

// TODO: como tem fornecedor que tem mais de um email, fazer um combobox listando os emails para o usuario escolher
