#include "userconfig.h"
#include "cadastrousuario.h"
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

void UserConfig::on_pushButtonUserFolder_clicked() {
  QString path = QFileDialog::getExistingDirectory(this, "Pasta PFD/Excel", QDir::currentPath());

  if (path.isEmpty()) {
    return;
  }

  ui->lineEditUserFolder->setText(path);
}

void UserConfig::on_pushButtonACBrEntrada_clicked() {
  QString path = QFileDialog::getExistingDirectory(this, "Pasta Entrada ACBr", QDir::currentPath());

  if (path.isEmpty()) {
    return;
  }

  ui->lineEditPastaEntACBr->setText(path);
}

void UserConfig::on_pushButtonACBrSaida_clicked() {
  QString path = QFileDialog::getExistingDirectory(this, "Pasta Saída ACBr", QDir::currentPath());

  if (path.isEmpty()) {
    return;
  }

  ui->lineEditPastaSaiACBr->setText(path);
}

void UserConfig::on_pushButtonACBrXML_clicked() {
  QString path = QFileDialog::getExistingDirectory(this, "Pasta XML ACBr", QDir::currentPath());

  if (path.isEmpty()) {
    return;
  }

  ui->lineEditPastaXmlACBr->setText(path);
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

QVariant UserConfig::settings(const QString &key) const { return UserSession::getSettings(key); }

void UserConfig::setSettings(const QString &key, const QVariant &value) const { UserSession::setSettings(key, value); }

void UserConfig::on_pushButtonAlterarDados_clicked() {
  CadastroUsuario *usuario = new CadastroUsuario(this);
  usuario->show();
  usuario->viewRegisterById(UserSession::getIdUsuario());
  usuario->modificarUsuario();
}
