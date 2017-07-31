#include <QDebug>
#include <QFileDialog>

#include "cadastrousuario.h"
#include "ui_userconfig.h"
#include "userconfig.h"
#include "usersession.h"

UserConfig::UserConfig(QWidget *parent) : QDialog(parent), ui(new Ui::UserConfig) {
  ui->setupUi(this);

  ui->itemBoxLoja->setSearchDialog(SearchDialog::loja(this));
  ui->itemBoxLoja->setValue(UserSession::settings("User/lojaACBr"));

  ui->lineEditServidorSMTP->setText(UserSession::settings("User/servidorSMTP").toString());
  ui->lineEditPortaSMTP->setText(UserSession::settings("User/portaSMTP").toString());
  ui->lineEditEmail->setText(UserSession::settings("User/emailCompra").toString());
  ui->lineEditEmailSenha->setText(UserSession::settings("User/emailSenha").toString());
  ui->lineEditEmailCopia->setText(UserSession::settings("User/emailCopia").toString());

  ui->lineEditOrcamentosFolder->setText(UserSession::settings("User/OrcamentosFolder").toString());
  ui->lineEditVendasFolder->setText(UserSession::settings("User/VendasFolder").toString());
  ui->lineEditComprasFolder->setText(UserSession::settings("User/ComprasFolder").toString());
  ui->lineEditEntregasXmlFolder->setText(UserSession::settings("User/EntregasXmlFolder").toString());
  ui->lineEditEntregasPdfFolder->setText(UserSession::settings("User/EntregasPdfFolder").toString());

  if (UserSession::tipoUsuario() == "VENDEDOR" or UserSession::tipoUsuario() == "VENDEDOR ESPECIAL") {
    ui->groupBoxAcbr->hide();
    ui->groupBoxEmailCompra->hide();
    ui->labelCompras->hide();
    ui->lineEditComprasFolder->hide();
    ui->pushButtonComprasFolder->hide();
  }

  adjustSize();
}

UserConfig::~UserConfig() { delete ui; }

void UserConfig::on_pushButtonOrcamentosFolder_clicked() {
  const QString path = QFileDialog::getExistingDirectory(this, "Pasta PDF/Excel", QDir::currentPath());

  if (path.isEmpty()) return;

  ui->lineEditOrcamentosFolder->setText(path);
}

void UserConfig::on_pushButtonSalvar_clicked() {
  UserSession::setSettings("User/lojaACBr", ui->itemBoxLoja->getValue());

  UserSession::setSettings("User/servidorSMTP", ui->lineEditServidorSMTP->text());
  UserSession::setSettings("User/portaSMTP", ui->lineEditPortaSMTP->text());
  UserSession::setSettings("User/emailCompra", ui->lineEditEmail->text());
  UserSession::setSettings("User/emailSenha", ui->lineEditEmailSenha->text());
  UserSession::setSettings("User/emailCopia", ui->lineEditEmailCopia->text());

  UserSession::setSettings("User/OrcamentosFolder", ui->lineEditOrcamentosFolder->text());
  UserSession::setSettings("User/VendasFolder", ui->lineEditVendasFolder->text());
  UserSession::setSettings("User/ComprasFolder", ui->lineEditComprasFolder->text());
  UserSession::setSettings("User/EntregasXmlFolder", ui->lineEditEntregasXmlFolder->text());
  UserSession::setSettings("User/EntregasPdfFolder", ui->lineEditEntregasPdfFolder->text());

  QDialog::accept();

  close();
}

void UserConfig::on_pushButtonAlterarDados_clicked() {
  auto *usuario = new CadastroUsuario(this);
  usuario->show();
  usuario->viewRegisterById(UserSession::idUsuario());
  usuario->modificarUsuario();
}

void UserConfig::on_pushButtonVendasFolder_clicked() {
  const QString path = QFileDialog::getExistingDirectory(this, "Pasta PDF/Excel", QDir::currentPath());

  if (path.isEmpty()) return;

  ui->lineEditVendasFolder->setText(path);
}

void UserConfig::on_pushButtonComprasFolder_clicked() {
  const QString path = QFileDialog::getExistingDirectory(this, "Pasta PDF/Excel", QDir::currentPath());

  if (path.isEmpty()) return;

  ui->lineEditComprasFolder->setText(path);
}

void UserConfig::on_pushButtonEntregasXmlFolder_clicked() {
  const QString path = QFileDialog::getExistingDirectory(this, "Pasta XML", QDir::currentPath());

  if (path.isEmpty()) return;

  ui->lineEditEntregasXmlFolder->setText(path);
}

void UserConfig::on_pushButtonEntregasPdfFolder_clicked() {
  const QString path = QFileDialog::getExistingDirectory(this, "Pasta PDF/Excel", QDir::currentPath());

  if (path.isEmpty()) return;

  ui->lineEditEntregasPdfFolder->setText(path);
}
