#include <QSqlError>
#include <QMessageBox>
#include <QDebug>

#include "cadastroprofissional.h"
#include "ui_cadastroprofissional.h"
#include "usersession.h"
#include "cepcompleter.h"
#include "itembox.h"

CadastroProfissional::CadastroProfissional(QWidget *parent)
  : RegisterAddressDialog("profissional", "idProfissional", parent), ui(new Ui::CadastroProfissional) {
  ui->setupUi(this);

  setWindowModality(Qt::NonModal);

  setupTables();
  setupUi();
  setupMapper();
  newRegister();

  for (const auto *line : findChildren<QLineEdit *>(QString() , Qt::FindDirectChildrenOnly)) {
    connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);
  }

  if (UserSession::getTipoUsuario() != "ADMINISTRADOR") {
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
  addMapping(ui->lineEditBanco, "banco");
  addMapping(ui->lineEditAgencia, "agencia");
  addMapping(ui->lineEditCC, "cc");
  addMapping(ui->lineEditNomeBancario, "nomeBanco");
  addMapping(ui->lineEditCPFBancario, "cpfBanco");
  addMapping(ui->comboBoxTipo, "tipoProf");
  addMapping(ui->lineEditProfissional, "nome_razao");
  addMapping(ui->lineEditContatoNome, "contatoNome");
  addMapping(ui->lineEditContatoApelido, "contatoApelido");
  addMapping(ui->lineEditContatoRG, "contatoRG");
  addMapping(ui->lineEditCPF, "cpf");
  addMapping(ui->lineEditCNPJ, "cnpj");
  addMapping(ui->lineEditNomeFantasia, "nomeFantasia");
  addMapping(ui->lineEditInscEstadual, "inscEstadual");
  addMapping(ui->lineEditTel_Res, "tel");
  addMapping(ui->lineEditTel_Cel, "telCel");
  addMapping(ui->lineEditTel_Com, "telCom");
  addMapping(ui->lineEditIdNextel, "idNextel");
  addMapping(ui->lineEditNextel, "nextel");
  addMapping(ui->lineEditEmail, "email");
  addMapping(ui->lineEditContatoNome, "contatoNome");
  addMapping(ui->lineEditContatoCPF, "contatoCPF");
  addMapping(ui->lineEditContatoApelido, "contatoApelido");
  addMapping(ui->lineEditContatoRG, "contatoRG");

  mapperEnd.addMapping(ui->comboBoxTipoEnd, modelEnd.fieldIndex("descricao"));
  mapperEnd.addMapping(ui->lineEditCEP, modelEnd.fieldIndex("CEP"));
  mapperEnd.addMapping(ui->lineEditLogradouro, modelEnd.fieldIndex("logradouro"));
  mapperEnd.addMapping(ui->lineEditNro, modelEnd.fieldIndex("numero"));
  mapperEnd.addMapping(ui->lineEditComp, modelEnd.fieldIndex("complemento"));
  mapperEnd.addMapping(ui->lineEditBairro, modelEnd.fieldIndex("bairro"));
  mapperEnd.addMapping(ui->lineEditCidade, modelEnd.fieldIndex("cidade"));
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

bool CadastroProfissional::viewRegister(const QModelIndex index) {
  if (not RegisterDialog::viewRegister(index)) {
    return false;
  }

  modelEnd.setFilter("idProfissional = " + data(primaryKey).toString() + " AND desativado = FALSE");

  if (not modelEnd.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela endereço profissional: " + modelEnd.lastError().text());
    return false;
  }

  ui->tableEndereco->resizeColumnsToContents();

  tipoPFPJ = data("pfpj").toString();

  if (tipoPFPJ == "PJ") {
    ui->radioButtonPJ->setChecked(true);
  }

  if (tipoPFPJ == "PF") {
    ui->radioButtonPF->setChecked(true);
  }

  return true;
}

bool CadastroProfissional::verifyRequiredField(QLineEdit *line, const bool silent) {
  if (line->styleSheet() != requiredStyle()) {
    return true;
  }

  if (not line->isVisible()) {
    return true;
  }

  if ((line->text().isEmpty()) or (line->text() == "0,00") or (line->text() == "../-") or
      (line->text().size() < line->inputMask().remove(";").remove(">").remove("_").size()) or
      (line->text().size() < line->placeholderText().size() - 1)) {
    if (not silent) {
      QMessageBox::critical(this, "Erro!", "Você não preencheu um campo obrigatório: " + line->objectName());
      line->setFocus();
    }

    return false;
  }

  return true;
}

bool CadastroProfissional::verifyFields() {
  if (modelEnd.rowCount() == 0) {
    incompleto = true;
    QMessageBox::warning(this, "Aviso!", "Faltou endereço.");
    return true;
  }

  int ok = 0;

  for (auto *line : ui->groupBoxContatos->findChildren<QLineEdit *>(QString() , Qt::FindDirectChildrenOnly)) {
    verifyRequiredField(line, true) ? ok++ : QMessageBox::critical(this, "Erro!", "Faltou: " + line->objectName());
  }

  if (ok != ui->groupBoxContatos->findChildren<QLineEdit *>(QString() , Qt::FindDirectChildrenOnly).size()) {
    incompleto = true;
    return true;
  }

  ok = 0;

  for (auto *line : ui->groupBoxPJuridica->findChildren<QLineEdit *>(QString() , Qt::FindDirectChildrenOnly)) {
    verifyRequiredField(line, true) ? ok++ : QMessageBox::critical(this, "Erro!", "Faltou: " + line->objectName());
  }

  if (ok != ui->groupBoxPJuridica->findChildren<QLineEdit *>(QString() , Qt::FindDirectChildrenOnly).size()) {
    incompleto = true;
    return true;
  }

  return true;
}

bool CadastroProfissional::savingProcedures(const int row) {
  // TODO: unify savingProcedures by sending to registerDialog the widgets (or the texts directly) to save and let it do
  // the setData and error messages

  if (not ui->lineEditProfissional->text().isEmpty() and
      not setData(row, "nome_razao", ui->lineEditProfissional->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Nome/Razão!");
    return false;
  }

  if (not ui->lineEditNomeFantasia->text().isEmpty() and
      not setData(row, "nomeFantasia", ui->lineEditNomeFantasia->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Nome Fantasia!");
    return false;
  }

  if (not ui->lineEditCPF->text().remove(".").remove("-").isEmpty() and
      not setData(row, "cpf", ui->lineEditCPF->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando CPF!");
    return false;
  }

  if (not ui->lineEditContatoNome->text().isEmpty() and
      not setData(row, "contatoNome", ui->lineEditContatoNome->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Nome Contato!");
    return false;
  }

  if (not ui->lineEditContatoCPF->text().remove(".").remove("-").isEmpty() and
      not setData(row, "contatoCPF", ui->lineEditContatoCPF->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando CPF Contato!");
    return false;
  }

  if (not ui->lineEditContatoApelido->text().isEmpty() and
      not setData(row, "contatoApelido", ui->lineEditContatoApelido->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Apelido Contato!");
    return false;
  }

  if (not ui->lineEditContatoRG->text().remove(".").remove("-").isEmpty() and
      not setData(row, "contatoRG", ui->lineEditContatoRG->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando RG Contato!");
    return false;
  }

  if (not ui->lineEditCNPJ->text().remove(".").remove("/").remove("-").isEmpty() and
      not setData(row, "cnpj", ui->lineEditCNPJ->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando CNPJ!");
    return false;
  }

  if (not ui->lineEditInscEstadual->text().isEmpty() and
      not setData(row, "inscEstadual", ui->lineEditInscEstadual->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Insc. Estadual!");
    return false;
  }

  if (not ui->lineEditTel_Res->text().isEmpty() and not setData(row, "tel", ui->lineEditTel_Res->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Telefone!");
    return false;
  }

  if (not ui->lineEditTel_Cel->text().isEmpty() and not setData(row, "telCel", ui->lineEditTel_Cel->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Celular!");
    return false;
  }

  if (not ui->lineEditTel_Com->text().isEmpty() and not setData(row, "telCom", ui->lineEditTel_Com->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Tel. Comercial!");
    return false;
  }

  if (not ui->lineEditNextel->text().isEmpty() and not setData(row, "nextel", ui->lineEditNextel->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Nextel!");
    return false;
  }

  if (not ui->lineEditEmail->text().isEmpty() and not setData(row, "email", ui->lineEditEmail->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando E-mail!");
    return false;
  }

  if (not setData(row, "pfpj", tipoPFPJ)) {
    QMessageBox::critical(this, "Erro!", "Erro guardando PF/PJ!");
    return false;
  }

  if (not setData(row, "tipoProf", ui->comboBoxTipo->currentText())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Tipo Prof.!");
    return false;
  }

  // Dados bancários
  if (not setData(row, "banco", ui->lineEditBanco->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Banco!");
    return false;
  }

  if (not ui->lineEditAgencia->text().remove("-").isEmpty()) {
    if (not setData(row, "agencia", ui->lineEditAgencia->text())) {
      QMessageBox::critical(this, "Erro!", "Erro guardando Agência!");
      return false;
    }
  }

  if (not setData(row, "cc", ui->lineEditCC->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Conta Corrente!");
    return false;
  }

  if (not setData(row, "nomeBanco", ui->lineEditNomeBancario->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Nome Banco!");
    return false;
  }

  if (not ui->lineEditCPFBancario->text().remove(".").remove("-").isEmpty()) {
    if (not setData(row, "cpfBanco", ui->lineEditCPFBancario->text())) {
      QMessageBox::critical(this, "Erro!", "Erro guardando CPF Banco!");
      return false;
    }
  }

  if (not setData(row, "incompleto", incompleto)) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Incompleto!");
    return false;
  }

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

  for (auto *box : this->findChildren<ItemBox *>(QString() , Qt::FindDirectChildrenOnly)) {
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

void CadastroProfissional::show() {
  QWidget::show();
  adjustSize();
}

void CadastroProfissional::on_lineEditCPF_textEdited(const QString &text) {
  ui->lineEditCPF->setStyleSheet(validaCPF(QString(text).remove(".").remove("-")) ? "" : "color: rgb(255, 0, 0);");
}

void CadastroProfissional::on_lineEditCNPJ_textEdited(const QString &text) {
  ui->lineEditCNPJ->setStyleSheet(
        validaCNPJ(QString(text).remove(".").remove("/").remove("-")) ? "" : "color: rgb(255, 0, 0);");
}

bool CadastroProfissional::cadastrarEndereco(const bool isUpdate) {
  if (not RegisterDialog::verifyFields({ui->lineEditCEP, ui->lineEditLogradouro, ui->lineEditNro, ui->lineEditBairro,
                                       ui->lineEditCidade, ui->lineEditUF})) {
    return false;
  }

  if (not ui->lineEditCEP->isValid()) {
    ui->lineEditCEP->setFocus();
    QMessageBox::warning(this, "Atenção!", "CEP inválido!");
    return false;
  }

  const int row = (isUpdate) ? mapperEnd.currentIndex() : model.rowCount();

  if (not isUpdate) {
    modelEnd.insertRow(row);
  }

  if (not modelEnd.setData(row, "descricao", ui->comboBoxTipoEnd->currentText())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Descrição!");
    return false;
  }

  if (not ui->lineEditCEP->text().isEmpty()) {
    if (not modelEnd.setData(row, "CEP", ui->lineEditCEP->text())) {
      QMessageBox::critical(this, "Erro!", "Erro guardando CEP!");
      return false;
    }
  }

  if (not ui->lineEditLogradouro->text().isEmpty()) {
    if (not modelEnd.setData(row, "logradouro", ui->lineEditLogradouro->text())) {
      QMessageBox::critical(this, "Erro!", "Erro guardando Logradouro!");
      return false;
    }
  }

  if (not ui->lineEditNro->text().isEmpty()) {
    if (not modelEnd.setData(row, "numero", ui->lineEditNro->text())) {
      QMessageBox::critical(this, "Erro!", "Erro guardando Número!");
      return false;
    }
  }

  if (not ui->lineEditComp->text().isEmpty()) {
    if (not modelEnd.setData(row, "complemento", ui->lineEditComp->text())) {
      QMessageBox::critical(this, "Erro!", "Erro guardando Complemento!");
      return false;
    }
  }

  if (not ui->lineEditBairro->text().isEmpty()) {
    if (not modelEnd.setData(row, "bairro", ui->lineEditBairro->text())) {
      QMessageBox::critical(this, "Erro!", "Erro guardando Bairro!");
      return false;
    }
  }

  if (not ui->lineEditCidade->text().isEmpty()) {
    if (not modelEnd.setData(row, "cidade", ui->lineEditCidade->text())) {
      QMessageBox::critical(this, "Erro!", "Erro guardando Cidade!");
      return false;
    }
  }

  if (not ui->lineEditUF->text().isEmpty()) {
    if (not modelEnd.setData(row, "uf", ui->lineEditUF->text())) {
      QMessageBox::critical(this, "Erro!", "Erro guardando UF!");
      return false;
    }

    if (not modelEnd.setData(row, "codUF", getCodigoUF(ui->lineEditUF->text()))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando codUF!");
      return false;
    }
  }

  if (not modelEnd.setData(row, "desativado", 0)) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Desativado!");
    return false;
  }

  ui->tableEndereco->resizeColumnsToContents();

  return true;
}

void CadastroProfissional::on_pushButtonAdicionarEnd_clicked() {
  if (not cadastrarEndereco(false)) {
    QMessageBox::critical(this, "Erro!", "Não foi possível cadastrar este endereço.");
    return;
  }

  novoEndereco();
}

void CadastroProfissional::on_pushButtonAtualizarEnd_clicked() {
  if (not cadastrarEndereco(true)) {
    QMessageBox::critical(this, "Erro!", "Não foi possível atualizar este endereço.");
  }

  novoEndereco();
}

void CadastroProfissional::on_lineEditCEP_textChanged(const QString &cep) {
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

void CadastroProfissional::on_pushButtonEndLimpar_clicked() { novoEndereco(); }

void CadastroProfissional::on_tableEndereco_clicked(const QModelIndex &index) {
  ui->pushButtonAtualizarEnd->show();
  ui->pushButtonAdicionarEnd->hide();
  mapperEnd.setCurrentModelIndex(index);
}

void CadastroProfissional::on_lineEditContatoCPF_textEdited(const QString &text) {
  ui->lineEditContatoCPF->setStyleSheet(validaCPF(QString(text).remove(".").remove("-")) ? ""
                                                                                         : "color: rgb(255, 0, 0);");
}

void CadastroProfissional::on_checkBoxMostrarInativos_clicked(const bool checked) {
  modelEnd.setFilter("idProfissional = " + data(primaryKey).toString() + (checked ? "" : " AND desativado = FALSE"));
}

void CadastroProfissional::on_pushButtonRemoverEnd_clicked() {
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
                            "Erro lendo tabela endereço do profissional: " + modelEnd.lastError().text());
      return;
    }

    novoEndereco();
  }
}

void CadastroProfissional::on_radioButtonPF_toggled(const bool checked) {
  if (checked) {
    tipoPFPJ = "PF";
    ui->lineEditCNPJ->hide();
    ui->labelCNPJ->hide();
    ui->lineEditCPF->show();
    ui->labelCPF->show();
    ui->lineEditInscEstadual->hide();
    ui->labelInscricaoEstadual->hide();

    ui->lineEditCNPJ->clear();
  } else {
    tipoPFPJ = "PJ";
    ui->lineEditCNPJ->show();
    ui->labelCNPJ->show();
    ui->lineEditCPF->hide();
    ui->labelCPF->hide();
    ui->lineEditInscEstadual->show();
    ui->labelInscricaoEstadual->show();

    ui->lineEditCPF->clear();
  }

  adjustSize();
}

void CadastroProfissional::on_lineEditCPFBancario_textEdited(const QString &text) {
  ui->lineEditCPFBancario->setStyleSheet(validaCPF(QString(text).remove(".").remove("-")) ? ""
                                                                                          : "color: rgb(255, 0, 0);");
}
