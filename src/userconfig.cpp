#include <QDebug>
#include <QFileDialog>

#include "cadastrousuario.h"
#include "ui_userconfig.h"
#include "userconfig.h"
#include "usersession.h"

UserConfig::UserConfig(QWidget *parent) : QDialog(parent), ui(new Ui::UserConfig) {
  ui->setupUi(this);

  ui->itemBoxLoja->setSearchDialog(SearchDialog::loja(this));

  ui->lineEditPastaEntACBr->setText(UserSession::settings("User/pastaEntACBr").toString());
  ui->lineEditPastaSaiACBr->setText(UserSession::settings("User/pastaSaiACBr").toString());
  ui->lineEditPastaXmlACBr->setText(UserSession::settings("User/pastaXmlACBr").toString());
  ui->itemBoxLoja->setValue(UserSession::settings("User/lojaACBr"));

  ui->lineEditServidorSMTP->setText(UserSession::settings("User/servidorSMTP").toString());
  ui->lineEditPortaSMTP->setText(UserSession::settings("User/portaSMTP").toString());
  ui->lineEditEmail->setText(UserSession::settings("User/emailCompra").toString());
  ui->lineEditEmailSenha->setText(UserSession::settings("User/emailSenha").toString());
  ui->lineEditEmailCopia->setText(UserSession::settings("User/emailCopia").toString());

  ui->lineEditOrcamentosFolder->setText(UserSession::settings("User/OrcamentosFolder").toString());
  ui->lineEditVendasFolder->setText(UserSession::settings("User/VendasFolder").toString());
  ui->lineEditComprasFolder->setText(UserSession::settings("User/ComprasFolder").toString());
  ui->lineEditEntregasFolder->setText(UserSession::settings("User/EntregasFolder").toString());

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

void UserConfig::on_pushButtonACBrEntrada_clicked() {
  const QString path = QFileDialog::getExistingDirectory(this, "Pasta Entrada ACBr", QDir::currentPath());

  if (path.isEmpty()) return;

  ui->lineEditPastaEntACBr->setText(path);
}

void UserConfig::on_pushButtonACBrSaida_clicked() {
  const QString path = QFileDialog::getExistingDirectory(this, "Pasta Saída ACBr", QDir::currentPath());

  if (path.isEmpty()) return;

  ui->lineEditPastaSaiACBr->setText(path);
}

void UserConfig::on_pushButtonACBrXML_clicked() {
  const QString path = QFileDialog::getExistingDirectory(this, "Pasta XML ACBr", QDir::currentPath());

  if (path.isEmpty()) return;

  ui->lineEditPastaXmlACBr->setText(path);
}

void UserConfig::on_pushButtonSalvar_clicked() {
  UserSession::setSettings("User/pastaEntACBr", ui->lineEditPastaEntACBr->text());
  UserSession::setSettings("User/pastaSaiACBr", ui->lineEditPastaSaiACBr->text());
  UserSession::setSettings("User/pastaXmlACBr", ui->lineEditPastaXmlACBr->text());
  UserSession::setSettings("User/lojaACBr", ui->itemBoxLoja->value());

  UserSession::setSettings("User/servidorSMTP", ui->lineEditServidorSMTP->text());
  UserSession::setSettings("User/portaSMTP", ui->lineEditPortaSMTP->text());
  UserSession::setSettings("User/emailCompra", ui->lineEditEmail->text());
  UserSession::setSettings("User/emailSenha", ui->lineEditEmailSenha->text());
  UserSession::setSettings("User/emailCopia", ui->lineEditEmailCopia->text());

  UserSession::setSettings("User/OrcamentosFolder", ui->lineEditOrcamentosFolder->text());
  UserSession::setSettings("User/VendasFolder", ui->lineEditVendasFolder->text());
  UserSession::setSettings("User/ComprasFolder", ui->lineEditComprasFolder->text());
  UserSession::setSettings("User/EntregasFolder", ui->lineEditEntregasFolder->text());

  QDialog::accept();

  close();
}

void UserConfig::on_pushButtonAlterarDados_clicked() {
  CadastroUsuario *usuario = new CadastroUsuario(this);
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

void UserConfig::on_pushButtonEntregasFolder_clicked() {
  const QString path = QFileDialog::getExistingDirectory(this, "Pasta PDF/Excel", QDir::currentPath());

  if (path.isEmpty()) return;

  ui->lineEditEntregasFolder->setText(path);
}
