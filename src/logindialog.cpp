#include <QMessageBox>

#include "logindialog.h"
#include "ui_logindialog.h"
#include "mainwindow.h"
#include "usersession.h"
#include "loginconfig.h"

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent), ui(new Ui::LoginDialog) {
  ui->setupUi(this);

  setWindowTitle("ERP Login");
  setWindowModality(Qt::WindowModal);

  ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Login");
  ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Cancelar");

  ui->lineEditUser->setFocus();

  if (settingsContains("User/lastuser")) {
    ui->lineEditUser->setText(settings("User/lastuser").toString());
    ui->lineEditPass->setFocus();
  }

  accept();
}

LoginDialog::~LoginDialog() { delete ui; }

void LoginDialog::on_buttonBox_accepted() {
  verify();

  setSettings("User/lastuser", ui->lineEditUser->text());
}

void LoginDialog::on_buttonBox_rejected() { reject(); }

void LoginDialog::verify() {
  if (MainWindow *window = qobject_cast<MainWindow *>(parentWidget())) {
    if (not window->dbConnect()) {
      return;
    }
  }

  if (not UserSession::login(ui->lineEditUser->text(), ui->lineEditPass->text())) {
    QMessageBox::critical(this, "Erro!", "Login inválido!");
    ui->lineEditPass->setFocus();
    return;
  }

  accept();
}

void LoginDialog::on_pushButtonConfig_clicked() {
  LoginConfig *config = new LoginConfig(this);
  config->show();
}

QVariant LoginDialog::settings(QString key) const { return UserSession::getSettings(key); }

void LoginDialog::setSettings(QString key, QVariant value) const { UserSession::setSettings(key, value); }

bool LoginDialog::settingsContains(const QString &key) const { return UserSession::settingsContains(key); }
