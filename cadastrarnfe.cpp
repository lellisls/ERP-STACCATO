#include "cadastrarnfe.h"
#include "cadastrocliente.h"
#include "ui_cadastrarnfe.h"

CadastrarNFE::CadastrarNFE(QWidget *parent) :
  RegisterDialog("NFe","idNFe",parent),
  ui(new Ui::CadastrarNFE) {
  ui->setupUi(this);
  SearchDialog * sdCliente = SearchDialog::cliente(ui->itemBoxCliente);
  ui->itemBoxCliente->setSearchDialog(sdCliente);

  RegisterDialog * cadCliente = new CadastroCliente(this);
  ui->itemBoxCliente->setRegisterDialog(cadCliente);

  SearchDialog * sdEndereco = SearchDialog::endereco(ui->itemBoxEndereco);
  ui->itemBoxEndereco->setSearchDialog(sdEndereco);
}

CadastrarNFE::~CadastrarNFE() {
  delete ui;
}

void CadastrarNFE::on_pushButtonGerarNFE_clicked() {

}

void CadastrarNFE::on_pushButtonCancelar_clicked() {
  close();
}

void CadastrarNFE::gerarNFe(QString idVenda, QList<int> items()) {

}

// RegisterDialog interface

bool CadastrarNFE::verifyFields(int row) {
}

bool CadastrarNFE::savingProcedures(int row) {
}

void CadastrarNFE::clearFields() {
}

void CadastrarNFE::setupMapper() {
}

void CadastrarNFE::registerMode() {
}

void CadastrarNFE::updateMode() {
}

// End of RegisterDialog interface
