#include <QDebug>
#include <QDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "cadastrotransportadora.h"
#include "ui_cadastrotransportadora.h"
#include "searchdialog.h"
#include "cepcompleter.h"
#include "usersession.h"

CadastroTransportadora::CadastroTransportadora(QWidget *parent)
  : RegisterDialog("Transportadora", "idTransportadora", parent), ui(new Ui::CadastroTransportadora) {
  ui->setupUi(this);

  modelEnd.setTable("Transportadora_has_Endereco");
  modelEnd.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelEnd.setHeaderData(modelEnd.fieldIndex("descricao"), Qt::Horizontal, "Descrição");
  modelEnd.setHeaderData(modelEnd.fieldIndex("cep"), Qt::Horizontal, "CEP");
  modelEnd.setHeaderData(modelEnd.fieldIndex("logradouro"), Qt::Horizontal, "Logradouro");
  modelEnd.setHeaderData(modelEnd.fieldIndex("numero"), Qt::Horizontal, "Número");
  modelEnd.setHeaderData(modelEnd.fieldIndex("complemento"), Qt::Horizontal, "Compl.");
  modelEnd.setHeaderData(modelEnd.fieldIndex("bairro"), Qt::Horizontal, "Bairro");
  modelEnd.setHeaderData(modelEnd.fieldIndex("cidade"), Qt::Horizontal, "Cidade");
  modelEnd.setHeaderData(modelEnd.fieldIndex("uf"), Qt::Horizontal, "UF");
  modelEnd.setFilter("idEndereco = 0");

  if (not modelEnd.select()) {
    qDebug() << "erro modelEnd: " << modelEnd.lastError();
    return;
  }

  ui->tableEndereco->setModel(&modelEnd);
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("idEndereco"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("desativado"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("idTransportadora"));

  setupUi();
  setupMapper();
  newRegister();

  foreach (QLineEdit *line, findChildren<QLineEdit *>()) {
    connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);
  }

  if (UserSession::getTipoUsuario() != "ADMINISTRADOR") {
    ui->pushButtonRemover->setDisabled(true);
    ui->pushButtonRemoverEnd->setDisabled(true);
  }
}

CadastroTransportadora::~CadastroTransportadora() { delete ui; }

void CadastroTransportadora::clearFields() {
  RegisterDialog::clearFields();
  novoEndereco();
  setupUi();
}

bool CadastroTransportadora::verifyFields(int row) {
  Q_UNUSED(row);

  if (not RegisterDialog::verifyFields({ui->lineEditANTT, ui->lineEditCNPJ, ui->lineEditInscEstadual,
                                       ui->lineEditNomeFantasia, ui->lineEditPlaca, ui->lineEditRazaoSocial,
                                       ui->lineEditTel})) {
    return false;
  }

  return true;
}

bool CadastroTransportadora::savingProcedures(int row) {
  setData(row, "cnpj", ui->lineEditCNPJ->text());
  setData(row, "razaoSocial", ui->lineEditRazaoSocial->text());
  setData(row, "nomeFantasia", ui->lineEditNomeFantasia->text());
  setData(row, "inscEstadual", ui->lineEditInscEstadual->text());
  setData(row, "tel", ui->lineEditTel->text());
  setData(row, "antt", ui->lineEditANTT->text());
  setData(row, "placaVeiculo", ui->lineEditPlaca->text());

  if (not model.submitAll()) {
    qDebug() << objectName() << " : " << __LINE__ << " : Error on model.submitAll() : " << model.lastError();
    return false;
  }

  int idTransportadora = data(row, primaryKey).toInt();

  if (not data(row, primaryKey).isValid()) {
    idTransportadora = model.query().lastInsertId().toInt();
  }

  for (int row = 0; row < modelEnd.rowCount(); ++row) {
    modelEnd.setData(model.index(row, modelEnd.fieldIndex(primaryKey)), idTransportadora);
  }

  if (not modelEnd.submitAll()) {
    qDebug() << objectName() << " : " << __LINE__ << " : Error on modelEnd.submitAll() : " << modelEnd.lastError();
    qDebug() << "QUERY : " << modelEnd.query().lastQuery();
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

  mapperEnd.setModel(&modelEnd);

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

bool CadastroTransportadora::viewRegister(QModelIndex index) {
  if (not RegisterDialog::viewRegister(index)) {
    return false;
  }

  modelEnd.setFilter("idTransportadora = " + data(primaryKey).toString() + " AND desativado = FALSE");

  if (not modelEnd.select()) {
    qDebug() << "Erro no model endereco: " << modelEnd.lastError();
    return false;
  }

  return true;
}

void CadastroTransportadora::on_pushButtonCadastrar_clicked() { save(); }

void CadastroTransportadora::on_pushButtonAtualizar_clicked() { update(); }

void CadastroTransportadora::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroTransportadora::on_pushButtonRemover_clicked() { remove(); }

void CadastroTransportadora::on_pushButtonCancelar_clicked() { close(); }

void CadastroTransportadora::on_pushButtonBuscar_clicked() {
  SearchDialog *sdTransportadora = SearchDialog::transportadora(this);
  connect(sdTransportadora, &SearchDialog::itemSelected, this, &CadastroTransportadora::changeItem);
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
  if (not validaCNPJ(QString(text).remove(".").remove("/").remove("-"))) {
    ui->lineEditCNPJ->setStyleSheet("color: rgb(255, 0, 0);");
  } else {
    ui->lineEditCNPJ->setStyleSheet("");
  }
}

void CadastroTransportadora::on_pushButtonAdicionarEnd_clicked() {
  if (not cadastrarEndereco(false)) {
    QMessageBox::warning(this, "Atenção!", "Não foi possível cadastrar este endereço.", QMessageBox::Ok,
                         QMessageBox::NoButton);
  }
}

void CadastroTransportadora::on_pushButtonAtualizarEnd_clicked() {
  if (not cadastrarEndereco(true)) {
    QMessageBox::warning(this, "Atenção!", "Não foi possível atualizar este endereço.", QMessageBox::Ok,
                         QMessageBox::NoButton);
  }
}

void CadastroTransportadora::on_pushButtonEndLimpar_clicked() { novoEndereco(); }

void CadastroTransportadora::on_pushButtonRemoverEnd_clicked() {
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

void CadastroTransportadora::on_checkBoxMostrarInativos_clicked(bool checked) {
  if (checked) {
    modelEnd.setFilter("idTransportadora = " + data(primaryKey).toString());
  } else {
    modelEnd.setFilter("idTransportadora = " + data(primaryKey).toString() + " AND desativado = false");
  }
}

bool CadastroTransportadora::cadastrarEndereco(bool isUpdate) {
  if (not RegisterDialog::verifyFields({ui->lineEditCEP, ui->lineEditLogradouro, ui->lineEditNro, ui->lineEditBairro,
                                       ui->lineEditCidade, ui->lineEditUF})) {
    return false;
  }

  if (not ui->lineEditCEP->isValid()) {
    ui->lineEditCEP->setFocus();
    QMessageBox::warning(this, "Atenção!", "CEP inválido!", QMessageBox::Ok, QMessageBox::NoButton);
    return false;
  }

  int row;

  if (isUpdate) {
    row = mapperEnd.currentIndex();
  } else {
    row = modelEnd.rowCount();
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
  }

  if (not modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("desativado")), 0)) {
    qDebug() << "Erro setData desativado: " << modelEnd.lastError();
    return false;
  }

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
