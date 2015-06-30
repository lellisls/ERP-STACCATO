#include <QSqlDriver>
#include <QSqlRecord>

#include "cadastrocliente.h"
#include "ui_cadastrocliente.h"
#include "searchdialog.h"
#include "cepcompleter.h"
#include "cadastroprofissional.h"
#include "usersession.h"

CadastroCliente::CadastroCliente(QWidget *parent)
  : RegisterDialog("Cliente", "idCliente", parent), ui(new Ui::CadastroCliente) {
  ui->setupUi(this);

  SearchDialog *sdCliente = SearchDialog::cliente(ui->itemBoxCliente);
  ui->itemBoxCliente->setSearchDialog(sdCliente);

  SearchDialog *sdProfissional = SearchDialog::profissional(this);
  ui->itemBoxProfissional->setSearchDialog(sdProfissional);

  RegisterDialog *regProfissional = new CadastroProfissional(this);
  ui->itemBoxProfissional->setRegisterDialog(regProfissional);

  SearchDialog *sdVendedor = SearchDialog::usuario(this);
  ui->itemBoxVendedor->setSearchDialog(sdVendedor);

  setupEndereco();
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

void CadastroCliente::setupEndereco() {
  modelEnd.setTable("Cliente_has_Endereco");
  modelEnd.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelEnd.setHeaderData(modelEnd.fieldIndex("descricao"), Qt::Horizontal, "Descrição");
  modelEnd.setHeaderData(modelEnd.fieldIndex("cep"), Qt::Horizontal, "CEP");
  modelEnd.setHeaderData(modelEnd.fieldIndex("logradouro"), Qt::Horizontal, "Logradouro");
  modelEnd.setHeaderData(modelEnd.fieldIndex("numero"), Qt::Horizontal, "Número");
  modelEnd.setHeaderData(modelEnd.fieldIndex("complemento"), Qt::Horizontal, "Compl.");
  modelEnd.setHeaderData(modelEnd.fieldIndex("bairro"), Qt::Horizontal, "Bairro");
  modelEnd.setHeaderData(modelEnd.fieldIndex("cidade"), Qt::Horizontal, "Cidade");
  modelEnd.setHeaderData(modelEnd.fieldIndex("uf"), Qt::Horizontal, "UF");
  modelEnd.setFilter("idEndereco = 1");

  if (not modelEnd.select()) {
    qDebug() << "erro modelEnd: " << modelEnd.lastError();
    return;
  }

  ui->tableEndereco->setModel(&modelEnd);
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("idEndereco"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("desativado"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("idCliente"));
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
  if (not ui->lineEditCliente->text().isEmpty()) {
    setData(row, "nome_razao", ui->lineEditCliente->text());
  }

  if (not ui->lineEditNomeFantasia->text().isEmpty()) {
    setData(row, "nomeFantasia", ui->lineEditNomeFantasia->text());
  }

  if (not ui->lineEditCPF->text().remove(".").remove("-").isEmpty()) {
    setData(row, "cpf", ui->lineEditCPF->text());
  }

  if (not ui->lineEditContatoNome->text().isEmpty()) {
    setData(row, "contatoNome", ui->lineEditContatoNome->text());
  }

  if (not ui->lineEditContatoCPF->text().remove(".").remove("-").isEmpty()) {
    setData(row, "contatoCPF", ui->lineEditContatoCPF->text());
  }

  if (not ui->lineEditContatoApelido->text().isEmpty()) {
    setData(row, "contatoApelido", ui->lineEditContatoApelido->text());
  }

  if (not ui->lineEditContatoRG->text().remove(".").remove("-").isEmpty()) {
    setData(row, "contatoRG", ui->lineEditContatoRG->text());
  }

  if (not ui->lineEditCNPJ->text().remove(".").remove("/").remove("-").isEmpty()) {
    setData(row, "cnpj", ui->lineEditCNPJ->text());
  }

  if (not ui->lineEditInscEstadual->text().isEmpty()) {
    setData(row, "inscEstadual", ui->lineEditInscEstadual->text());
  }

  if (not ui->lineEditTel_Res->text().isEmpty()) {
    setData(row, "tel", ui->lineEditTel_Res->text());
  }

  if (not ui->lineEditTel_Cel->text().isEmpty()) {
    setData(row, "telCel", ui->lineEditTel_Cel->text());
  }

  if (not ui->lineEditTel_Com->text().isEmpty()) {
    setData(row, "telCom", ui->lineEditTel_Com->text());
  }

  if (not ui->lineEditNextel->text().isEmpty()) {
    setData(row, "nextel", ui->lineEditNextel->text());
  }

  if (not ui->lineEditEmail->text().isEmpty()) {
    setData(row, "email", ui->lineEditEmail->text());
  }

  setData(row, "idCadastroRel", ui->itemBoxCliente->value());
  setData(row, "idProfissionalRel", ui->itemBoxProfissional->value());
  setData(row, "idUsuarioRel", ui->itemBoxVendedor->value());
  setData(row, "pfpj", tipoPFPJ);

  if (not model.submitAll()) {
    qDebug() << objectName() << " : " << __LINE__ << " : Error on model.submitAll() : " << model.lastError();
    return false;
  }

  const int idCliente =
      (data(row, primaryKey).isValid()) ? data(row, primaryKey).toInt() : model.query().lastInsertId().toInt();

  for (int row = 0, rowCount = modelEnd.rowCount(); row < rowCount; ++row) {
    if (not modelEnd.setData(model.index(row, modelEnd.fieldIndex(primaryKey)), idCliente)) {
      qDebug() << "error: " << modelEnd.lastError();
    }
  }

  if (not modelEnd.submitAll()) {
    qDebug() << objectName() << " : " << __LINE__ << " : Error on modelEnd.submitAll() : " << modelEnd.lastError();
    qDebug() << "Last query: "
             << modelEnd.database().driver()->sqlStatement(QSqlDriver::InsertStatement, modelEnd.tableName(),
                                                           modelEnd.record(row), false);
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

  mapperEnd.setModel(&modelEnd);
  mapperEnd.setSubmitPolicy(QDataWidgetMapper::ManualSubmit);

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

  ui->itemBoxCliente->searchDialog()->setFilter("idCliente NOT IN (" + data(primaryKey).toString() + ")");
  modelEnd.setFilter("idCliente = " + data(primaryKey).toString() + " AND desativado = FALSE");

  if (not modelEnd.select()) {
    qDebug() << modelEnd.lastError();
    return false;
  }

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
  connect(sdCliente, &SearchDialog::itemSelected, this, &CadastroCliente::changeItem);
  sdCliente->show();
}

void CadastroCliente::changeItem(QVariant value) { viewRegisterById(value); }

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

  if (not modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("descricao")), ui->comboBoxTipoEnd->currentText())) {
    qDebug() << "Erro setData descricao: " << modelEnd.lastError();
    return false;
  }

  if (not ui->lineEditCEP->text().isEmpty()) {
    if (not modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("CEP")), ui->lineEditCEP->text())) {
      qDebug() << "Erro setData cep: " << modelEnd.lastError();
      return false;
    }
  }

  if (not ui->lineEditLogradouro->text().isEmpty()) {
    if (not modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("logradouro")), ui->lineEditLogradouro->text())) {
      qDebug() << "Erro setData logradouro: " << modelEnd.lastError();
      return false;
    }
  }

  if (not ui->lineEditNro->text().isEmpty()) {
    if (not modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("numero")), ui->lineEditNro->text())) {
      qDebug() << "Erro setData numero: " << modelEnd.lastError();
      return false;
    }
  }

  if (not ui->lineEditComp->text().isEmpty()) {
    if (not modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("complemento")), ui->lineEditComp->text())) {
      qDebug() << "Erro setData complemento: " << modelEnd.lastError();
      return false;
    }
  }

  if (not ui->lineEditBairro->text().isEmpty()) {
    if (not modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("bairro")), ui->lineEditBairro->text())) {
      qDebug() << "Erro setData bairro: " << modelEnd.lastError();
      return false;
    }
  }

  if (not ui->lineEditCidade->text().isEmpty()) {
    if (not modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("cidade")), ui->lineEditCidade->text())) {
      qDebug() << "Erro setData cidade: " << modelEnd.lastError();
      return false;
    }
  }

  if (not ui->lineEditUF->text().isEmpty()) {
    if (not modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("uf")), ui->lineEditUF->text())) {
      qDebug() << "Erro setData uf: " << modelEnd.lastError();
      return false;
    }

    if (not modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("codUF")), getCodigoUF())) {
      qDebug() << "Erro setData uf: " << modelEnd.lastError();
      return false;
    }
  }

  if (not modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("desativado")), 0)) {
    qDebug() << "Erro setData desativado: " << modelEnd.lastError();
    return false;
  }

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

int CadastroCliente::getCodigoUF() {
  const QString uf = ui->lineEditUF->text().toLower();

  if (uf == "ro") return 11;
  if (uf == "ac") return 12;
  if (uf == "am") return 13;
  if (uf == "rr") return 14;
  if (uf == "pa") return 15;
  if (uf == "ap") return 16;
  if (uf == "to") return 17;
  if (uf == "ma") return 21;
  if (uf == "pi") return 22;
  if (uf == "ce") return 23;
  if (uf == "rn") return 24;
  if (uf == "pb") return 25;
  if (uf == "pe") return 26;
  if (uf == "al") return 27;
  if (uf == "se") return 28;
  if (uf == "ba") return 29;
  if (uf == "mg") return 31;
  if (uf == "es") return 32;
  if (uf == "rj") return 33;
  if (uf == "sp") return 35;
  if (uf == "pr") return 41;
  if (uf == "sc") return 42;
  if (uf == "rs") return 43;
  if (uf == "ms") return 50;
  if (uf == "mt") return 51;
  if (uf == "go") return 52;
  if (uf == "df") return 53;

  return 0;
}
