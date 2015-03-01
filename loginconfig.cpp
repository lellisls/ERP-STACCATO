#include <QSettings>

#include "loginconfig.h"
#include "ui_loginconfig.h"
#include "mainwindow.h"

LoginConfig::LoginConfig(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::LoginConfig)
{
  ui->setupUi(this);

  QSettings settings("ERP", "Staccato");
  settings.beginGroup("Login");
  ui->lineEditHostname->setText(settings.value("hostname").toString());
  ui->lineEditUsername->setText(settings.value("username").toString());
  ui->lineEditPassword->setText(settings.value("password").toString());
  ui->lineEditPort->setText(settings.value("port").toString());
}

LoginConfig::~LoginConfig()
{
  delete ui;
}

void LoginConfig::on_pushButtonSalvar_clicked()
{
  QSettings settings("ERP", "Staccato");
  settings.beginGroup("Login");
  settings.setValue("hostname", ui->lineEditHostname->text());
  settings.setValue("username", ui->lineEditUsername->text());
  settings.setValue("password", ui->lineEditPassword->text());
  settings.setValue("port", ui->lineEditPort->text());

    if(MainWindow *window = qobject_cast<MainWindow *>(parentWidget()->parentWidget())){
//      qDebug() << "cast ok!";
      window->setHostname(ui->lineEditHostname->text());
      window->setUsername(ui->lineEditUsername->text());
      window->setPassword(ui->lineEditPassword->text());
      window->setPort(ui->lineEditPort->text());
      if(!window->dbConnect()){
        return;
      }
    }
    close();
}

void LoginConfig::on_pushButtonCancelar_clicked()
{
    close();
}
