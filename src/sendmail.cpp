#include <QFileDialog>

#include "sendmail.h"
#include "smtp.h"
#include "ui_sendmail.h"

SendMail::SendMail(QWidget *parent) : QDialog(parent), ui(new Ui::SendMail) {
  ui->setupUi(this);

  setWindowModality(Qt::NonModal);
  ui->textEdit->insertHtml("<h1>Título</h1><p>Texto</p>");

  exec();
}

SendMail::~SendMail() { delete ui; }

void SendMail::on_pushButtonBuscar_clicked() {
  ui->lineEditAnexo->setText(QFileDialog::getOpenFileName(this, "Abrir anexo", QDir::homePath(), "*"));
}

void SendMail::on_pushButtonEnviar_clicked() {
  Smtp *smtp = new Smtp(ui->lineEditEmail->text(), ui->lineEditPasswd->text(), "smtp.gmail.com", 465, 30000);

  if (ui->lineEditAnexo->text().isEmpty()) {
    smtp->sendMail(ui->lineEditEmail->text(), ui->lineEditDest->text(), ui->lineEditTitulo->text(),
                   ui->textEdit->toHtml());
  } else {
    smtp->sendMail(ui->lineEditEmail->text(), ui->lineEditDest->text(), ui->lineEditTitulo->text(),
                   ui->textEdit->toHtml(), QStringList(ui->lineEditAnexo->text()));
  }
}

void SendMail::on_pushButtonCancelar_clicked() { close(); }
