#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

#include "cadastrocliente.h"
#include "ui_cadastrocliente.h"
#include "cepcompleter.h"
#include "cadastroprofissional.h"
#include "usersession.h"

CadastroCliente::CadastroCliente(QWidget *parent)
  : RegisterAddressDialog("cliente", "idCliente", parent), ui(new Ui::CadastroCliente) {
  ui->setupUi(this);

  SearchDialog *sdCliente = SearchDialog::cliente(ui->itemBoxCliente);
  ui->itemBoxCliente->setSearchDialog(sdCliente);

  SearchDialog *sdProfissional = SearchDialog::profissional(this);
  ui->itemBoxProfissional->setSearchDialog(sdProfissional);

  RegisterDialog *regProfissional = new CadastroProfissional(this);
  ui->itemBoxProfissional->setRegisterDialog(regProfissional);

  SearchDialog *sdVendedor = SearchDialog::usuario(this);
  ui->itemBoxVendedor->setSearchDialog(sdVendedor);

  setupTables();
  setupMapper();
  newRegister();

  for (const auto *line : findChildren<QLineEdit *>()) {
    connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);
  }

  if (UserSession::getTipoUsuario() != "ADMINISTRADOR") {
    ui->pushButtonRemover->setDisabled(true);
    ui->pushButtonRemoverEnd->setDisabled(true);
  }
}

void CadastroCliente::setupUi() {
  ui->lineEditCPF->setInputMask("999.999.999-99;_");
  ui->lineEditContatoCPF->setInputMask("999.999.999-99;_");
  ui->lineEditContatoRG->setInputMask("99.999.999-9;_");
  ui->lineEditIdNextel->setInputMask("99*9999999*99999;_");
  ui->lineEditCNPJ->setInputMask("99.999.999/9999-99;_");
  ui->lineEditCEP->setInputMask("99999-999;_");
  ui->lineEditUF->setInputMask(">AA;_");
}

CadastroCliente::~CadastroCliente() { delete ui; }

