#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QTest>

#include "cadastrocliente.h"
#include "ui_cadastrocliente.h"
#include "searchdialog.h"
#include "cepcompleter.h"
#include "cadastroprofissional.h"

CadastroCliente::CadastroCliente(bool closeBeforeUpdate, QWidget *parent)
  : RegisterDialog("Cliente", "idCliente", parent), ui(new Ui::CadastroCliente),
    closeBeforeUpdate(closeBeforeUpdate) {
  ui->setupUi(this);

  ui->lineEditCEP->setInputMask("99999-999;_");
  ui->lineEditUF->setInputMask(">AA;_");

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
  modelEnd.select();

  ui->tableEndereco->setModel(&modelEnd);
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("idEndereco"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("desativado"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("idCliente"));

  setupUi();

  SearchDialog *sdCliente = SearchDialog::cliente(ui->itemBoxCliente);
  ui->itemBoxCliente->setSearchDialog(sdCliente);

  SearchDialog *sdProfissional = SearchDialog::profissional(this);
  ui->itemBoxProfissional->setSearchDialog(sdProfissional);

  RegisterDialog *regProfissional = new CadastroProfissional(this);
  ui->itemBoxProfissional->setRegisterDialog(regProfissional);

  SearchDialog *sdVendedor = SearchDialog::usuario(this);
  ui->itemBoxVendedor->setSearchDialog(sdVendedor);

  setupMapper();
  newRegister();
}

void CadastroCliente::setupUi() {
  ui->lineEditCPF->setInputMask("999.999.999-99;_");
  ui->lineEditContatoCPF->setInputMask("999.999.999-99;_");
  ui->lineEditContatoRG->setInputMask("99.999.999-9;_");
  ui->lineEditIdNextel->setInputMask("99*9999999*99999;_");
  ui->lineEditCNPJ->setInputMask("99.999.999/9999-99;_");

  // Placeholders
  ui->lineEditContatoCPF->setPlaceholderText("999.999.999-99");
  ui->lineEditCPF->setPlaceholderText("999.999.999-99");
  ui->lineEditEmail->setPlaceholderText("usuario@email.com");
  ui->lineEditNextel->setPlaceholderText("(99)99999-9999");
}

bool CadastroCliente::TestClienteIncompleto()
{
  QTest::keyClicks(ui->lineEditCliente, "Cliente Incompleto");
  QTest::keyClicks(ui->lineEditCPF, "877.533.489-57");
  return save(true);
}

bool CadastroCliente::TestClienteEndereco(){
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
  return save(true);
}

bool CadastroCliente::TestClienteCompleto(){
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
  return save(true);
}

CadastroCliente::~CadastroCliente() { delete ui; }

bool CadastroCliente::verifyRequiredField(QLineEdit *line, bool silent) {
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
      QMessageBox::warning(this, "Atenção!",
                           "Você não preencheu um campo obrigatório!", QMessageBox::Ok,
                           QMessageBox::NoButton);
      line->setFocus();
    }
    return false;
  }

  return true;
}

bool CadastroCliente::verifyFields(int row) {
  if (modelEnd.rowCount() == 0) {
    setData(row, "incompleto", true);
    return true;
    qDebug() << "Faltou endereço!";
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
    return true;
  }

  return true;
}

bool CadastroCliente::savingProcedures(int row) {
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

  int idCliente = data(row, primaryKey).toInt();

  if (not data(row, primaryKey).isValid()) {
    QSqlQuery qryLastId("SELECT LAST_INSERT_ID() AS lastId;");
    qryLastId.exec();
    qryLastId.first();
    idCliente = qryLastId.value("lastId").toInt();
  }

  for (int end = 0; end < modelEnd.rowCount(); ++end) {
    modelEnd.setData(modelEnd.index(end, modelEnd.fieldIndex(primaryKey)), idCliente);
  }

//  qDebug() << "Endereco filter: " << modelEnd.filter();
//  qDebug() << "Query: " << modelEnd.query().lastQuery();
  if (not modelEnd.submitAll()) {
    qDebug() << objectName() << " : " << __LINE__
             << " : Error on modelEnd.submitAll() : " << modelEnd.lastError();
    qDebug() << "QUERY : " << modelEnd.query().lastQuery();
    return false;
  }

  return true;
}

