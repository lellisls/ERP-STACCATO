#include <QMessageBox>
#include <QSqlError>
#include <QDebug>

#include "cadastrotransportadora.h"
#include "ui_cadastrotransportadora.h"
#include "searchdialog.h"
#include "cepcompleter.h"
#include "usersession.h"

CadastroTransportadora::CadastroTransportadora(QWidget *parent)
  : RegisterAddressDialog("transportadora", "idTransportadora", parent), ui(new Ui::CadastroTransportadora) {
  ui->setupUi(this);

  setupTables();
  setupUi();
  setupMapper();
  newRegister();

  for (const auto *line : findChildren<QLineEdit *>(QString() , Qt::FindDirectChildrenOnly)) {
    connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);
  }

  if (UserSession::getTipoUsuario() != "ADMINISTRADOR") {
    ui->pushButtonRemover->setDisabled(true);
    ui->pushButtonRemoverEnd->setDisabled(true);
  }
}

CadastroTransportadora::~CadastroTransportadora() { delete ui; }

void CadastroTransportadora::setupTables() {
  ui->tableEndereco->setModel(&modelEnd);
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("idEndereco"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("desativado"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("idTransportadora"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("codUF"));
}

void CadastroTransportadora::clearFields() {
  RegisterDialog::clearFields();
  novoEndereco();
  setupUi();
}

bool CadastroTransportadora::verifyFields() {
  if (not RegisterDialog::verifyFields({ui->lineEditANTT, ui->lineEditCNPJ, ui->lineEditInscEstadual,
                                       ui->lineEditNomeFantasia, ui->lineEditPlaca, ui->lineEditRazaoSocial,
                                       ui->lineEditTel})) {
    return false;
  }

  return true;
}

bool CadastroTransportadora::savingProcedures(const int row) {
  if (not setData(row, "cnpj", ui->lineEditCNPJ->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando CNPJ!");
    return false;
  }

  if (not setData(row, "razaoSocial", ui->lineEditRazaoSocial->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Razão Social!");
    return false;
  }

  if (not setData(row, "nomeFantasia", ui->lineEditNomeFantasia->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Nome Fantasia!");
    return false;
  }

  if (not setData(row, "inscEstadual", ui->lineEditInscEstadual->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Insc. Estadual!");
    return false;
  }

  if (not setData(row, "tel", ui->lineEditTel->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Telefone!");
    return false;
  }

  if (not setData(row, "antt", ui->lineEditANTT->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando ANTT!");
    return false;
  }

  if (not setData(row, "placaVeiculo", ui->lineEditPlaca->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Placa Veículo!");
    return false;
  }

  return true;
}

void CadastroTransportadora::setupMapper() {
  addMapping(ui->lineEditCNPJ, "cnpj");
  addMapping(ui->lineEditRazaoSocial, "razaoSocial");
  addMapping(ui->lineEditNomeFantasia, "nomeFantasia");
  addMapping(ui->lineEditInscEstadual, "inscEstadual");
  addMapping(ui->lineEditTel, "tel");
  addMapping(ui->lineEditANTT, "antt");
  addMapping(ui->lineEditPlaca, "placaVeiculo");

  mapperEnd.addMapping(ui->comboBoxTipoEnd, modelEnd.fieldIndex("descricao"));
  mapperEnd.addMapping(ui->lineEditCEP, modelEnd.fieldIndex("CEP"));
  mapperEnd.addMapping(ui->lineEditLogradouro, modelEnd.fieldIndex("logradouro"));
  mapperEnd.addMapping(ui->lineEditNro, modelEnd.fieldIndex("numero"));
  mapperEnd.addMapping(ui->lineEditComp, modelEnd.fieldIndex("complemento"));
  mapperEnd.addMapping(ui->lineEditBairro, modelEnd.fieldIndex("bairro"));
  mapperEnd.addMapping(ui->lineEditCidade, modelEnd.fieldIndex("cidade"));
  mapperEnd.addMapping(ui->lineEditUF, modelEnd.fieldIndex("uf"));
}

void CadastroTransportadora::registerMode() {
  ui->pushButtonCadastrar->show();
  ui->pushButtonAtualizar->hide();
  ui->pushButtonRemover->hide();
}

void CadastroTransportadora::updateMode() {
  ui->pushButtonCadastrar->hide();
  ui->pushButtonAtualizar->show();
  ui->pushButtonRemover->show();
}

void CadastroTransportadora::on_pushButtonCadastrar_clicked() { save(); }

void CadastroTransportadora::on_pushButtonAtualizar_clicked() { update(); }

void CadastroTransportadora::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroTransportadora::on_pushButtonRemover_clicked() { remove(); }

void CadastroTransportadora::on_pushButtonCancelar_clicked() { close(); }

void CadastroTransportadora::on_pushButtonBuscar_clicked() {
  SearchDialog *sdTransportadora = SearchDialog::transportadora(this);
  connect(sdTransportadora, &SearchDialog::itemSelected, this, &CadastroTransportadora::viewRegisterById);
  sdTransportadora->show();
}

bool CadastroTransportadora::newRegister() {
  if (not RegisterDialog::newRegister()) {
    return false;
  }

  novoItem();

  return true;
}

void CadastroTransportadora::novoItem() {
  ui->pushButtonAtualizar->hide();
  ui->pushButtonRemover->hide();
}

void CadastroTransportadora::on_lineEditCNPJ_textEdited(const QString &text) {
  ui->lineEditCNPJ->setStyleSheet(
        validaCNPJ(QString(text).remove(".").remove("/").remove("-")) ? "" : "color: rgb(255, 0, 0);");
}

void CadastroTransportadora::on_pushButtonAdicionarEnd_clicked() {
  cadastrarEndereco(false)
      ? novoEndereco()
      : static_cast<void>(QMessageBox::critical(this, "Erro!", "Não foi possível cadastrar este endereço!"));
}

void CadastroTransportadora::on_pushButtonAtualizarEnd_clicked() {
  cadastrarEndereco(true)
      ? novoEndereco()
      : static_cast<void>(QMessageBox::critical(this, "Erro!", "Não foi possível atualizar este endereço!"));
}

void CadastroTransportadora::on_pushButtonEndLimpar_clicked() { novoEndereco(); }

void CadastroTransportadora::on_pushButtonRemoverEnd_clicked() {
  QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Tem certeza que deseja remover?",
                     QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Sim");
  msgBox.setButtonText(QMessageBox::No, "Não");

  if (msgBox.exec() == QMessageBox::Yes) {
    if (not modelEnd.submitAll()) {
      QMessageBox::critical(this, "Erro!", "Não foi possível remover este item: " + modelEnd.lastError().text());
      return;
    }

    if (not modelEnd.select()) {
      QMessageBox::critical(this, "Erro!",
                            "Erro ao ler a tabela de endereço da transportadora: " + modelEnd.lastError().text());
      return;
    }

    novoEndereco();
  }
}

void CadastroTransportadora::on_checkBoxMostrarInativos_clicked(const bool checked) {
  modelEnd.setFilter("idTransportadora = " + data(primaryKey).toString() + (checked ? "" : " AND desativado = FALSE"));
}

bool CadastroTransportadora::cadastrarEndereco(const bool isUpdate) {
  if (not RegisterDialog::verifyFields({ui->lineEditCEP, ui->lineEditLogradouro, ui->lineEditNro, ui->lineEditBairro,
                                       ui->lineEditCidade, ui->lineEditUF})) {
    return false;
  }

  if (not ui->lineEditCEP->isValid()) {
    ui->lineEditCEP->setFocus();
    QMessageBox::critical(this, "Erro!", "CEP inválido!");
    return false;
  }

  const int row = (isUpdate) ? mapperEnd.currentIndex() : modelEnd.rowCount();

  if (not isUpdate) {
    modelEnd.insertRow(row);
  }

  if (not modelEnd.setData(row, "descricao", ui->comboBoxTipoEnd->currentText())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando descrição: " + modelEnd.lastError().text());
    return false;
  }

  if (not ui->lineEditCEP->text().isEmpty()) {
    if (not modelEnd.setData(row, "CEP", ui->lineEditCEP->text())) {
      QMessageBox::critical(this, "Erro!", "Erro guardando CEP: " + modelEnd.lastError().text());
      return false;
    }
  }

  if (not ui->lineEditLogradouro->text().isEmpty()) {
    if (not modelEnd.setData(row, "logradouro", ui->lineEditLogradouro->text())) {
      QMessageBox::critical(this, "Erro!", "Erro guardando logradouro: " + modelEnd.lastError().text());
      return false;
    }
  }

  if (not ui->lineEditNro->text().isEmpty()) {
    if (not modelEnd.setData(row, "numero", ui->lineEditNro->text())) {
      QMessageBox::critical(this, "Erro!", "Erro guardando número: " + modelEnd.lastError().text());
      return false;
    }
  }

  if (not ui->lineEditComp->text().isEmpty()) {
    if (not modelEnd.setData(row, "complemento", ui->lineEditComp->text())) {
      QMessageBox::critical(this, "Erro!", "Erro guardando complemento: " + modelEnd.lastError().text());
      return false;
    }
  }

  if (not ui->lineEditBairro->text().isEmpty()) {
    if (not modelEnd.setData(row, "bairro", ui->lineEditBairro->text())) {
      QMessageBox::critical(this, "Erro!", "Erro guardando bairro: " + modelEnd.lastError().text());
      return false;
    }
  }

  if (not ui->lineEditCidade->text().isEmpty()) {
    if (not modelEnd.setData(row, "cidade", ui->lineEditCidade->text())) {
      QMessageBox::critical(this, "Erro!", "Erro guardando cidade: " + modelEnd.lastError().text());
      return false;
    }
  }

  if (not ui->lineEditUF->text().isEmpty()) {
    if (not modelEnd.setData(row, "uf", ui->lineEditUF->text())) {
      QMessageBox::critical(this, "Erro!", "Erro guardando UF: " + modelEnd.lastError().text());
      return false;
    }

    if (not modelEnd.setData(row, "codUF", getCodigoUF(ui->lineEditUF->text()))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando codUF: " + modelEnd.lastError().text());
      return false;
    }
  }

  if (not modelEnd.setData(row, "desativado", 0)) {
    QMessageBox::critical(this, "Erro!", "Erro guardando desativado: " + modelEnd.lastError().text());
    return false;
  }

  ui->tableEndereco->resizeColumnsToContents();

  return true;
}

void CadastroTransportadora::novoEndereco() {
  ui->pushButtonAtualizarEnd->hide();
  ui->pushButtonAdicionarEnd->show();
  ui->tableEndereco->clearSelection();
  clearEndereco();
}

void CadastroTransportadora::clearEndereco() {
  ui->lineEditBairro->clear();
  ui->lineEditCEP->clear();
  ui->lineEditCidade->clear();
  ui->lineEditComp->clear();
  ui->lineEditLogradouro->clear();
  ui->lineEditNro->clear();
  ui->lineEditUF->clear();
}

void CadastroTransportadora::on_lineEditCEP_textChanged(const QString &cep) {
  if (not ui->lineEditCEP->isValid()) {
    return;
  }

  ui->lineEditNro->clear();
  ui->lineEditComp->clear();

  CepCompleter cc;

  if (not cc.buscaCEP(cep)) {
    QMessageBox::warning(this, "Aviso!", "CEP não encontrado!");
    return;
  }

  ui->lineEditUF->setText(cc.getUf());
  ui->lineEditCidade->setText(cc.getCidade());
  ui->lineEditLogradouro->setText(cc.getEndereco());
  ui->lineEditBairro->setText(cc.getBairro());
}

void CadastroTransportadora::on_tableEndereco_clicked(const QModelIndex &index) {
  ui->pushButtonAtualizarEnd->show();
  ui->pushButtonAdicionarEnd->hide();
  mapperEnd.setCurrentModelIndex(index);
}

void CadastroTransportadora::show() {
  QWidget::show();
  adjustSize();
}

void CadastroTransportadora::setupUi() {
  ui->lineEditCNPJ->setInputMask("99.999.999/9999-99;_");
  ui->lineEditANTT->setInputMask("99999999;_");
  ui->lineEditPlaca->setInputMask("AAA-9999;_");
  ui->lineEditCEP->setInputMask("99999-999;_");
  ui->lineEditUF->setInputMask(">AA;_");
}

bool CadastroTransportadora::viewRegister(const QModelIndex index) {
  if (not RegisterDialog::viewRegister(index)) {
    return false;
  }

  modelEnd.setFilter("idTransportadora = " + data(primaryKey).toString() + " AND desativado = FALSE");

  if (not modelEnd.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela endereço transportadora: " + modelEnd.lastError().text());
    return false;
  }

  ui->tableEndereco->resizeColumnsToContents();

  return true;
}
