#include <QDebug>
#include <QMessageBox>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>

#include "cadastrocliente.h"
#include "cadastroprofissional.h"
#include "cepcompleter.h"
#include "ui_cadastrocliente.h"
#include "usersession.h"

CadastroCliente::CadastroCliente(QWidget *parent) : RegisterAddressDialog("cliente", "idCliente", parent), ui(new Ui::CadastroCliente) {
  ui->setupUi(this);

  for (auto const *line : findChildren<QLineEdit *>()) {
    connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);
  }

  ui->itemBoxCliente->setSearchDialog(SearchDialog::cliente(this));
  ui->itemBoxProfissional->setSearchDialog(SearchDialog::profissional(this));
  ui->itemBoxProfissional->setRegisterDialog(new CadastroProfissional(this));
  ui->itemBoxVendedor->setSearchDialog(SearchDialog::vendedor(this));

  setupTables();
  setupMapper();
  newRegister();

  sdCliente = SearchDialog::cliente(this);
  connect(sdCliente, &SearchDialog::itemSelected, this, &CadastroCliente::viewRegisterById);

  if (UserSession::tipoUsuario() != "ADMINISTRADOR") {
    ui->pushButtonRemover->setDisabled(true);
    ui->pushButtonRemoverEnd->setDisabled(true);
  }

  ui->lineEditCliente->setFocus();
}

CadastroCliente::~CadastroCliente() { delete ui; }

void CadastroCliente::setupUi() {
  ui->lineEditCPF->setInputMask("999.999.999-99;_");
  ui->lineEditContatoCPF->setInputMask("999.999.999-99;_");
  ui->lineEditContatoRG->setInputMask("99.999.999-9;_");
  ui->lineEditIdNextel->setInputMask("99*9999999*99999;_");
  ui->lineEditCNPJ->setInputMask("99.999.999/9999-99;_");
  ui->lineEditInscEstadual->setValidator(new QRegExpValidator(QRegExp(R"([0-9]\d{0,15})"), this));
  ui->lineEditCEP->setInputMask("99999-999;_");
  ui->lineEditUF->setInputMask(">AA;_");
}

void CadastroCliente::setupTables() {
  ui->tableEndereco->setModel(&modelEnd);
  ui->tableEndereco->hideColumn("idEndereco");
  ui->tableEndereco->hideColumn("desativado");
  ui->tableEndereco->hideColumn("idCliente");
  ui->tableEndereco->hideColumn("codUF");
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

  if (not isUpdate) {
    QSqlQuery query;
    query.prepare("SELECT cpf, cnpj FROM cliente WHERE cpf = :cpf OR cnpj = :cnpj");
    query.bindValue(":cpf", ui->lineEditCPF->text());
    query.bindValue(":cnpj", ui->lineEditCNPJ->text());

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro verificando se CPF/CNPJ já cadastrado!");
      return false;
    }

    if (query.first()) {
      QMessageBox::critical(this, "Erro!", "CPF/CNPJ já cadastrado!");
      return false;
    }
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
  if (not setData("dataNasc", ui->dateEdit->date())) return false;
  if (not setData("tel", ui->lineEditTel_Res->text())) return false;
  if (not setData("telCel", ui->lineEditTel_Cel->text())) return false;
  if (not setData("telCom", ui->lineEditTel_Com->text())) return false;
  if (not setData("nextel", ui->lineEditNextel->text())) return false;
  if (not setData("email", ui->lineEditEmail->text())) return false;
  if (not setData("idCadastroRel", ui->itemBoxCliente->getValue())) return false;
  if (not setData("idProfissionalRel", ui->itemBoxProfissional->getValue())) return false;
  if (not setData("idUsuarioRel", ui->itemBoxVendedor->getValue())) return false;
  if (not setData("pfpj", tipoPFPJ)) return false;
  if (not setData("incompleto", incompleto)) return false;
  if (not setData("credito", ui->doubleSpinBoxCredito->value())) return false;

  return true;
}

void CadastroCliente::clearFields() {
  RegisterDialog::clearFields();

  ui->radioButtonPF->setChecked(true);
  ui->checkBoxInscEstIsento->setChecked(false);
  novoEndereco();

  for (auto const &box : findChildren<ItemBox *>()) box->clear();

  setupUi();
}

