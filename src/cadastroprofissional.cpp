#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

#include "cadastroprofissional.h"
#include "cepcompleter.h"
#include "itembox.h"
#include "ui_cadastroprofissional.h"
#include "usersession.h"

CadastroProfissional::CadastroProfissional(QWidget *parent)
  : RegisterAddressDialog("profissional", "idProfissional", parent), ui(new Ui::CadastroProfissional) {
  ui->setupUi(this);

  for (const auto *line : findChildren<QLineEdit *>()) {
    connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);
  }

  setWindowModality(Qt::NonModal);

  setupTables();
  setupUi();
  setupMapper();
  newRegister();

  if (UserSession::tipoUsuario() != "ADMINISTRADOR") {
    ui->tabWidget->setTabEnabled(1, false);
    ui->pushButtonRemover->setDisabled(true);
  }
}

CadastroProfissional::~CadastroProfissional() { delete ui; }

void CadastroProfissional::setupTables() {
  ui->tableEndereco->setModel(&modelEnd);
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("idEndereco"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("desativado"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("idProfissional"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("codUF"));
}

void CadastroProfissional::setupUi() {
  ui->lineEditCEP->setInputMask("99999-999;_");
  ui->lineEditUF->setInputMask(">AA;_");
  ui->lineEditCPF->setInputMask("999.999.999-99;_");
  ui->lineEditContatoCPF->setInputMask("999.999.999-99;_");
  ui->lineEditContatoRG->setInputMask("99.999.999-9;_");
  ui->lineEditIdNextel->setInputMask("99*9999999*99999;_");
  ui->lineEditCNPJ->setInputMask("99.999.999/9999-99;_");
  ui->lineEditCPFBancario->setInputMask("999.999.999-99;_");
  ui->lineEditAgencia->setInputMask("9999-9;_");
}

void CadastroProfissional::setupMapper() {
  addMapping(ui->comboBoxTipo, "tipoProf");
  addMapping(ui->lineEditAgencia, "agencia");
  addMapping(ui->lineEditBanco, "banco");
  addMapping(ui->lineEditCC, "cc");
  addMapping(ui->lineEditCNPJ, "cnpj");
  addMapping(ui->lineEditContatoApelido, "contatoApelido");
  addMapping(ui->lineEditContatoApelido, "contatoApelido");
  addMapping(ui->lineEditContatoCPF, "contatoCPF");
  addMapping(ui->lineEditContatoNome, "contatoNome");
  addMapping(ui->lineEditContatoNome, "contatoNome");
  addMapping(ui->lineEditContatoRG, "contatoRG");
  addMapping(ui->lineEditContatoRG, "contatoRG");
  addMapping(ui->lineEditCPF, "cpf");
  addMapping(ui->lineEditCPFBancario, "cpfBanco");
  addMapping(ui->lineEditEmail, "email");
  addMapping(ui->lineEditIdNextel, "idNextel");
  addMapping(ui->lineEditInscEstadual, "inscEstadual");
  addMapping(ui->lineEditNextel, "nextel");
  addMapping(ui->lineEditNomeBancario, "nomeBanco");
  addMapping(ui->lineEditNomeFantasia, "nomeFantasia");
  addMapping(ui->lineEditProfissional, "nome_razao");
  addMapping(ui->lineEditTel_Cel, "telCel");
  addMapping(ui->lineEditTel_Com, "telCom");
  addMapping(ui->lineEditTel_Res, "tel");

  mapperEnd.addMapping(ui->comboBoxTipoEnd, modelEnd.fieldIndex("descricao"));
  mapperEnd.addMapping(ui->lineEditBairro, modelEnd.fieldIndex("bairro"));
  mapperEnd.addMapping(ui->lineEditCEP, modelEnd.fieldIndex("CEP"));
  mapperEnd.addMapping(ui->lineEditCidade, modelEnd.fieldIndex("cidade"));
  mapperEnd.addMapping(ui->lineEditComp, modelEnd.fieldIndex("complemento"));
  mapperEnd.addMapping(ui->lineEditLogradouro, modelEnd.fieldIndex("logradouro"));
  mapperEnd.addMapping(ui->lineEditNro, modelEnd.fieldIndex("numero"));
  mapperEnd.addMapping(ui->lineEditUF, modelEnd.fieldIndex("uf"));
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

bool CadastroProfissional::viewRegister(const QModelIndex &index) {
  if (not RegisterDialog::viewRegister(index)) return false;

  modelEnd.setFilter("idProfissional = " + data("idProfissional").toString() + " AND desativado = FALSE");

  if (not modelEnd.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela endereço profissional: " + modelEnd.lastError().text());
    return false;
  }

  ui->tableEndereco->resizeColumnsToContents();

  tipoPFPJ = data("pfpj").toString();

  tipoPFPJ == "PF" ? ui->radioButtonPF->setChecked(true) : ui->radioButtonPJ->setChecked(true);

  return true;
}

bool CadastroProfissional::verifyFields() {
  for (auto const &line : ui->frame->findChildren<QLineEdit *>()) {
    if (not verifyRequiredField(line)) return false;
  }

  if (ui->radioButtonPF->isChecked() and ui->lineEditCPF->styleSheet().contains("color: rgb(255, 0, 0)")) {
    QMessageBox::critical(this, "Erro!", "CPF inválido!");
    return false;
  }

  if (ui->radioButtonPJ->isChecked() and ui->lineEditCNPJ->styleSheet().contains("color: rgb(255, 0, 0)")) {
    QMessageBox::critical(this, "Erro!", "CNPJ inválido!");
    return false;
  }

  return true;
}

bool CadastroProfissional::savingProcedures() {
  if (not setData("nome_razao", ui->lineEditProfissional->text())) return false;
  if (not setData("nomeFantasia", ui->lineEditNomeFantasia->text())) return false;
  if (not setData("cpf", ui->lineEditCPF->text())) return false;
  if (not setData("contatoNome", ui->lineEditContatoNome->text())) return false;
  if (not setData("contatoCPF", ui->lineEditContatoCPF->text())) return false;
  if (not setData("contatoApelido", ui->lineEditContatoApelido->text())) return false;
  if (not setData("contatoRG", ui->lineEditContatoRG->text())) return false;
  if (not setData("cnpj", ui->lineEditCNPJ->text())) return false;
  if (not setData("inscEstadual", ui->lineEditInscEstadual->text())) return false;
  if (not setData("tel", ui->lineEditTel_Res->text())) return false;
  if (not setData("telCel", ui->lineEditTel_Cel->text())) return false;
  if (not setData("telCom", ui->lineEditTel_Com->text())) return false;
  if (not setData("nextel", ui->lineEditNextel->text())) return false;
  if (not setData("email", ui->lineEditEmail->text())) return false;
  if (not setData("pfpj", tipoPFPJ)) return false;
  if (not setData("tipoProf", ui->comboBoxTipo->currentText())) return false;
  // Dados bancários
  if (not setData("banco", ui->lineEditBanco->text())) return false;
  if (not setData("agencia", ui->lineEditAgencia->text())) return false;
  if (not setData("cc", ui->lineEditCC->text())) return false;
  if (not setData("nomeBanco", ui->lineEditNomeBancario->text())) return false;
  if (not setData("cpfBanco", ui->lineEditCPFBancario->text())) return false;

  return true;
}

void CadastroProfissional::on_pushButtonCadastrar_clicked() { save(); }

void CadastroProfissional::on_pushButtonAtualizar_clicked() { update(); }

void CadastroProfissional::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroProfissional::on_pushButtonRemover_clicked() { remove(); }

void CadastroProfissional::clearFields() {
  RegisterDialog::clearFields();

  ui->radioButtonPF->setChecked(true);
  novoEndereco();

  for (auto const &box : findChildren<ItemBox *>()) {
    box->clear();
  }

  setupUi();

  ui->comboBoxTipo->setCurrentIndex(0);
}

void CadastroProfissional::novoEndereco() {
  ui->pushButtonAdicionarEnd->show();
  ui->pushButtonAtualizarEnd->hide();
  ui->tableEndereco->clearSelection();
  clearEndereco();
}

void CadastroProfissional::clearEndereco() {
  ui->lineEditBairro->clear();
  ui->lineEditCEP->clear();
  ui->lineEditCidade->clear();
  ui->lineEditComp->clear();
  ui->lineEditLogradouro->clear();
  ui->lineEditNro->clear();
  ui->lineEditUF->clear();
}

void CadastroProfissional::on_pushButtonCancelar_clicked() { close(); }

void CadastroProfissional::on_pushButtonBuscar_clicked() {
  SearchDialog *sdProfissional = SearchDialog::profissional(this);
  sdProfissional->setFilter("idProfissional NOT IN (1) AND desativado = FALSE");
  connect(sdProfissional, &SearchDialog::itemSelected, this, &CadastroProfissional::viewRegisterById);
  sdProfissional->show();
}

void CadastroProfissional::on_lineEditCPF_textEdited(const QString &text) {
  ui->lineEditCPF->setStyleSheet(validaCPF(QString(text).remove(".").remove("-")) ? "" : "color: rgb(255, 0, 0)");
}

void CadastroProfissional::on_lineEditCNPJ_textEdited(const QString &text) {
  ui->lineEditCNPJ->setStyleSheet(
        validaCNPJ(QString(text).remove(".").remove("/").remove("-")) ? "" : "color: rgb(255, 0, 0)");
}

bool CadastroProfissional::cadastrarEndereco(const bool &isUpdate) {
  for (auto const &line : ui->groupBoxEndereco->findChildren<QLineEdit *>()) {
    if (not verifyRequiredField(line)) return false;
  }

  if (not ui->lineEditCEP->isValid()) {
    ui->lineEditCEP->setFocus();
    QMessageBox::critical(this, "Erro!", "CEP inválido!");
    return false;
  }

  rowEnd = isUpdate ? mapperEnd.currentIndex() : modelEnd.rowCount();

  if (not isUpdate) modelEnd.insertRow(rowEnd);

  if (not setDataEnd("descricao", ui->comboBoxTipoEnd->currentText())) return false;
  if (not setDataEnd("CEP", ui->lineEditCEP->text())) return false;
  if (not setDataEnd("logradouro", ui->lineEditLogradouro->text())) return false;
  if (not setDataEnd("numero", ui->lineEditNro->text())) return false;
  if (not setDataEnd("complemento", ui->lineEditComp->text())) return false;
  if (not setDataEnd("bairro", ui->lineEditBairro->text())) return false;
  if (not setDataEnd("cidade", ui->lineEditCidade->text())) return false;
  if (not setDataEnd("uf", ui->lineEditUF->text())) return false;
  if (not setDataEnd("codUF", getCodigoUF(ui->lineEditUF->text()))) return false;
  if (not setDataEnd("desativado", false)) return false;

  ui->tableEndereco->resizeColumnsToContents();

  return true;
}

void CadastroProfissional::on_pushButtonAdicionarEnd_clicked() {
  cadastrarEndereco(false)
      ? novoEndereco()
      : static_cast<void>(QMessageBox::critical(this, "Erro!", "Não foi possível cadastrar este endereço!"));
}

void CadastroProfissional::on_pushButtonAtualizarEnd_clicked() {
  cadastrarEndereco(true)
      ? novoEndereco()
      : static_cast<void>(QMessageBox::critical(this, "Erro!", "Não foi possível atualizar este endereço!"));
}

void CadastroProfissional::on_lineEditCEP_textChanged(const QString &cep) {
  if (not ui->lineEditCEP->isValid()) return;

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

void CadastroProfissional::on_pushButtonEndLimpar_clicked() { novoEndereco(); }

void CadastroProfissional::on_tableEndereco_clicked(const QModelIndex &index) {
  ui->pushButtonAtualizarEnd->show();
  ui->pushButtonAdicionarEnd->hide();
  mapperEnd.setCurrentModelIndex(index);
}

void CadastroProfissional::on_lineEditContatoCPF_textEdited(const QString &text) {
  ui->lineEditContatoCPF->setStyleSheet(validaCPF(QString(text).remove(".").remove("-")) ? ""
                                                                                         : "color: rgb(255, 0, 0)");
}

void CadastroProfissional::on_checkBoxMostrarInativos_clicked(const bool &checked) {
  modelEnd.setFilter("idProfissional = " + data("idProfissional").toString() +
                     (checked ? "" : " AND desativado = FALSE"));
}

void CadastroProfissional::on_pushButtonRemoverEnd_clicked() {
  QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Tem certeza que deseja remover?",
                     QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Remover");
  msgBox.setButtonText(QMessageBox::No, "Voltar");

  if (msgBox.exec() == QMessageBox::Yes) {
    if (not setDataEnd("desativado", true)) {
      QMessageBox::critical(this, "Erro!", "Erro marcando desativado!");
      return;
    }

    if (not modelEnd.submitAll()) {
      QMessageBox::critical(this, "Erro!", "Não foi possível remover este item: " + modelEnd.lastError().text());
      return;
    }

    novoEndereco();
  }
}

void CadastroProfissional::on_radioButtonPF_toggled(const bool &checked) {
  tipoPFPJ = checked ? "PF" : "PJ";
  ui->lineEditCNPJ->setHidden(checked);
  ui->labelCNPJ->setHidden(checked);
  ui->lineEditCPF->setVisible(checked);
  ui->labelCPF->setVisible(checked);
  ui->lineEditInscEstadual->setHidden(checked);
  ui->labelInscricaoEstadual->setHidden(checked);
  checked ? ui->lineEditCNPJ->clear() : ui->lineEditCPF->clear();

  adjustSize();
}

void CadastroProfissional::on_lineEditCPFBancario_textEdited(const QString &text) {
  ui->lineEditCPFBancario->setStyleSheet(validaCPF(QString(text).remove(".").remove("-")) ? ""
                                                                                          : "color: rgb(255, 0, 0)");
}

void CadastroProfissional::successMessage() {
  QMessageBox::information(this, "Atenção!", "Profissional cadastrado com sucesso!");
}
