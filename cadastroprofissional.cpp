#include "cadastroprofissional.h"
#include "ui_cadastroprofissional.h"
#include "searchdialog.h"

CadastroProfissional::CadastroProfissional(QWidget *parent)
  : RegisterDialog("Profissional", "idProfissional", parent), ui(new Ui::CadastroProfissional) {
  ui->setupUi(this);

  setupMapper();
  newRegister();
}

CadastroProfissional::~CadastroProfissional() { delete ui; }

void CadastroProfissional::setupMapper() {
  mapper.addMapping(ui->lineEditNome, model.fieldIndex("nome"));
  mapper.addMapping(ui->lineEditEmail, model.fieldIndex("email"));
  mapper.addMapping(ui->lineEditTel, model.fieldIndex("tel"));
  mapper.addMapping(ui->comboBoxTipo, model.fieldIndex("tipo"));
  mapper.addMapping(ui->radioButtonAtivo, model.fieldIndex("ativo"));
  mapper.addMapping(ui->lineEditBanco, model.fieldIndex("banco"));
  mapper.addMapping(ui->lineEditAgencia, model.fieldIndex("agencia"));
  mapper.addMapping(ui->lineEditCC, model.fieldIndex("cc"));
  mapper.addMapping(ui->lineEditNomeBancario, model.fieldIndex("nomeBanco"));
  mapper.addMapping(ui->lineEditCPFBancario, model.fieldIndex("cpfBanco"));
}

void CadastroProfissional::registerMode() {
  ui->pushButtonCadastrar->show();
  ui->pushButtonAtualizar->hide();
  ui->pushButtonRemover->hide();
}

void CadastroProfissional::updateMode() {
  ui->pushButtonCadastrar->hide();
  ui->pushButtonAtualizar->show();
  ui->pushButtonRemover->show();
}

bool CadastroProfissional::verifyFields(int row) {
  Q_UNUSED(row);
  if (not RegisterDialog::verifyFields({ui->lineEditNome, ui->lineEditNome, ui->lineEditTel})) {
    return false;
  }
  return true;
}

bool CadastroProfissional::savingProcedures(int row) {
  setData(row, "nome", ui->lineEditNome->text());
  setData(row, "email", ui->lineEditEmail->text());
  setData(row, "tel", ui->lineEditTel->text());
  setData(row, "tipo", ui->comboBoxTipo->currentText());
  setData(row, "ativo", ui->radioButtonAtivo->isChecked());
  setData(row, "banco", ui->lineEditBanco->text());
  setData(row, "agencia", ui->lineEditAgencia->text());
  setData(row, "cc", ui->lineEditCC->text());
  setData(row, "nomeBanco", ui->lineEditNomeBancario->text());
  setData(row, "cpfBanco", ui->lineEditCPFBancario->text());
  return true;
}

void CadastroProfissional::on_pushButtonCadastrar_clicked() { save(); }

void CadastroProfissional::on_pushButtonAtualizar_clicked() { save(); }

void CadastroProfissional::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroProfissional::on_pushButtonRemover_clicked() { remove(); }

void CadastroProfissional::clearFields() {
  ui->radioButtonAtivo->setChecked(true);
  ui->radioButtonAtivo->setChecked(true);
  ui->lineEditEmail->clear();
  ui->lineEditNome->clear();
  ui->lineEditTel->clear();
  ui->comboBoxTipo->setCurrentIndex(0);
}

void CadastroProfissional::on_pushButtonCancelar_clicked() { close(); }

void CadastroProfissional::on_pushButtonBuscar_clicked() {
  SearchDialog *sdProfissional = SearchDialog::profissional(this);
  sdProfissional->show();
}
