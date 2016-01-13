#include <QFileDialog>

#include "sendmail.h"
#include "ui_sendmail.h"
#include "smtp.h"
#include "usersession.h"

SendMail::SendMail(QWidget *parent, const QString &text, const QString &arquivo)
  : QDialog(parent), ui(new Ui::SendMail) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);
  setAttribute(Qt::WA_DeleteOnClose);

  ui->textEdit->setText(text);

  files.append(arquivo);

  ui->lineEditAnexo->setText(arquivo);

  ui->lineEditServidor->setText(settings("User/servidorSMTP").toString());
  ui->lineEditPorta->setText(settings("User/portaSMTP").toString());
  ui->lineEditEmail->setText(settings("User/emailCompra").toString());
  ui->lineEditPasswd->setText(settings("User/emailSenha").toString());

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
  progress->show();

  setSettings("User/servidorSMTP", ui->lineEditServidor->text());
  setSettings("User/portaSMTP", ui->lineEditPorta->text());
  setSettings("User/emailCompra", ui->lineEditEmail->text());
  setSettings("User/emailSenha", ui->lineEditPasswd->text());

  Smtp *smtp = new Smtp(ui->lineEditEmail->text(), ui->lineEditPasswd->text(), ui->lineEditServidor->text(),
                        ui->lineEditPorta->text().toInt());
  connect(smtp, &Smtp::status, this, &SendMail::mailSent);

  smtp->sendMail(ui->lineEditEmail->text(), ui->lineEditDest->text(), ui->lineEditTitulo->text(),
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

QVariant SendMail::settings(const QString &key) const { return UserSession::settings(key); }

void SendMail::setSettings(const QString &key, const QVariant &value) const { UserSession::setSettings(key, value); }

// NOTE: SMTP: email-ssl.com.br - 465
