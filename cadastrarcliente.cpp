#include "cadastrarcliente.h"
#include "ui_cadastrarcliente.h"
#include "cadastrocliente.h"

CadastrarCliente::CadastrarCliente(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::CadastrarCliente) {
  ui->setupUi(this);
}

CadastrarCliente::~CadastrarCliente() {
  delete ui;
}

void CadastrarCliente::on_pushButtonPJ_clicked() {
  if(CadastroCliente *cad = qobject_cast<CadastroCliente *>(parentWidget())) {
    qDebug() << "cast ok!";
    cad->setTipo("PJ");
    cad->show();
    close();
  }
}

void CadastrarCliente::on_pushButtonPF_clicked() {
  if(CadastroCliente *cad = qobject_cast<CadastroCliente *>(parentWidget())) {
    qDebug() << "cast ok!";
    cad->setTipo("PF");
    cad->show();
    close();
  }
}
