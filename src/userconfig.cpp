#include "userconfig.h"
#include "ui_userconfig.h"
#include "usersession.h"

#include <QFileDialog>

UserConfig::UserConfig(QWidget *parent) : QDialog(parent), ui(new Ui::UserConfig) {
  ui->setupUi(this);

  ui->lineEditPastaEntACBr->setText(settings("User/pastaEntACBr").toString());
  ui->lineEditPastaSaiACBr->setText(settings("User/pastaSaiACBr").toString());
  ui->lineEditPastaXmlACBr->setText(settings("User/pastaXmlACBr").toString());

  ui->lineEditServidorSMTP->setText(settings("User/servidorSMTP").toString());
  ui->lineEditPortaSMTP->setText(settings("User/portaSMTP").toString());
  ui->lineEditEmail->setText(settings("User/emailCompra").toString());
  ui->lineEditEmailSenha->setText(settings("User/emailSenha").toString());

  ui->lineEditUserFolder->setText(settings("User/userFolder").toString());
}

UserConfig::~UserConfig() { delete ui; }

void UserConfig::show() {
  QDialog::show();
  adjustSize();
}

void UserConfig::on_pushButtonUserFolder_clicked() {
  QString path = QFileDialog::getExistingDirectory(this, "Pasta PFD/Excel", QDir::currentPath());

  if (path.isEmpty()) {
    return;
  }

  ui->lineEditUserFolder->setText(path);
}

void UserConfig::on_pushButtonSalvar_clicked() {
  setSettings("User/pastaEntACBr", ui->lineEditPastaEntACBr->text());
  setSettings("User/pastaSaiACBr", ui->lineEditPastaSaiACBr->text());
  setSettings("User/pastaXmlACBr", ui->lineEditPastaXmlACBr->text());

  setSettings("User/servidorSMTP", ui->lineEditServidorSMTP->text());
  setSettings("User/portaSMTP", ui->lineEditPortaSMTP->text());
  setSettings("User/emailCompra", ui->lineEditEmail->text());
  setSettings("User/emailSenha", ui->lineEditEmailSenha->text());

  setSettings("User/userFolder", ui->lineEditUserFolder->text());

  QDialog::accept();

  close();
}

void UserConfig::on_pushButtonCancelar_clicked() {
  QDialog::reject();

  close();
}

QVariant UserConfig::settings(const QString &key) const { return UserSession::getSettings(key); }

void UserConfig::setSettings(const QString &key, const QVariant &value) const { UserSession::setSettings(key, value); }