void CadastroCliente::setupTables() {
  ui->tableEndereco->setModel(&modelEnd);
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("idEndereco"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("desativado"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("idCliente"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("codUF"));
}

bool CadastroCliente::verifyRequiredField(QLineEdit *line, const bool silent) {
  if (line->styleSheet() != requiredStyle()) {
    return true;
  }

  if (not line->isVisible()) {
    return true;
  }

  if ((line->text().isEmpty()) or (line->text() == "0,00") or (line->text() == "../-") or
      (line->text().size() < line->inputMask().remove(";").remove(">").remove("_").size()) or
      (line->text().size() < line->placeholderText().size() - 1)) {
    if (not silent) { // TODO: use acessibleName?
      QMessageBox::critical(this, "Erro!", "Você não preencheu um campo obrigatório: " + line->objectName());
      line->setFocus();
    }

    return false;
  }

  return true;
}

bool CadastroCliente::verifyFields() {
  if (modelEnd.rowCount() == 0) {
    incompleto = true;
    QMessageBox::critical(this, "Erro!", "Faltou endereço!");
    return true;
  }

  int ok = 0;

  for (auto *line : ui->groupBoxContatos->findChildren<QLineEdit *>()) {
    // TODO: utilizar acessibleName?
    verifyRequiredField(line, true) ? ok++ : QMessageBox::critical(this, "Erro!", "Faltou " + line->objectName());
  }

  if (ok != ui->groupBoxContatos->findChildren<QLineEdit *>().size()) {
    incompleto = true;
    return true;
  }

  ok = 0;

  for (auto *line : ui->groupBoxPJuridica->findChildren<QLineEdit *>()) {
    // TODO: utilizar acessibleName?
    verifyRequiredField(line, true) ? ok++ : QMessageBox::critical(this, "Erro!", "Faltou " + line->objectName());
  }

  if (ok != ui->groupBoxPJuridica->findChildren<QLineEdit *>().size()) {
    incompleto = true;
    return true;
  }

  if (ui->lineEditCliente->text().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Faltou: \"" + ui->lineEditCliente->accessibleName() + "\"");
    return false;
  }

  return true;
}

bool CadastroCliente::savingProcedures(const int row) {
  if (not ui->lineEditCliente->text().isEmpty() and not setData(row, "nome_razao", ui->lineEditCliente->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Nome/Razão.");
    return false;
  }

  if (not ui->lineEditNomeFantasia->text().isEmpty() and
      not setData(row, "nomeFantasia", ui->lineEditNomeFantasia->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Nome Fantasia.");
    return false;
  }

  if (not ui->lineEditCPF->text().remove(".").remove("-").isEmpty() and
      not setData(row, "cpf", ui->lineEditCPF->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando CPF.");
    return false;
  }

  if (not ui->lineEditContatoNome->text().isEmpty() and
      not setData(row, "contatoNome", ui->lineEditContatoNome->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Nome do Contato.");
    return false;
  }

  if (not ui->lineEditContatoCPF->text().remove(".").remove("-").isEmpty() and
      not setData(row, "contatoCPF", ui->lineEditContatoCPF->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando CPF do Contato.");
    return false;
  }

  if (not ui->lineEditContatoApelido->text().isEmpty() and
      not setData(row, "contatoApelido", ui->lineEditContatoApelido->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Apelido do Contato.");
    return false;
  }

  if (not ui->lineEditContatoRG->text().remove(".").remove("-").isEmpty() and
      not setData(row, "contatoRG", ui->lineEditContatoRG->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando RG do Contato.");
    return false;
  }

  if (not ui->lineEditCNPJ->text().remove(".").remove("/").remove("-").isEmpty() and
      not setData(row, "cnpj", ui->lineEditCNPJ->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando CNPJ.");
    return false;
  }

  if (not ui->lineEditInscEstadual->text().isEmpty() and
      not setData(row, "inscEstadual", ui->lineEditInscEstadual->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Insc. Estadual.");
    return false;
  }

  if (ui->dateEdit->date().toString("dd-MM-yyyy") != "01-01-1900") {
    if (not setData(row, "dataNasc", ui->dateEdit->date().toString("yyyy-MM-dd"))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando Data Nasc.");
      return false;
    }
  }

  if (not ui->lineEditTel_Res->text().isEmpty() and not setData(row, "tel", ui->lineEditTel_Res->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Telefone.");
    return false;
  }

  if (not ui->lineEditTel_Cel->text().isEmpty() and not setData(row, "telCel", ui->lineEditTel_Cel->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Celular.");
    return false;
  }

  if (not ui->lineEditTel_Com->text().isEmpty() and not setData(row, "telCom", ui->lineEditTel_Com->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Tel. Comercial.");
    return false;
  }

  if (not ui->lineEditNextel->text().isEmpty() and not setData(row, "nextel", ui->lineEditNextel->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Nextel.");
    return false;
  }

  if (not ui->lineEditEmail->text().isEmpty() and not setData(row, "email", ui->lineEditEmail->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando E-mail.");
    return false;
  }

  if (not setData(row, "idCadastroRel", ui->itemBoxCliente->value())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Cliente Relacionado.");
    return false;
  }

  if (not setData(row, "idProfissionalRel", ui->itemBoxProfissional->value())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Profissional Relacionado.");
    return false;
  }

  if (not setData(row, "idUsuarioRel", ui->itemBoxVendedor->value())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Vendedor Relacionado.");
    return false;
  }

  if (not setData(row, "pfpj", tipoPFPJ)) {
    QMessageBox::critical(this, "Erro!", "Erro guardando PF/PJ.");
    return false;
  }

  if (not setData(row, "incompleto", incompleto)) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Incompleto.");
    return false;
  }

  return true;
}

void CadastroCliente::clearFields() {
  RegisterDialog::clearFields();

  ui->radioButtonPF->setChecked(true);
  novoEndereco();

  for (auto *box : this->findChildren<ItemBox *>()) {
    box->clear();
  }

  setupUi();
}

void CadastroCliente::setupMapper() {
  addMapping(ui->lineEditCliente, "nome_razao");
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
  addMapping(ui->itemBoxCliente, "idCadastroRel", "value");
  addMapping(ui->itemBoxProfissional, "idProfissionalRel", "value");
  addMapping(ui->itemBoxVendedor, "idUsuarioRel", "value");
  addMapping(ui->dateEdit, "dataNasc");

  mapperEnd.addMapping(ui->comboBoxTipoEnd, modelEnd.fieldIndex("descricao"));
  mapperEnd.addMapping(ui->lineEditCEP, modelEnd.fieldIndex("CEP"));
  mapperEnd.addMapping(ui->lineEditLogradouro, modelEnd.fieldIndex("logradouro"));
  mapperEnd.addMapping(ui->lineEditNro, modelEnd.fieldIndex("numero"));
  mapperEnd.addMapping(ui->lineEditComp, modelEnd.fieldIndex("complemento"));
  mapperEnd.addMapping(ui->lineEditBairro, modelEnd.fieldIndex("bairro"));
  mapperEnd.addMapping(ui->lineEditCidade, modelEnd.fieldIndex("cidade"));
  mapperEnd.addMapping(ui->lineEditUF, modelEnd.fieldIndex("uf"));
}

void CadastroCliente::registerMode() {
  ui->pushButtonCadastrar->show();
  ui->pushButtonAtualizar->hide();
  ui->pushButtonRemover->hide();
}

void CadastroCliente::updateMode() {
  ui->pushButtonCadastrar->hide();
  ui->pushButtonAtualizar->show();
  ui->pushButtonRemover->show();
}

bool CadastroCliente::viewRegister(const QModelIndex index) {
  if (not RegisterDialog::viewRegister(index)) {
    return false;
  }

  modelEnd.setFilter("idCliente = " + data(primaryKey).toString() + " AND desativado = FALSE");

  if (not modelEnd.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela endereço do cliente: " + modelEnd.lastError().text());
  }

  ui->itemBoxCliente->searchDialog()->setFilter("idCliente NOT IN (" + data(primaryKey).toString() + ")");

  QSqlQuery query;
  query.prepare("SELECT idCliente, nome_razao, nomeFantasia FROM cliente WHERE idCadastroRel = :primaryKey");
  query.bindValue(":primaryKey", data(primaryKey));

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro na query cliente: " + query.lastError().text());
    return false;
  }

  while (query.next()) {
    ui->textEditObservacoes->insertPlainText(query.value("idCliente").toString() + " - " +
                                             query.value("nome_razao").toString() + " - " +
                                             query.value("nomeFantasia").toString() + "\n");
  }

  tipoPFPJ = data("pfpj").toString();

  if (tipoPFPJ == "PJ") {
    ui->radioButtonPJ->setChecked(true);
  }

  if (tipoPFPJ == "PF") {
    ui->radioButtonPF->setChecked(true);
  }

  ui->tableEndereco->resizeColumnsToContents();

  return true;
}

void CadastroCliente::on_pushButtonCadastrar_clicked() { save(); }

void CadastroCliente::on_pushButtonAtualizar_clicked() { update(); }

void CadastroCliente::show() {
  QWidget::show();
  adjustSize();
}

void CadastroCliente::on_pushButtonCancelar_clicked() { close(); }

void CadastroCliente::on_pushButtonRemover_clicked() { remove(); }

void CadastroCliente::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroCliente::on_pushButtonBuscar_clicked() {
  SearchDialog *sdCliente = SearchDialog::cliente(this);
  connect(sdCliente, &SearchDialog::itemSelected, this, &CadastroCliente::viewRegisterById);
  sdCliente->show();
}

void CadastroCliente::on_lineEditCPF_textEdited(const QString &text) {
  ui->lineEditCPF->setStyleSheet(validaCPF(QString(text).remove(".").remove("-")) ? "" : "color: rgb(255, 0, 0);");
}

void CadastroCliente::on_lineEditCNPJ_textEdited(const QString &text) {
  ui->lineEditCNPJ->setStyleSheet(
        validaCNPJ(QString(text).remove(".").remove("/").remove("-")) ? "" : "color: rgb(255, 0, 0);");
}

bool CadastroCliente::cadastrarEndereco(const bool isUpdate) {
  if (not RegisterDialog::verifyFields({ui->lineEditCEP, ui->lineEditLogradouro, ui->lineEditNro, ui->lineEditBairro,
                                       ui->lineEditCidade, ui->lineEditUF})) {
    return false;
  }

  if (not ui->lineEditCEP->isValid()) {
    ui->lineEditCEP->setFocus();
    QMessageBox::warning(this, "Atenção!", "CEP inválido!");
    return false;
  }

  const int row = (isUpdate) ? mapperEnd.currentIndex() : modelEnd.rowCount();

  if (not isUpdate) {
    modelEnd.insertRow(row);
  }

  if (not setDataEnd(row, "descricao", ui->comboBoxTipoEnd->currentText())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando descrição: " + modelEnd.lastError().text());
    return false;
  }

  if (not ui->lineEditCEP->text().isEmpty() and not setDataEnd(row, "cep", ui->lineEditCEP->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando CEP: " + modelEnd.lastError().text());
    return false;
  }

  if (not ui->lineEditLogradouro->text().isEmpty() and
      not setDataEnd(row, "logradouro", ui->lineEditLogradouro->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando logradouro: " + modelEnd.lastError().text());
    return false;
  }

  if (not ui->lineEditNro->text().isEmpty() and not setDataEnd(row, "numero", ui->lineEditNro->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando número: " + modelEnd.lastError().text());
    return false;
  }

  if (not ui->lineEditComp->text().isEmpty() and not setDataEnd(row, "complemento", ui->lineEditComp->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando complemento: " + modelEnd.lastError().text());
    return false;
  }

  if (not ui->lineEditBairro->text().isEmpty() and not setDataEnd(row, "bairro", ui->lineEditBairro->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando bairro: " + modelEnd.lastError().text());
    return false;
  }

  if (not ui->lineEditCidade->text().isEmpty() and not setDataEnd(row, "cidade", ui->lineEditCidade->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando cidade: " + modelEnd.lastError().text());
    return false;
  }

  if (not ui->lineEditUF->text().isEmpty() and not setDataEnd(row, "uf", ui->lineEditUF->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando UF: " + modelEnd.lastError().text());
    return false;
  }

  if (not setDataEnd(row, "codUF", getCodigoUF(ui->lineEditUF->text()))) {
    QMessageBox::critical(this, "Erro!", "Erro guardando codUF: " + modelEnd.lastError().text());
    return false;
  }

  if (not setDataEnd(row, "desativado", false)) {
    QMessageBox::critical(this, "Erro!", "Erro guardando desativado: " + modelEnd.lastError().text());
    return false;
  }

  ui->tableEndereco->resizeColumnsToContents();

  return true;
}

void CadastroCliente::on_pushButtonAdicionarEnd_clicked() {
  if (not cadastrarEndereco(false)) {
    QMessageBox::critical(this, "Erro!", "Não foi possível cadastrar este endereço!");
  } else {
    novoEndereco();
  }
}

void CadastroCliente::on_pushButtonAtualizarEnd_clicked() {
  if (not cadastrarEndereco(true)) {
    QMessageBox::critical(this, "Erro!", "Não foi possível atualizar este endereço!");
  } else {
    novoEndereco();
  }
}

void CadastroCliente::on_lineEditCEP_textChanged(const QString &cep) {
  if (not ui->lineEditCEP->isValid()) {
    return;
  }

  ui->lineEditNro->clear();
  ui->lineEditComp->clear();

  CepCompleter cc;

  if (cc.buscaCEP(cep)) {
    ui->lineEditUF->setText(cc.getUf());
    ui->lineEditCidade->setText(cc.getCidade());
    ui->lineEditLogradouro->setText(cc.getEndereco());
    ui->lineEditBairro->setText(cc.getBairro());
  } else {
    QMessageBox::warning(this, "Aviso!", "CEP não encontrado!");
  }
}

void CadastroCliente::clearEndereco() {
  ui->lineEditBairro->clear();
  ui->lineEditCEP->clear();
  ui->lineEditCidade->clear();
  ui->lineEditComp->clear();
  ui->lineEditLogradouro->clear();
  ui->lineEditNro->clear();
  ui->lineEditUF->clear();
}

void CadastroCliente::novoEndereco() {
  ui->pushButtonAdicionarEnd->show();
  ui->pushButtonAtualizarEnd->hide();
  ui->tableEndereco->clearSelection();
  clearEndereco();
}

void CadastroCliente::on_pushButtonEndLimpar_clicked() { novoEndereco(); }

void CadastroCliente::on_tableEndereco_clicked(const QModelIndex &index) {
  ui->pushButtonAtualizarEnd->show();
  ui->pushButtonAdicionarEnd->hide();
  mapperEnd.setCurrentModelIndex(index);
}

void CadastroCliente::on_radioButtonPF_toggled(const bool checked) {
  if (checked) {
    tipoPFPJ = "PF";
    ui->lineEditCNPJ->hide();
    ui->labelCNPJ->hide();
    ui->lineEditCPF->show();
    ui->labelCPF->show();
    ui->lineEditInscEstadual->hide();
    ui->labelInscricaoEstadual->hide();
    ui->dateEdit->show();
    ui->labelDataNasc->show();

    ui->lineEditCNPJ->clear();
  } else {
    tipoPFPJ = "PJ";
    ui->lineEditCNPJ->show();
    ui->labelCNPJ->show();
    ui->lineEditCPF->hide();
    ui->labelCPF->hide();
    ui->lineEditInscEstadual->show();
    ui->labelInscricaoEstadual->show();
    ui->dateEdit->hide();
    ui->labelDataNasc->hide();

    ui->lineEditCPF->clear();
  }
}

void CadastroCliente::on_lineEditContatoCPF_textEdited(const QString &text) {
  ui->lineEditContatoCPF->setStyleSheet(validaCPF(QString(text).remove(".").remove("-")) ? ""
                                                                                         : "color: rgb(255, 0, 0);");
}

void CadastroCliente::on_checkBoxMostrarInativos_clicked(const bool checked) {
  modelEnd.setFilter("idCliente = " + data(primaryKey).toString() + (checked ? "" : " AND desativado = FALSE"));
}

void CadastroCliente::on_pushButtonRemoverEnd_clicked() {
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
                            "Erro ao ler a tabela de endereço do cliente: " + modelEnd.lastError().text());
      return;
    }

    novoEndereco();
  }
}
