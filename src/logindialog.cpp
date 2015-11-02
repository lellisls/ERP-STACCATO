#include <QMessageBox>
#include <QSettings>

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

  QSettings settings("Staccato", "ERP");
  settings.beginGroup("User");

  if (settings.contains("lastuser")) {
    ui->lineEditUser->setText(settings.value("lastuser").toString());
    ui->lineEditPass->setFocus();
  } else {
    ui->lineEditUser->setFocus();
  }

  accept();
}

LoginDialog::~LoginDialog() { delete ui; }

void LoginDialog::on_buttonBox_accepted() {
  verify();

  QSettings settings("Staccato", "ERP");
  settings.beginGroup("User");
  settings.setValue("lastuser", QString(ui->lineEditUser->text()));
}

void LoginDialog::on_buttonBox_rejected() { reject(); }

void LoginDialog::verify() {
  if (MainWindow *window = qobject_cast<MainWindow *>(parentWidget())) {
    if (not window->dbConnect()) {
      return;
    }
  }

  if (UserSession::login(ui->lineEditUser->text(), ui->lineEditPass->text())) {
    accept();
  } else {
    QMessageBox::critical(this, "Erro!", "Login inválido");
    ui->lineEditPass->setFocus();
  }
}

void LoginDialog::on_pushButtonConfig_clicked() {
  LoginConfig *config = new LoginConfig(this);
  config->show();
}
