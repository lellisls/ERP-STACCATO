#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "cadastrocliente.h"
#include "cadastroprofissional.h"
#include "cepcompleter.h"
#include "ui_cadastrocliente.h"
#include "usersession.h"

CadastroCliente::CadastroCliente(QWidget *parent)
  : RegisterAddressDialog("cliente", "idCliente", parent), ui(new Ui::CadastroCliente) {
  ui->setupUi(this);

  for (const auto *line : findChildren<QLineEdit *>()) {
    connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);
  }

  ui->itemBoxCliente->setSearchDialog(SearchDialog::cliente(this));
  ui->itemBoxProfissional->setSearchDialog(SearchDialog::profissional(this));
  ui->itemBoxProfissional->setRegisterDialog(new CadastroProfissional(this));
  ui->itemBoxVendedor->setSearchDialog(SearchDialog::vendedor(this));

  setupTables();
  setupMapper();
  newRegister();

  if (UserSession::tipoUsuario() != "ADMINISTRADOR") {
    ui->pushButtonRemover->setDisabled(true);
    ui->pushButtonRemoverEnd->setDisabled(true);
  }

  ui->lineEditCliente->setFocus();
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

bool CadastroCliente::verifyFields() {
  if (modelEnd.rowCount() == 0) incompleto = true;

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

bool CadastroCliente::savingProcedures() {
  if (not setData("nome_razao", ui->lineEditCliente->text())) return false;
  if (not setData("nomeFantasia", ui->lineEditNomeFantasia->text())) return false;
  if (not setData("cpf", ui->lineEditCPF->text())) return false;
  if (not setData("contatoNome", ui->lineEditContatoNome->text())) return false;
  if (not setData("contatoCPF", ui->lineEditContatoCPF->text())) return false;
  if (not setData("contatoApelido", ui->lineEditContatoApelido->text())) return false;
  if (not setData("contatoRG", ui->lineEditContatoRG->text())) return false;
  if (not setData("cnpj", ui->lineEditCNPJ->text())) return false;
  if (not setData("inscEstadual", ui->lineEditInscEstadual->text())) return false;
  if (not setData("dataNasc", ui->dateEdit->date().toString("yyyy-MM-dd"))) return false;
  if (not setData("tel", ui->lineEditTel_Res->text())) return false;
  if (not setData("telCel", ui->lineEditTel_Cel->text())) return false;
  if (not setData("telCom", ui->lineEditTel_Com->text())) return false;
  if (not setData("nextel", ui->lineEditNextel->text())) return false;
  if (not setData("email", ui->lineEditEmail->text())) return false;
  if (not setData("idCadastroRel", ui->itemBoxCliente->value())) return false;
  if (not setData("idProfissionalRel", ui->itemBoxProfissional->value())) return false;
  if (not setData("idUsuarioRel", ui->itemBoxVendedor->value())) return false;
  if (not setData("pfpj", tipoPFPJ)) return false;
  if (not setData("incompleto", incompleto)) return false;

  return true;
}

void CadastroCliente::clearFields() {
  RegisterDialog::clearFields();

  ui->radioButtonPF->setChecked(true);
  novoEndereco();

  for (auto const &box : this->findChildren<ItemBox *>()) {
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

bool CadastroCliente::viewRegister(const QModelIndex &index) {
  if (not RegisterDialog::viewRegister(index)) return false;

  if (data("idCliente").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "idCliente vazio!");
    return false;
  }

  modelEnd.setFilter("idCliente = " + data("idCliente").toString() + " AND desativado = FALSE");

  if (not modelEnd.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela endereço do cliente: " + modelEnd.lastError().text());
  }

  ui->itemBoxCliente->searchDialog()->setFilter("idCliente NOT IN (" + data("idCliente").toString() + ")");

  QSqlQuery query;
  query.prepare("SELECT idCliente, nome_razao, nomeFantasia FROM cliente WHERE idCadastroRel = :idCadastroRel");
  query.bindValue(":idCadastroRel", data("idCliente"));

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

  tipoPFPJ == "PF" ? ui->radioButtonPF->setChecked(true) : ui->radioButtonPJ->setChecked(true);

  ui->tableEndereco->resizeColumnsToContents();

  return true;
}

void CadastroCliente::on_pushButtonCadastrar_clicked() { save(); }

void CadastroCliente::on_pushButtonAtualizar_clicked() { update(); }

void CadastroCliente::show() {
  QWidget::show();
  adjustSize();
}

void CadastroCliente::on_pushButtonRemover_clicked() { remove(); }

void CadastroCliente::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroCliente::on_pushButtonBuscar_clicked() {
  SearchDialog *sdCliente = SearchDialog::cliente(this);
  connect(sdCliente, &SearchDialog::itemSelected, this, &CadastroCliente::viewRegisterById);
  sdCliente->show();
}

void CadastroCliente::on_lineEditCPF_textEdited(const QString &text) {
  ui->lineEditCPF->setStyleSheet(validaCPF(QString(text).remove(".").remove("-")) ? "" : "color: rgb(255, 0, 0);");

  QSqlQuery query;
  query.prepare("SELECT * FROM cliente WHERE cpf = :cpf");
  query.bindValue(":cpf", text);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando CPF: " + query.lastError().text());
    return;
  }

  if (query.first()) {
    QMessageBox::critical(this, "Erro!", "CPF já cadastrado!");
    viewRegisterById(query.value("idCliente"));
  }
}

void CadastroCliente::on_lineEditCNPJ_textEdited(const QString &text) {
  ui->lineEditCNPJ->setStyleSheet(
        validaCNPJ(QString(text).remove(".").remove("/").remove("-")) ? "" : "color: rgb(255, 0, 0);");

  QSqlQuery query;
  query.prepare("SELECT * FROM cliente WHERE cnpj = :cnpj");
  query.bindValue(":cnpj", text);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando CNPJ: " + query.lastError().text());
    return;
  }

  if (query.first()) {
    QMessageBox::critical(this, "Erro!", "CNPJ já cadastrado!");
    viewRegisterById(query.value("idCliente"));
  }
}

