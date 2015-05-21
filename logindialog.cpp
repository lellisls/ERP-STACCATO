#include <QDebug>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>

#include "logindialog.h"
#include "mainwindow.h"
#include "ui_logindialog.h"
#include "usersession.h"
#include "loginconfig.h"
LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent), ui(new Ui::LoginDialog) {
  ui->setupUi(this);

  setWindowTitle("ERP Login");
  setWindowModality(Qt::WindowModal);

  ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Login");
  ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Cancelar");
  QSettings settings("ERP", "Staccato");
  settings.beginGroup("User");
  if (settings.contains("lastuser")) {
    ui->lineEditUser->setText(settings.value("lastuser").toString());
    ui->lineEditPass->setFocus();
  } else {
    ui->lineEditUser->setFocus();
  }
  accept(); // para pular a tela de login, edite o construtor de mainwindow
}

LoginDialog::~LoginDialog() { delete ui; }

void LoginDialog::on_buttonBox_accepted() {
  verify();
  QSettings settings("ERP", "Staccato");
  settings.beginGroup("User");
  settings.setValue("lastuser", QString(ui->lineEditUser->text()));
}

void LoginDialog::on_buttonBox_rejected() { reject(); }

void LoginDialog::verify() {
  if (MainWindow *window = qobject_cast<MainWindow *>(parentWidget())) {
    //    qDebug() << "cast ok!";
    if (not window->dbConnect()) {
      return;
    }
  }

  if (UserSession::login(ui->lineEditUser->text(), ui->lineEditPass->text())) {
    accept();
  } else {
    QMessageBox::warning(this, "Aviso", "Login inválido", QMessageBox::Ok, QMessageBox::NoButton);
    ui->lineEditPass->setFocus();
  }
}

void LoginDialog::on_pushButtonConfig_clicked() {
  LoginConfig *config = new LoginConfig(this);
  config->show();
}
