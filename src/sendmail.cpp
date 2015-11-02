#include <QFileDialog>

#include "sendmail.h"
#include "ui_sendmail.h"
#include "smtp.h"
#include "usersession.h"

SendMail::SendMail(QWidget *parent, QString text, QString arquivo) : QDialog(parent), ui(new Ui::SendMail) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  ui->textEdit->setText(text);

  files.append(arquivo);

  ui->lineEditAnexo->setText(arquivo);

  QString email = UserSession::getFromLoja("emailCompra");
  ui->lineEditEmail->setText(email);
  ui->lineEditServidor->setText("smtp." + email.split("@").at(1));
  ui->lineEditPasswd->setText(UserSession::getFromLoja("emailSenha"));
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
  Smtp *smtp = new Smtp(ui->lineEditEmail->text(), ui->lineEditPasswd->text(), ui->lineEditServidor->text(),
                        ui->lineEditPorta->text().toInt());
  connect(smtp, &Smtp::status, this, &SendMail::mailSent);

  smtp->sendMail(ui->lineEditEmail->text(), ui->lineEditDest->text(), ui->lineEditTitulo->text(),
                 ui->textEdit->toPlainText(), files);
}

void SendMail::on_pushButtonCancelar_clicked() {
  QDialog::reject();
  close();
}

void SendMail::mailSent(QString status) {
  if (status == "Message sent") {
    QMessageBox::information(0, tr("Qt Simple SMTP client"), tr("Mensagem enviada!"));
  } else {
    QMessageBox::critical(0, tr("Qt Simple SMTP client"), "Ocorreu erro: " + status);
  }

  QDialog::accept();
}
