#include "loginconfig.h"
#include "ui_loginconfig.h"
#include "mainwindow.h"

LoginConfig::LoginConfig(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::LoginConfig)
{
  ui->setupUi(this);
}

LoginConfig::~LoginConfig()
{
  delete ui;
}

void LoginConfig::on_pushButtonSalvar_clicked()
{
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
