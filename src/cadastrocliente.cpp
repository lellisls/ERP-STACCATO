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
  : RegisterAddressDialog("Cliente", "idCliente", parent), ui(new Ui::CadastroCliente) {
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

  foreach (const QLineEdit *line, findChildren<QLineEdit *>()) {
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

  if ((line->text().isEmpty()) or line->text() == "0,00" or line->text() == "../-" or
      (line->text().size() < (line->inputMask().remove(";").remove(">").remove("_").size()) or
       (line->text().size() < line->placeholderText().size() - 1))) {
    qDebug() << "ObjectName: " << line->parent()->objectName() << ", line: " << line->objectName() << " | "
             << line->text();

    if (not silent) {
      QMessageBox::warning(this, "Atenção!", "Você não preencheu um campo obrigatório!", QMessageBox::Ok,
                           QMessageBox::NoButton);
      line->setFocus();
    }

    return false;
  }

  return true;
}

bool CadastroCliente::verifyFields(const int row) {
  if (modelEnd.rowCount() == 0) {
    setData(row, "incompleto", true);
    qDebug() << "Faltou endereço!";
    return true;
  } else {
    setData(row, "incompleto", false);
  }

  int ok = 0;

  foreach (QLineEdit *line, ui->groupBoxContatos->findChildren<QLineEdit *>()) {
    if (not verifyRequiredField(line, true)) {
      qDebug() << "Faltou " << line->objectName();
    } else {
      ok++;
    }
  }

  if (ok == ui->groupBoxContatos->findChildren<QLineEdit *>().size()) {
    setData(row, "incompleto", false);
  } else {
    setData(row, "incompleto", true);
    return true;
  }

  ok = 0;

  foreach (QLineEdit *line, ui->groupBoxPJuridica->findChildren<QLineEdit *>()) {
    if (not verifyRequiredField(line, true)) {
      qDebug() << "Faltou " << line->objectName();
    } else {
      ok++;
    }
  }

  if (ok == ui->groupBoxPJuridica->findChildren<QLineEdit *>().size()) {
    setData(row, "incompleto", false);
  } else {
    setData(row, "incompleto", true);
  }

  if (ui->lineEditCliente->text().isEmpty()) {
    QMessageBox::warning(this, "Aviso!", "Faltou: \"" + ui->lineEditCliente->accessibleName() + "\"");
    return false;
  }

  return true;
}

bool CadastroCliente::savingProcedures(const int row) {
  if (not ui->lineEditCliente->text().isEmpty() and not setData(row, "nome_razao", ui->lineEditCliente->text())) {
    qDebug() << "Erro setando nome_razao";
    return false;
  }

  if (not ui->lineEditNomeFantasia->text().isEmpty() and
      not setData(row, "nomeFantasia", ui->lineEditNomeFantasia->text())) {
    qDebug() << "Erro setando nomeFantasia";
    return false;
  }

  if (not ui->lineEditCPF->text().remove(".").remove("-").isEmpty() and
      not setData(row, "cpf", ui->lineEditCPF->text())) {
    qDebug() << "Erro setando cpf";
    return false;
  }

  if (not ui->lineEditContatoNome->text().isEmpty() and
      not setData(row, "contatoNome", ui->lineEditContatoNome->text())) {
    qDebug() << "Erro setando contatoNome";
    return false;
  }

  if (not ui->lineEditContatoCPF->text().remove(".").remove("-").isEmpty() and
      not setData(row, "contatoCPF", ui->lineEditContatoCPF->text())) {
    qDebug() << "Erro setando contatoCPF";
    return false;
  }

  if (not ui->lineEditContatoApelido->text().isEmpty() and
      not setData(row, "contatoApelido", ui->lineEditContatoApelido->text())) {
    qDebug() << "Erro setando contatoApelido";
    return false;
  }

  if (not ui->lineEditContatoRG->text().remove(".").remove("-").isEmpty() and
      not setData(row, "contatoRG", ui->lineEditContatoRG->text())) {
    qDebug() << "Erro setando contatoRG";
    return false;
  }

  if (not ui->lineEditCNPJ->text().remove(".").remove("/").remove("-").isEmpty() and
      not setData(row, "cnpj", ui->lineEditCNPJ->text())) {
    qDebug() << "Erro setando cnpj";
    return false;
  }

  if (not ui->lineEditInscEstadual->text().isEmpty() and
      not setData(row, "inscEstadual", ui->lineEditInscEstadual->text())) {
    qDebug() << "Erro setando inscEstadual";
    return false;
  }

  if (ui->dateEdit->date().toString("dd-MM-yyyy") != "01-01-1900") {
    if (not setData(row, "dataNasc", ui->dateEdit->date().toString("yyyy-MM-dd"))) {
      qDebug() << "Erro setando data nasc.";
      return false;
    }
  }

  if (not ui->lineEditTel_Res->text().isEmpty() and not setData(row, "tel", ui->lineEditTel_Res->text())) {
    qDebug() << "Erro setando tel";
    return false;
  }

  if (not ui->lineEditTel_Cel->text().isEmpty() and not setData(row, "telCel", ui->lineEditTel_Cel->text())) {
    qDebug() << "Erro setando telCel";
    return false;
  }

  if (not ui->lineEditTel_Com->text().isEmpty() and not setData(row, "telCom", ui->lineEditTel_Com->text())) {
    qDebug() << "Erro setando telCom";
    return false;
  }

  if (not ui->lineEditNextel->text().isEmpty() and not setData(row, "nextel", ui->lineEditNextel->text())) {
    qDebug() << "Erro setando nextel";
    return false;
  }

  if (not ui->lineEditEmail->text().isEmpty() and not setData(row, "email", ui->lineEditEmail->text())) {
    qDebug() << "Erro setando email";
    return false;
  }

  if (not setData(row, "idCadastroRel", ui->itemBoxCliente->value())) {
    qDebug() << "Erro setando idCadastroRel";
    return false;
  }

  if (not setData(row, "idProfissionalRel", ui->itemBoxProfissional->value())) {
    qDebug() << "Erro setando idProfissionalRel";
    return false;
  }

  if (not setData(row, "idUsuarioRel", ui->itemBoxVendedor->value())) {
    qDebug() << "Erro setando idUsuarioRel";
    return false;
  }

  if (not setData(row, "pfpj", tipoPFPJ)) {
    qDebug() << "Erro setando pfpj";
    return false;
  }

  return true;
}

void CadastroCliente::clearFields() {
  RegisterDialog::clearFields();

  ui->radioButtonPF->setChecked(true);
  novoEndereco();

  foreach (ItemBox *box, this->findChildren<ItemBox *>()) { box->clear(); }

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
    qDebug() << "erro: " << modelEnd.lastError();
  }

  ui->itemBoxCliente->searchDialog()->setFilter("idCliente NOT IN (" + data(primaryKey).toString() + ")");

  QSqlQuery query;
  query.prepare("SELECT idCliente, nome_razao, nomeFantasia FROM Cliente WHERE idCadastroRel = :primaryKey");
  query.bindValue(":primaryKey", data(primaryKey));

  if (not query.exec()) {
    qDebug() << "Erro na query cliente: " << query.lastError();
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
  if (not validaCPF(QString(text).remove(".").remove("-"))) {
    ui->lineEditCPF->setStyleSheet("color: rgb(255, 0, 0);");
  } else {
    ui->lineEditCPF->setStyleSheet("");
  }
}

void CadastroCliente::on_lineEditCNPJ_textEdited(const QString &text) {
  if (not validaCNPJ(QString(text).remove(".").remove("/").remove("-"))) {
    ui->lineEditCNPJ->setStyleSheet("color: rgb(255, 0, 0);");
  } else {
    ui->lineEditCNPJ->setStyleSheet("");
  }
}

bool CadastroCliente::cadastrarEndereco(const bool isUpdate) {
  if (not RegisterDialog::verifyFields({ui->lineEditCEP, ui->lineEditLogradouro, ui->lineEditNro, ui->lineEditBairro,
                                       ui->lineEditCidade, ui->lineEditUF})) {
    return false;
  }

  if (not ui->lineEditCEP->isValid()) {
    ui->lineEditCEP->setFocus();
    QMessageBox::warning(this, "Atenção!", "CEP inválido!", QMessageBox::Ok, QMessageBox::NoButton);
    return false;
  }

  const int row = (isUpdate) ? mapperEnd.currentIndex() : modelEnd.rowCount();

  if (not isUpdate) {
    modelEnd.insertRow(row);
  }

  if (not setDataEnd(row, "descricao", ui->comboBoxTipoEnd->currentText())) {
    qDebug() << "Erro setData descricao: " << modelEnd.lastError();
    return false;
  }

  if (not ui->lineEditCEP->text().isEmpty() and not setDataEnd(row, "cep", ui->lineEditCEP->text())) {
    qDebug() << "Erro setData cep: " << modelEnd.lastError();
    return false;
  }

  if (not ui->lineEditLogradouro->text().isEmpty() and
      not setDataEnd(row, "logradouro", ui->lineEditLogradouro->text())) {
    qDebug() << "Erro setData logradouro: " << modelEnd.lastError();
    return false;
  }

  if (not ui->lineEditNro->text().isEmpty() and not setDataEnd(row, "numero", ui->lineEditNro->text())) {
    qDebug() << "Erro setData numero: " << modelEnd.lastError();
    return false;
  }

  if (not ui->lineEditComp->text().isEmpty() and not setDataEnd(row, "complemento", ui->lineEditComp->text())) {
    qDebug() << "Erro setData complemento: " << modelEnd.lastError();
    return false;
  }

  if (not ui->lineEditBairro->text().isEmpty() and not setDataEnd(row, "bairro", ui->lineEditBairro->text())) {
    qDebug() << "Erro setData bairro: " << modelEnd.lastError();
    return false;
  }

  if (not ui->lineEditCidade->text().isEmpty() and not setDataEnd(row, "cidade", ui->lineEditCidade->text())) {
    qDebug() << "Erro setData cidade: " << modelEnd.lastError();
    return false;
  }

  if (not ui->lineEditUF->text().isEmpty() and not setDataEnd(row, "uf", ui->lineEditUF->text())) {
    qDebug() << "Erro setData uf: " << modelEnd.lastError();
    return false;
  }

  if (not setDataEnd(row, "codUF", getCodigoUF(ui->lineEditUF->text()))) {
    qDebug() << "Erro setData codUF: " << modelEnd.lastError();
    return false;
  }

  if (not setDataEnd(row, "desativado", false)) {
    qDebug() << "Erro setData desativado: " << modelEnd.lastError();
    return false;
  }

  ui->tableEndereco->resizeColumnsToContents();

  return true;
}

void CadastroCliente::on_pushButtonAdicionarEnd_clicked() {
  if (not cadastrarEndereco(false)) {
    QMessageBox::warning(this, "Atenção!", "Não foi possível cadastrar este endereço.", QMessageBox::Ok,
                         QMessageBox::NoButton);
  }
}

void CadastroCliente::on_pushButtonAtualizarEnd_clicked() {
  if (not cadastrarEndereco(true)) {
    QMessageBox::warning(this, "Atenção!", "Não foi possível atualizar este endereço.", QMessageBox::Ok,
                         QMessageBox::NoButton);
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
    QMessageBox::warning(this, "Aviso!", "CEP não encontrado!", QMessageBox::Ok);
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
  if (not validaCPF(QString(text).remove(".").remove("-"))) {
    ui->lineEditContatoCPF->setStyleSheet("color: rgb(255, 0, 0);");
  } else {
    ui->lineEditContatoCPF->setStyleSheet("");
  }
}

void CadastroCliente::on_checkBoxMostrarInativos_clicked(const bool checked) {
  if (checked) {
    modelEnd.setFilter("idCliente = " + data(primaryKey).toString());
  } else {
    modelEnd.setFilter("idCliente = " + data(primaryKey).toString() + " AND desativado = FALSE");
  }
}

void CadastroCliente::on_pushButtonRemoverEnd_clicked() {
  QMessageBox msgBox(QMessageBox::Warning, "Atenção!", "Tem certeza que deseja remover?",
                     QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Sim");
  msgBox.setButtonText(QMessageBox::No, "Não");

  if (msgBox.exec() == QMessageBox::Yes) {
    if (modelEnd.submitAll()) {
      if (not modelEnd.select()) {
        qDebug() << "erro modelEnd: " << modelEnd.lastError();
        return;
      }

      novoEndereco();
    } else {
      QMessageBox::warning(this, "Atenção!", "Não foi possível remover este item.", QMessageBox::Ok,
                           QMessageBox::NoButton);
      qDebug() << "model error: " << modelEnd.lastError();
    }
  }
}

#ifdef TEST

#include <QTest>

bool CadastroCliente::TestClienteIncompleto() {
  QTest::keyClicks(ui->lineEditCliente, "Cliente Incompleto");
  QTest::keyClicks(ui->lineEditCPF, "877.533.489-57");

  silent = true;

  return save();
}

bool CadastroCliente::TestClienteEndereco() {
  QTest::keyClicks(ui->lineEditCEP, "12245-500");
  //  QTest::keyClicks(ui->lineEditEndereco, "Rua Abolição");
  QTest::keyClicks(ui->lineEditNro, "87");
  QTest::keyClicks(ui->lineEditComp, "Ap. 92 T. 01");
  //  QTest::keyClicks(ui->lineEditBairro, "Vila Betânia");
  //  QTest::keyClicks(ui->lineEditCidade, "São José dos Campos");
  //  QTest::keyClicks(ui->lineEditUF, "SP");
  on_pushButtonAdicionarEnd_clicked();

  QTest::keyClicks(ui->lineEditCliente, "Cliente Endereco");
  QTest::keyClicks(ui->lineEditCPF, "976.524.755-97");

  silent = true;

  return save();
}

bool CadastroCliente::TestClienteCompleto() {
  //  QTest::keyClicks(ui->lineEditCEP, "12245-500");
  //  QTest::keyClicks(ui->lineEditEndereco, "Rua Abolição");
  //  QTest::keyClicks(ui->lineEditNro, "87");
  //  QTest::keyClicks(ui->lineEditComp, "Ap. 92 T. 01");
  //  QTest::keyClicks(ui->lineEditBairro, "Vila Betânia");
  //  QTest::keyClicks(ui->lineEditCidade, "São José dos Campos");
  //  QTest::keyClicks(ui->lineEditUF, "SP");
  //  QTest::mouseClick(ui->pushButtonAdicionarEnd, Qt::LeftButton);

  QTest::keyClicks(ui->lineEditCliente, "Cliente Completo");
  QTest::keyClicks(ui->lineEditCPF, "187.958.502-28");
  // TODO: preencher campos restantes
  silent = true;

  return save();
}

#endif
