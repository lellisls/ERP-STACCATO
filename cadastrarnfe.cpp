#include "cadastrarnfe.h"
#include "ui_cadastrarnfe.h"

CadastrarNFE::CadastrarNFE(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::CadastrarNFE) {
  ui->setupUi(this);
}

CadastrarNFE::~CadastrarNFE() {
  delete ui;
}