void CadastroCliente::clearFields() {
  RegisterDialog::clearFields();

  ui->radioButtonPF->setChecked(true);
  novoEnd();

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

  mapperEnd.addMapping(ui->comboBoxTipoEnd, modelEnd.fieldIndex("descricao"));
  mapperEnd.addMapping(ui->lineEditCEP, modelEnd.fieldIndex("CEP"));
  mapperEnd.addMapping(ui->lineEditEndereco, modelEnd.fieldIndex("logradouro"));
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

QString CadastroCliente::getTipoClienteFornecedor() const { return tipoClienteFornecedor; }

void CadastroCliente::setTipoClienteFornecedor(const QString &value) {
  bool isForn = (value == "FORNECEDOR");
  ui->groupBoxMaisInfo->setHidden(isForn);
  ui->groupBoxPFPJ->setHidden(isForn);

  if (isForn) {
    ui->radioButtonPJ->setChecked(true);
  }

  tipoClienteFornecedor = value;
  adjustSize();
}

QString CadastroCliente::getTipo() const { return tipoPFPJ; }

void CadastroCliente::setTipo(const QString &value) { tipoPFPJ = value; }

bool CadastroCliente::viewRegister(QModelIndex idx) {
  if (not confirmationMessage()) {
    return false;
  }

  clearFields();
  updateMode();
  model.select();
  mapper.setCurrentModelIndex(idx);

  ui->itemBoxCliente->searchDialog()->setFilter("idCliente NOT IN (" + data(primaryKey).toString() + ")");
  modelEnd.setFilter("idCliente = " + data(primaryKey).toString() + " AND desativado = 0");

  if (not modelEnd.select()) {
    qDebug() << modelEnd.lastError();
  }

  int idCliente = data("idCliente").toInt();
  QSqlQuery query;

  if(not query.exec("SELECT idCliente, nome_razao, nomeFantasia, pfpj FROM Cliente WHERE idCadastroRel = '" +
                    QString::number(idCliente) + "';")){
    qDebug() << "Erro na query cliente: " << query.lastError();
    return false;
  }

  while (query.next()) {
    QString line =
        query.value("idCliente").toString() + " - " + query.value("nome_razao").toString() + " - " + query.value("nomeFantasia").toString();
    ui->textEditObservacoes->insertPlainText(line);
  }

  tipoPFPJ = data("pfpj").toString();

  if (tipoPFPJ == "PJ") {
    ui->radioButtonPJ->setChecked(true);
  }

  if (tipoPFPJ == "PF") {
    ui->radioButtonPF->setChecked(true);
  }

  show();
  return true;
}

void CadastroCliente::on_pushButtonCadastrar_clicked() {
  if (save()) {
    if (closeBeforeUpdate)
      accept();
  }
}

void CadastroCliente::on_pushButtonAtualizar_clicked() {
  if (save()) {
    if (closeBeforeUpdate)
      accept();
  }
}

void CadastroCliente::enableEditor() {
  ui->frame->setEnabled(true);
  ui->frame_2->setEnabled(true);
}

void CadastroCliente::disableEditor() {
  ui->frame->setEnabled(false);
  ui->frame_2->setEnabled(false);
}

void CadastroCliente::show() {
  adjustSize();
  QWidget::show();
}

void CadastroCliente::on_pushButtonCancelar_clicked() { close(); }

void CadastroCliente::on_pushButtonRemover_clicked() { remove(); }

void CadastroCliente::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroCliente::on_groupBoxPJuridica_toggled(bool arg1) { ui->widgetPJ->setEnabled(arg1); }

void CadastroCliente::on_pushButtonBuscar_clicked() {
  SearchDialog *sdCliente = SearchDialog::cliente(this);
  sdCliente->show();
  connect(sdCliente, &SearchDialog::itemSelected, this, &CadastroCliente::changeItem);
}

void CadastroCliente::changeItem(QVariant value, QString text) {
  Q_UNUSED(text);

  viewRegisterById(value);
}

void CadastroCliente::on_lineEditCPF_textEdited(const QString &) {
  QString text = ui->lineEditCPF->text().remove(".").remove("-");
  validaCPF(text);
}

void CadastroCliente::on_lineEditCNPJ_textEdited(const QString &) {
  QString text = ui->lineEditCNPJ->text().remove(".").remove("/").remove("-");
  validaCNPJ(text);
}

void CadastroCliente::validaCNPJ(QString text) {
  if (text.size() == 14) {

    int digito1;
    int digito2;

    QString sub = text.left(12);

    QVector<int> sub2;

    for (int i = 0; i < sub.size(); ++i) {
      sub2.push_back(sub.at(i).digitValue());
    }

    QVector<int> multiplicadores = {5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2};

    int soma = 0;

    for (int i = 0; i < 12; ++i) {
      soma += sub2.at(i) * multiplicadores.at(i);
    }

    int resto = soma % 11;

    if (resto < 2) {
      digito1 = 0;
    } else {
      digito1 = 11 - resto;
    }

    sub2.push_back(digito1);

    QVector<int> multiplicadores2 = {6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2};
    soma = 0;

    for (int i = 0; i < 13; ++i) {
      soma += sub2.at(i) * multiplicadores2.at(i);
    }

    resto = soma % 11;

    if (resto < 2) {
      digito2 = 0;
    } else {
      digito2 = 11 - resto;
    }

    if (digito1 != text.at(12).digitValue() or digito2 != text.at(13).digitValue()) {
      QMessageBox::warning(this, "Aviso!", "CNPJ inválido!");
      return;
    }
  }
}

void CadastroCliente::validaCPF(QString text) {
  if (text.size() == 11) {
    if (text == "00000000000" or text == "11111111111" or text == "22222222222" or text == "33333333333" or
        text == "44444444444" or text == "55555555555" or text == "66666666666" or text == "77777777777" or
        text == "88888888888" or text == "99999999999") {
      QMessageBox::warning(this, "Aviso!", "CPF inválido!");
      return;
    }

    int digito1;
    int digito2;

    QString sub = text.left(9);

    QVector<int> sub2;

    for (int i = 0; i < sub.size(); ++i) {
      sub2.push_back(sub.at(i).digitValue());
    }

    QVector<int> multiplicadores = {10, 9, 8, 7, 6, 5, 4, 3, 2};

    int soma = 0;

    for (int i = 0; i < 9; ++i) {
      soma += sub2.at(i) * multiplicadores.at(i);
    }

    int resto = soma % 11;

    if (resto < 2) {
      digito1 = 0;
    } else {
      digito1 = 11 - resto;
    }

    sub2.push_back(digito1);

    QVector<int> multiplicadores2 = {11, 10, 9, 8, 7, 6, 5, 4, 3, 2};
    soma = 0;

    for (int i = 0; i < 10; ++i) {
      soma += sub2.at(i) * multiplicadores2.at(i);
    }

    resto = soma % 11;

    if (resto < 2) {
      digito2 = 0;
    } else {
      digito2 = 11 - resto;
    }

    if (digito1 != text.at(9).digitValue() or digito2 != text.at(10).digitValue()) {
      QMessageBox::warning(this, "Aviso!", "CPF inválido!");
      return;
    }
  }
}

bool CadastroCliente::atualizarEnd() {
  if (not RegisterDialog::verifyFields({ui->lineEditCEP, ui->lineEditEndereco, ui->lineEditNro,
                                       ui->lineEditBairro, ui->lineEditCidade, ui->lineEditUF})){
    return false;
  }

  if (not ui->lineEditCEP->isValid()) {
    ui->lineEditCEP->setFocus();
    QMessageBox::warning(this, "Atenção!", "CEP inválido!", QMessageBox::Ok, QMessageBox::NoButton);
    return false;
  }

  int row = mapperEnd.currentIndex();

  if (row == -1) {
    row = modelEnd.rowCount();

    if (not modelEnd.insertRow(row)) {
      return false;
    }
  }

  if(not modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("descricao")), ui->comboBoxTipoEnd->currentText())){
    qDebug() << "Erro setData descricao: " << modelEnd.lastError();
    return false;
  }

  if (not ui->lineEditCEP->text().isEmpty()){
    if(not modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("CEP")), ui->lineEditCEP->text())){
      qDebug() << "Erro setData cep: " << modelEnd.lastError();
      return false;
    }
  }

  if (not ui->lineEditEndereco->text().isEmpty()){
    if(not modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("logradouro")), ui->lineEditEndereco->text())){
      qDebug() << "Erro setData logradouro: " << modelEnd.lastError();
      return false;
    }
  }

  if (not ui->lineEditNro->text().isEmpty()){
    if(not modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("numero")), ui->lineEditNro->text())){
      qDebug() << "Erro setData numero: " << modelEnd.lastError();
      return false;
    }
  }

  if (not ui->lineEditComp->text().isEmpty()){
    if(not modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("complemento")), ui->lineEditComp->text())){
      qDebug() << "Erro setData complemento: " << modelEnd.lastError();
      return false;
    }
  }

  if (not ui->lineEditBairro->text().isEmpty()){
    if(not modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("bairro")), ui->lineEditBairro->text())){
      qDebug() << "Erro setData bairro: " << modelEnd.lastError();
      return false;
    }
  }

  if (not ui->lineEditCidade->text().isEmpty()){
    if(not modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("cidade")), ui->lineEditCidade->text())){
      qDebug() << "Erro setData cidade: " << modelEnd.lastError();
      return false;
    }
  }

  if (not ui->lineEditUF->text().isEmpty()){
    if(not modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("uf")), ui->lineEditUF->text())){
      qDebug() << "Erro setData uf: " << modelEnd.lastError();
      return false;
    }
  }

  if(not modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("desativado")), 0)){
    qDebug() << "Erro setData desativado: " << modelEnd.lastError();
    return false;
  }

  return true;
}

