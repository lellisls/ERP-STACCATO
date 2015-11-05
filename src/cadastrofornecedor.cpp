#include <QMessageBox>
#include <QSqlError>
#include <QDebug>

#include "cadastrofornecedor.h"
#include "ui_cadastrofornecedor.h"
#include "searchdialog.h"
#include "cepcompleter.h"
#include "usersession.h"

CadastroFornecedor::CadastroFornecedor(QWidget *parent)
  : RegisterAddressDialog("fornecedor", "idFornecedor", parent), ui(new Ui::CadastroFornecedor) {
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

CadastroFornecedor::~CadastroFornecedor() { delete ui; }

void CadastroFornecedor::setupTables() {
  ui->tableEndereco->setModel(&modelEnd);
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("idEndereco"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("desativado"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("idFornecedor"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("codUF"));
}

void CadastroFornecedor::setupUi() {
  ui->lineEditContatoCPF->setInputMask("999.999.999-99;_");
  ui->lineEditContatoRG->setInputMask("99.999.999-9;_");
  ui->lineEditIdNextel->setInputMask("99*9999999*99999;_");
  ui->lineEditCNPJ->setInputMask("99.999.999/9999-99;_");
  ui->lineEditCEP->setInputMask("99999-999;_");
  ui->lineEditUF->setInputMask(">AA;_");
}

void CadastroFornecedor::show() {
  QWidget::show();
  adjustSize();
}

void CadastroFornecedor::clearEndereco() {
  ui->lineEditBairro->clear();
  ui->lineEditCEP->clear();
  ui->lineEditCidade->clear();
  ui->lineEditComp->clear();
  ui->lineEditEndereco->clear();
  ui->lineEditNro->clear();
  ui->lineEditUF->clear();
}

void CadastroFornecedor::novoEndereco() {
  ui->pushButtonAtualizarEnd->hide();
  ui->pushButtonAdicionarEnd->show();
  ui->tableEndereco->clearSelection();
  clearEndereco();
}

bool CadastroFornecedor::verifyFields() {
  if (modelEnd.rowCount() == 0) {
    incompleto = true;
    QMessageBox::critical(this, "Erro!", "Faltou endereço!");
    return true;
  }

  int ok = 0;

  for (auto *line : ui->groupBoxContatos->findChildren<QLineEdit *>(QString() , Qt::FindDirectChildrenOnly)) {
    if (verifyRequiredField(line, true)) {
      ok++;
    } else {
      QMessageBox::critical(this, "Erro!", "Faltou: " + line->objectName());
      // TODO: return?
    }
  }

  if (ok != ui->groupBoxContatos->findChildren<QLineEdit *>(QString() , Qt::FindDirectChildrenOnly).size()) {
    incompleto = true;
    return true;
  }

  ok = 0;

  for (auto *line : ui->groupBoxPJuridica->findChildren<QLineEdit *>(QString() , Qt::FindDirectChildrenOnly)) {
    if (verifyRequiredField(line, true)) {
      ok++;
    } else {
      QMessageBox::critical(this, "Erro!", "Faltou: " + line->objectName());
      // TODO: return?
    }
  }

  if (ok != ui->groupBoxPJuridica->findChildren<QLineEdit *>(QString() , Qt::FindDirectChildrenOnly).size()) {
    incompleto = true;
    return true;
  }

  return true;
}

bool CadastroFornecedor::savingProcedures(const int row) {
  if (not setData(row, "razaoSocial", ui->lineEditFornecedor->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Razão Social!");
    return false;
  }

  if (not setData(row, "nomeFantasia", ui->lineEditNomeFantasia->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Nome Fantasia!");
    return false;
  }

  if (not setData(row, "contatoNome", ui->lineEditContatoNome->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Nome Contato!");
    return false;
  }

  if (not ui->lineEditContatoCPF->text().remove(".").remove("-").isEmpty() and
      not setData(row, "contatoCPF", ui->lineEditContatoCPF->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando CPF Contato!");
    return false;
  }

  if (not setData(row, "contatoApelido", ui->lineEditContatoApelido->text())) {
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

  if (not setData(row, "inscEstadual", ui->lineEditInscEstadual->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Insc. Estadual!");
    return false;
  }

  if (not setData(row, "tel", ui->lineEditTel_Res->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Telefone!");
    return false;
  }

  if (not setData(row, "telCel", ui->lineEditTel_Cel->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Celular!");
    return false;
  }

  if (not setData(row, "telCom", ui->lineEditTel_Com->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Tel. Comercial!");
    return false;
  }

  if (not setData(row, "nextel", ui->lineEditNextel->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Nextel!");
    return false;
  }

  if (not setData(row, "email", ui->lineEditEmail->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando E-mail!");
    return false;
  }

  if (not setData(row, "incompleto", incompleto)) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Incompleto!");
    return false;
  }

  return true;
}

void CadastroFornecedor::clearFields() {
  RegisterDialog::clearFields();
  novoEndereco();
  setupUi();
}

void CadastroFornecedor::setupMapper() {
  addMapping(ui->lineEditFornecedor, "razaoSocial");
  addMapping(ui->lineEditContatoNome, "contatoNome");
  addMapping(ui->lineEditContatoApelido, "contatoApelido");
  addMapping(ui->lineEditContatoRG, "contatoRG");
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
  mapperEnd.addMapping(ui->lineEditEndereco, modelEnd.fieldIndex("logradouro"));
  mapperEnd.addMapping(ui->lineEditNro, modelEnd.fieldIndex("numero"));
  mapperEnd.addMapping(ui->lineEditComp, modelEnd.fieldIndex("complemento"));
  mapperEnd.addMapping(ui->lineEditBairro, modelEnd.fieldIndex("bairro"));
  mapperEnd.addMapping(ui->lineEditCidade, modelEnd.fieldIndex("cidade"));
  mapperEnd.addMapping(ui->lineEditUF, modelEnd.fieldIndex("uf"));
}

void CadastroFornecedor::registerMode() {
  ui->pushButtonCadastrar->show();
  ui->pushButtonAtualizar->hide();
  ui->pushButtonRemover->hide();
}

void CadastroFornecedor::updateMode() {
  ui->pushButtonCadastrar->hide();
  ui->pushButtonAtualizar->show();
  ui->pushButtonRemover->show();
}

bool CadastroFornecedor::verifyRequiredField(QLineEdit *line, const bool silent) {
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
      // TODO: usar accesibleName?
      QMessageBox::critical(this, "Erro!", "Você não preencheu um campo obrigatório: " + line->objectName());
      line->setFocus();
    }

    return false;
  }

  return true;
}

void CadastroFornecedor::on_pushButtonCadastrar_clicked() { save(); }

void CadastroFornecedor::on_pushButtonAtualizar_clicked() { update(); }

void CadastroFornecedor::on_pushButtonBuscar_clicked() {
  SearchDialog *sdFornecedor = SearchDialog::fornecedor(this);
  sdFornecedor->show();
  connect(sdFornecedor, &SearchDialog::itemSelected, this, &CadastroFornecedor::viewRegisterById);
}

void CadastroFornecedor::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroFornecedor::on_pushButtonRemover_clicked() { remove(); }

void CadastroFornecedor::on_pushButtonCancelar_clicked() { close(); }

void CadastroFornecedor::on_lineEditCNPJ_textEdited(const QString &text) {
  ui->lineEditCNPJ->setStyleSheet(
        validaCNPJ(QString(text).remove(".").remove("/").remove("-")) ? "" : "color: rgb(255, 0, 0);");
}

void CadastroFornecedor::on_lineEditContatoCPF_textEdited(const QString &text) {
  ui->lineEditContatoCPF->setStyleSheet(validaCPF(QString(text).remove(".").remove("-")) ? ""
                                                                                         : "color: rgb(255, 0, 0);");
}

void CadastroFornecedor::on_pushButtonAdicionarEnd_clicked() {
  cadastrarEndereco(false)
      ? novoEndereco()
      : static_cast<void>(QMessageBox::critical(this, "Erro!", "Não foi possível cadastrar este endereço."));
}

bool CadastroFornecedor::cadastrarEndereco(const bool isUpdate) {
  if (not RegisterDialog::verifyFields({ui->lineEditCEP, ui->lineEditEndereco, ui->lineEditNro, ui->lineEditBairro,
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

  if (not ui->lineEditEndereco->text().isEmpty() and not setDataEnd(row, "logradouro", ui->lineEditEndereco->text())) {
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

void CadastroFornecedor::on_lineEditCEP_textChanged(const QString &cep) {
  if (not ui->lineEditCEP->isValid()) {
    return;
  }

  ui->lineEditNro->clear();
  ui->lineEditComp->clear();

  CepCompleter cc;

  if (cc.buscaCEP(cep)) {
    ui->lineEditUF->setText(cc.getUf());
    ui->lineEditCidade->setText(cc.getCidade());
    ui->lineEditEndereco->setText(cc.getEndereco());
    ui->lineEditBairro->setText(cc.getBairro());
  } else {
    QMessageBox::warning(this, "Aviso!", "CEP não encontrado!");
  }
}

void CadastroFornecedor::on_pushButtonAtualizarEnd_clicked() {
  cadastrarEndereco(true)
      ? novoEndereco()
      : static_cast<void>(QMessageBox::critical(this, "Erro!", "Não foi possível atualizar este endereço!"));
}

void CadastroFornecedor::on_tableEndereco_clicked(const QModelIndex &index) {
  ui->pushButtonAtualizarEnd->show();
  ui->pushButtonAdicionarEnd->hide();
  mapperEnd.setCurrentModelIndex(index);
}

bool CadastroFornecedor::viewRegister(const QModelIndex index) {
  if (not RegisterDialog::viewRegister(index)) {
    return false;
  }

  modelEnd.setFilter("idFornecedor = " + data(primaryKey).toString() + " AND desativado = FALSE");

  if (not modelEnd.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela endereço do fornecedor: " + modelEnd.lastError().text());
    return false;
  }

  ui->tableEndereco->resizeColumnsToContents();

  return true;
}