void CadastroCliente::setupMapper() {
  addMapping(ui->dateEdit, "dataNasc");
  addMapping(ui->doubleSpinBoxCredito, "credito");
  addMapping(ui->itemBoxCliente, "idCadastroRel", "value");
  addMapping(ui->itemBoxProfissional, "idProfissionalRel", "value");
  addMapping(ui->itemBoxVendedor, "idUsuarioRel", "value");
  addMapping(ui->lineEditCliente, "nome_razao");
  addMapping(ui->lineEditCNPJ, "cnpj");
  addMapping(ui->lineEditContatoApelido, "contatoApelido");
  addMapping(ui->lineEditContatoApelido, "contatoApelido");
  addMapping(ui->lineEditContatoCPF, "contatoCPF");
  addMapping(ui->lineEditContatoNome, "contatoNome");
  addMapping(ui->lineEditContatoNome, "contatoNome");
  addMapping(ui->lineEditContatoRG, "contatoRG");
  addMapping(ui->lineEditContatoRG, "contatoRG");
  addMapping(ui->lineEditCPF, "cpf");
  addMapping(ui->lineEditEmail, "email");
  addMapping(ui->lineEditIdNextel, "idNextel");
  addMapping(ui->lineEditInscEstadual, "inscEstadual");
  addMapping(ui->lineEditNextel, "nextel");
  addMapping(ui->lineEditNomeFantasia, "nomeFantasia");
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

bool CadastroCliente::viewRegister() {
  if (not RegisterDialog::viewRegister()) return false;

  if (data("idCliente").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "idCliente vazio!");
    return false;
  }

  modelEnd.setFilter("idCliente = " + data("idCliente").toString() + " AND desativado = FALSE");

  if (not modelEnd.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela endereço do cliente: " + modelEnd.lastError().text());

  ui->itemBoxCliente->getSearchDialog()->setFilter("idCliente NOT IN (" + data("idCliente").toString() + ")");

  tipoPFPJ = data("pfpj").toString();

  tipoPFPJ == "PF" ? ui->radioButtonPF->setChecked(true) : ui->radioButtonPJ->setChecked(true);

  ui->checkBoxInscEstIsento->setChecked(data("inscEstadual").toString() == "ISENTO");

  ui->tableEndereco->resizeColumnsToContents();

  return true;
}

void CadastroCliente::on_pushButtonCadastrar_clicked() { save(); }

void CadastroCliente::on_pushButtonAtualizar_clicked() { save(); }

void CadastroCliente::on_pushButtonRemover_clicked() { remove(); }

void CadastroCliente::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroCliente::on_pushButtonBuscar_clicked() { sdCliente->show(); }

void CadastroCliente::on_lineEditCPF_textEdited(const QString &text) {
  ui->lineEditCPF->setStyleSheet(validaCPF(QString(text).remove(".").remove("-")) ? "" : "color: rgb(255, 0, 0)");

  if (not ui->lineEditCPF->styleSheet().contains("color: rgb(255, 0, 0)")) {
    QSqlQuery query;
    query.prepare("SELECT idCliente FROM cliente WHERE cpf = :cpf");
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
}

void CadastroCliente::on_lineEditCNPJ_textEdited(const QString &text) {
  ui->lineEditCNPJ->setStyleSheet(validaCNPJ(QString(text).remove(".").remove("/").remove("-")) ? "" : "color: rgb(255, 0, 0)");

  if (not ui->lineEditCNPJ->styleSheet().contains("color: rgb(255, 0, 0)")) {
    QSqlQuery query;
    query.prepare("SELECT idCliente FROM cliente WHERE cnpj = :cnpj");
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
}

bool CadastroCliente::cadastrarEndereco(const bool isUpdate) {
  for (auto const &line : ui->groupBoxEndereco->findChildren<QLineEdit *>()) {
    if (not verifyRequiredField(line)) return false;
  }

  if (not ui->lineEditCEP->isValid()) {
    ui->lineEditCEP->setFocus();
    QMessageBox::critical(this, "Erro!", "CEP inválido!");
    return false;
  }

  if (ui->lineEditNro->text().isEmpty()) {
    ui->lineEditNro->setFocus();
    QMessageBox::critical(this, "Erro!", "Número vazio!");
    return false;
  }

  currentRowEnd = isUpdate ? mapperEnd.currentIndex() : modelEnd.rowCount();

  if (not isUpdate) modelEnd.insertRow(currentRowEnd);

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

  isDirty = true;

  return true;
}

bool CadastroCliente::cadastrar() {
  currentRow = isUpdate ? mapper.currentIndex() : model.rowCount();

  if (currentRow == -1) {
    QMessageBox::critical(this, "Erro!", "Erro linha -1");
    return false;
  }

  if (not isUpdate and not model.insertRow(currentRow)) return false;

  if (not savingProcedures()) return false;

  for (int column = 0; column < model.rowCount(); ++column) {
    const QVariant dado = model.data(currentRow, column);
    if (dado.type() == QVariant::String) {
      if (not model.setData(currentRow, column, dado.toString().toUpper())) return false;
    }
  }

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro: " + model.lastError().text());
    return false;
  }

  primaryId = data(currentRow, primaryKey).isValid() ? data(currentRow, primaryKey).toString() : getLastInsertId().toString();

  if (primaryId.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "primaryId está vazio!");
    return false;
  }

  for (int row = 0, rowCount = modelEnd.rowCount(); row < rowCount; ++row) {
    if (not modelEnd.setData(row, primaryKey, primaryId)) return false;
  }

  for (int column = 0; column < modelEnd.rowCount(); ++column) {
    const QVariant dado = modelEnd.data(currentRow, column);
    if (dado.type() == QVariant::String) {
      if (not modelEnd.setData(currentRow, column, dado.toString().toUpper())) return false;
    }
  }

  if (not modelEnd.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro: " + modelEnd.lastError().text());
    return false;
  }

  return true;
}

void CadastroCliente::on_pushButtonAdicionarEnd_clicked() {
  if (not cadastrarEndereco()) {
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

void CadastroCliente::on_radioButtonPF_toggled(const bool checked) {
  tipoPFPJ = checked ? "PF" : "PJ";

  if (checked) {
    ui->lineEditCNPJ->setHidden(checked);
    ui->labelCNPJ->setHidden(checked);
    ui->lineEditInscEstadual->setHidden(checked);
    ui->labelInscricaoEstadual->setHidden(checked);
    ui->checkBoxInscEstIsento->setHidden(checked);

    ui->lineEditCPF->setVisible(checked);
    ui->labelCPF->setVisible(checked);
    ui->dateEdit->setVisible(checked);
    ui->labelDataNasc->setVisible(checked);
  } else {
    ui->lineEditCPF->setVisible(checked);
    ui->labelCPF->setVisible(checked);
    ui->dateEdit->setVisible(checked);
    ui->labelDataNasc->setVisible(checked);

    ui->lineEditCNPJ->setHidden(checked);
    ui->labelCNPJ->setHidden(checked);
    ui->lineEditInscEstadual->setHidden(checked);
    ui->labelInscricaoEstadual->setHidden(checked);
    ui->checkBoxInscEstIsento->setHidden(checked);
  }

  checked ? ui->lineEditCNPJ->clear() : ui->lineEditCPF->clear();

  adjustSize();
}

void CadastroCliente::on_lineEditContatoCPF_textEdited(const QString &text) { ui->lineEditContatoCPF->setStyleSheet(validaCPF(QString(text).remove(".").remove("-")) ? "" : "color: rgb(255, 0, 0)"); }

void CadastroCliente::on_checkBoxMostrarInativos_clicked(const bool checked) {
  modelEnd.setFilter("idCliente = " + data("idCliente").toString() + (checked ? "" : " AND desativado = FALSE"));

  if (not modelEnd.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela endereço: " + modelEnd.lastError().text());
  }
}

void CadastroCliente::on_pushButtonRemoverEnd_clicked() {
  QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Tem certeza que deseja remover?", QMessageBox::Yes | QMessageBox::No, this);
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

bool CadastroCliente::save() {
  if (not verifyFields()) return false;

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not cadastrar()) {
    QSqlQuery("ROLLBACK").exec();
    return false;
  }

  QSqlQuery("COMMIT").exec();

  isDirty = false;

  viewRegisterById(primaryId);

  successMessage();

  return true;
}

void CadastroCliente::successMessage() { QMessageBox::information(this, "Atenção!", isUpdate ? "Cadastro atualizado!" : "Cliente cadastrado com sucesso!"); }

void CadastroCliente::on_tableEndereco_entered(const QModelIndex &) { ui->tableEndereco->resizeColumnsToContents(); }

void CadastroCliente::on_checkBoxInscEstIsento_toggled(bool checked) {
  if (checked) {
    ui->lineEditInscEstadual->setValidator(nullptr);
    ui->lineEditInscEstadual->setText("ISENTO");
    ui->lineEditInscEstadual->setReadOnly(true);
  } else {
    ui->lineEditInscEstadual->setValidator(new QRegExpValidator(QRegExp(R"([0-9]\d{0,15})"), this));
    ui->lineEditInscEstadual->clear();
    ui->lineEditInscEstadual->setReadOnly(false);
  }
}

// TODO: ao trocar de cadastro nao esta limpando observacao (esta fazendo append)
// TODO: nao deixar cadastrar endereco sem numero, se necessario colocar 'S/N'