void CadastroCliente::on_pushButtonAdicionarEnd_clicked() {
  if(not atualizarEnd()){
    QMessageBox::warning(this, "Atenção!", "Não foi possível cadastrar este endereço.", QMessageBox::Ok,
                         QMessageBox::NoButton);
  }
}

void CadastroCliente::on_pushButtonAtualizarEnd_clicked() {
  if(not atualizarEnd()){
    QMessageBox::warning(this, "Atenção!", "Não foi possível cadastrar este endereço.", QMessageBox::Ok,
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
    ui->lineEditEndereco->setText(cc.getEndereco());
    ui->lineEditBairro->setText(cc.getBairro());
  } else {
    QMessageBox::warning(this, "Aviso!", "CEP não encontrado!", QMessageBox::Ok);
  }
}

void CadastroCliente::clearEnd() {
  ui->lineEditBairro->clear();
  ui->lineEditCEP->clear();
  ui->lineEditCidade->clear();
  ui->lineEditComp->clear();
  ui->lineEditEndereco->clear();
  ui->lineEditNro->clear();
  ui->lineEditUF->clear();
}

void CadastroCliente::novoEnd() {
  ui->pushButtonAtualizarEnd->hide();
  ui->pushButtonAdicionarEnd->show();
  ui->tableEndereco->clearSelection();
  mapper.setCurrentIndex(-1);
  clearEnd();
}

void CadastroCliente::on_pushButtonNovoEnd_clicked() { novoEnd(); }

void CadastroCliente::on_tableEndereco_clicked(const QModelIndex &index) {
  if (modelEnd.isDirty()) {
    QMessageBox msgBox(QMessageBox::Warning, "Atenção!", "Deseja aplicar as alterações?",
                       QMessageBox::Yes | QMessageBox::No);
    msgBox.setButtonText(QMessageBox::Yes, "Sim");
    msgBox.setButtonText(QMessageBox::No, "Não");

    if (msgBox.exec() == QMessageBox::Yes) {
      if (not atualizarEnd()) {
        return;
      }
    }
  }

  ui->pushButtonAtualizarEnd->show();
  ui->pushButtonAdicionarEnd->hide();
  mapperEnd.setCurrentModelIndex(index);
}

void CadastroCliente::on_radioButtonPF_toggled(bool checked) {
  if (checked) {
    tipoPFPJ = "PF";
    ui->lineEditCNPJ->hide();
    ui->labelCNPJ->hide();
    ui->lineEditCPF->show();
    ui->labelCPF->show();
    ui->lineEditInscEstadual->hide();
    ui->labelInscricaoEstadual->hide();
  } else {
    tipoPFPJ = "PJ";
    ui->lineEditCNPJ->show();
    ui->labelCNPJ->show();
    ui->lineEditCPF->hide();
    ui->labelCPF->hide();
    ui->lineEditInscEstadual->show();
    ui->labelInscricaoEstadual->show();
  }

  adjustSize();
}

void CadastroCliente::on_lineEditContatoCPF_textEdited(const QString &) {
  QString text = ui->lineEditContatoCPF->text().remove(".").remove("-");
  validaCPF(text);
}

void CadastroCliente::on_checkBoxMostrarInativos_clicked(bool checked)
{
  if(checked){
    modelEnd.setFilter("idCliente = " + data(primaryKey).toString());
  } else{
    modelEnd.setFilter("idCliente = " + data(primaryKey).toString() + " AND desativado = 0");
  }
}

void CadastroCliente::on_pushButtonRemoverEnd_clicked()
{
  QMessageBox msgBox(QMessageBox::Warning, "Atenção!", "Tem certeza que deseja remover?",
                     QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Sim");
  msgBox.setButtonText(QMessageBox::No, "Não");
  if (msgBox.exec() == QMessageBox::Yes) {
    qDebug() << "set desativado: " << modelEnd.setData(modelEnd.index(mapperEnd.currentIndex(), modelEnd.fieldIndex("desativado")), 1);
    if (modelEnd.submitAll()) {
      modelEnd.select();
      novoEnd();
    } else {
      QMessageBox::warning(this, "Atenção!", "Não foi possível remover este item.", QMessageBox::Ok,
                           QMessageBox::NoButton);
      qDebug() << "model error: " << modelEnd.lastError();
    }
  }
}