bool CadastroCliente::cadastrarEndereco(const bool &isUpdate) { // NOTE: pass this to RegisterDialog?
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
  if (not setDataEnd("cep", ui->lineEditCEP->text())) return false;
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

void CadastroCliente::on_pushButtonAdicionarEnd_clicked() {
  if (not cadastrarEndereco(false)) {
    QMessageBox::critical(this, "Erro!", "Não foi possível cadastrar este endereço!");
    return;
  }

  novoEndereco();
}

void CadastroCliente::on_pushButtonAtualizarEnd_clicked() {
  if (not cadastrarEndereco(true)) {
    QMessageBox::critical(this, "Erro!", "Não foi possível atualizar este endereço!");
    return;
  }

  novoEndereco();
}

void CadastroCliente::on_lineEditCEP_textChanged(const QString &cep) {
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

void CadastroCliente::on_radioButtonPF_toggled(const bool &checked) {
  tipoPFPJ = checked ? "PF" : "PJ";
  ui->lineEditCNPJ->setHidden(checked);
  ui->labelCNPJ->setHidden(checked);
  ui->lineEditCPF->setVisible(checked);
  ui->labelCPF->setVisible(checked);
  ui->lineEditInscEstadual->setHidden(checked);
  ui->labelInscricaoEstadual->setHidden(checked);
  ui->dateEdit->setVisible(checked);
  ui->labelDataNasc->setVisible(checked);
  checked ? ui->lineEditCNPJ->clear() : ui->lineEditCPF->clear();

  adjustSize();
}

void CadastroCliente::on_lineEditContatoCPF_textEdited(const QString &text) {
  ui->lineEditContatoCPF->setStyleSheet(validaCPF(QString(text).remove(".").remove("-")) ? ""
                                                                                         : "color: rgb(255, 0, 0);");
}

void CadastroCliente::on_checkBoxMostrarInativos_clicked(const bool &checked) {
  modelEnd.setFilter("idCliente = " + data("idCliente").toString() + (checked ? "" : " AND desativado = FALSE"));
}

void CadastroCliente::on_pushButtonRemoverEnd_clicked() {
  QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Tem certeza que deseja remover?",
                     QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Sim");
  msgBox.setButtonText(QMessageBox::No, "Não");

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
