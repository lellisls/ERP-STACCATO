#include "cadastroprofissional.h"
#include "ui_cadastroprofissional.h"
#include "searchdialog.h"
#include "usersession.h"

CadastroProfissional::CadastroProfissional(QWidget *parent)
  : RegisterDialog("Profissional", "idProfissional", parent), ui(new Ui::CadastroProfissional) {
  ui->setupUi(this);

  setupMapper();
  newRegister();

  if (UserSession::getTipoUsuario() != "ADMINISTRADOR") {
    ui->tabWidget->setTabEnabled(1, false);
    ui->pushButtonRemover->setDisabled(true);
  }
}

CadastroProfissional::~CadastroProfissional() { delete ui; }

void CadastroProfissional::setupMapper() {
  addMapping(ui->lineEditNome, "nome");
  addMapping(ui->lineEditEmail, "email");
  addMapping(ui->lineEditTel, "tel");
  addMapping(ui->comboBoxTipo, "tipo");
  addMapping(ui->lineEditBanco, "banco");
  addMapping(ui->lineEditAgencia, "agencia");
  addMapping(ui->lineEditCC, "cc");
  addMapping(ui->lineEditNomeBancario, "nomeBanco");
  addMapping(ui->lineEditCPFBancario, "cpfBanco");
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

bool CadastroProfissional::viewRegister(QModelIndex index) {
  if (not RegisterDialog::viewRegister(index)) {
    return false;
  }

  return true;
}

void CadastroProfissional::changeItem(QVariant value, QString text) {
  Q_UNUSED(text);

  viewRegisterById(value);
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
  ui->lineEditEmail->clear();
  ui->lineEditNome->clear();
  ui->lineEditTel->clear();
  ui->comboBoxTipo->setCurrentIndex(0);
}

void CadastroProfissional::on_pushButtonCancelar_clicked() { close(); }

void CadastroProfissional::on_pushButtonBuscar_clicked() {
  SearchDialog *sdProfissional = SearchDialog::profissional(this);
  sdProfissional->setFilter("idProfissional NOT IN (1) AND desativado = false");
  connect(sdProfissional, &SearchDialog::itemSelected, this, &CadastroProfissional::changeItem);
  sdProfissional->show();
}

void CadastroProfissional::show() {
  QWidget::show();
  adjustSize();
}
