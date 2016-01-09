#include "loginconfig.h"
#include "logindialog.h"
#include "ui_loginconfig.h"
#include "usersession.h"

LoginConfig::LoginConfig(QWidget *parent) : QDialog(parent), ui(new Ui::LoginConfig) {
  ui->setupUi(this);

  setAttribute(Qt::WA_DeleteOnClose);

  ui->lineEditHostname->setText(settings("Login/hostname").toString());
  ui->lineEditUsername->setText(settings("Login/username").toString());
  ui->lineEditPassword->setText(settings("Login/password").toString());
  ui->lineEditPort->setText(settings("Login/port").toString());
  ui->checkBoxHomologacao->setChecked(settings("Login/homologacao").toBool());
}

LoginConfig::~LoginConfig() { delete ui; }

void LoginConfig::on_pushButtonSalvar_clicked() {
  setSettings("Login/hostname", ui->lineEditHostname->text());
  setSettings("Login/username", ui->lineEditUsername->text());
  setSettings("Login/password", ui->lineEditPassword->text());
  setSettings("Login/port", ui->lineEditPort->text());
  setSettings("Login/homologacao", ui->checkBoxHomologacao->isChecked());

  if (auto *dialog = qobject_cast<LoginDialog *>(parentWidget())) {
    dialog->readSettings();

    if (not dialog->dbConnect()) return;
  }

  close();
}

QVariant LoginConfig::settings(const QString &key) const { return UserSession::getSettings(key); }

void LoginConfig::setSettings(const QString &key, const QVariant &value) const { UserSession::setSettings(key, value); }
