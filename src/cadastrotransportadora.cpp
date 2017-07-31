#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

#include "cadastrotransportadora.h"
#include "cepcompleter.h"
#include "searchdialog.h"
#include "ui_cadastrotransportadora.h"
#include "usersession.h"

CadastroTransportadora::CadastroTransportadora(QWidget *parent) : RegisterAddressDialog("transportadora", "idTransportadora", parent), ui(new Ui::CadastroTransportadora) {
  ui->setupUi(this);

  for (auto const *line : findChildren<QLineEdit *>()) connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);

  setupUi();
  setupTables();
  setupMapper();
  newRegister();

  sdTransportadora = SearchDialog::transportadora(this);
  connect(sdTransportadora, &SearchDialog::itemSelected, this, &CadastroTransportadora::viewRegisterById);

  if (UserSession::tipoUsuario() != "ADMINISTRADOR") {
    ui->pushButtonRemover->setDisabled(true);
    ui->pushButtonRemoverEnd->setDisabled(true);
  }
}

CadastroTransportadora::~CadastroTransportadora() { delete ui; }

void CadastroTransportadora::setupTables() {
  modelVeiculo.setTable("transportadora_has_veiculo");
  modelVeiculo.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelVeiculo.setHeaderData("modelo", "Modelo");
  modelVeiculo.setHeaderData("capacidade", "Carga Kg");
  modelVeiculo.setHeaderData("placa", "Placa");
  modelVeiculo.setHeaderData("ufPlaca", "UF Placa");

  modelVeiculo.setFilter("idVeiculo = 0");

  if (not modelVeiculo.select()) {
    QMessageBox::critical(this, "Erro!", "Ocorreu um erro ao acessar a tabela de veículo: " + modelVeiculo.lastError().text());
  }

  ui->tableVeiculo->setModel(&modelVeiculo);
  ui->tableVeiculo->hideColumn("idVeiculo");
  ui->tableVeiculo->hideColumn("idTransportadora");
  ui->tableVeiculo->hideColumn("desativado");

  ui->tableEndereco->setModel(&modelEnd);
  ui->tableEndereco->hideColumn("idEndereco");
  ui->tableEndereco->hideColumn("desativado");
  ui->tableEndereco->hideColumn("idTransportadora");
  ui->tableEndereco->hideColumn("codUF");
}

void CadastroTransportadora::clearFields() {
  RegisterDialog::clearFields();
  novoEndereco();
  novoVeiculo();
  setupUi();
}

bool CadastroTransportadora::verifyFields() {
  for (auto const &line : ui->groupBox_7->findChildren<QLineEdit *>()) {
    if (not verifyRequiredField(line)) return false;
  }

  return true;
}

bool CadastroTransportadora::savingProcedures() {
  if (not setData("cnpj", ui->lineEditCNPJ->text())) return false;
  if (not setData("razaoSocial", ui->lineEditRazaoSocial->text())) return false;
  if (not setData("nomeFantasia", ui->lineEditNomeFantasia->text())) return false;
  if (not setData("inscEstadual", ui->lineEditInscEstadual->text())) return false;
  if (not setData("tel", ui->lineEditTel->text())) return false;
  if (not setData("antt", ui->lineEditANTT->text())) return false;

  return true;
}

void CadastroTransportadora::setupMapper() {
  addMapping(ui->lineEditANTT, "antt");
  addMapping(ui->lineEditCNPJ, "cnpj");
  addMapping(ui->lineEditInscEstadual, "inscEstadual");
  addMapping(ui->lineEditNomeFantasia, "nomeFantasia");
  addMapping(ui->lineEditRazaoSocial, "razaoSocial");
  addMapping(ui->lineEditTel, "tel");

  mapperVeiculo.setModel(&modelVeiculo);
  mapperVeiculo.setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
  mapperVeiculo.addMapping(ui->lineEditModelo, modelVeiculo.fieldIndex("modelo"));
  mapperVeiculo.addMapping(ui->lineEditCarga, modelVeiculo.fieldIndex("capacidade"));
  mapperVeiculo.addMapping(ui->lineEditPlaca, modelVeiculo.fieldIndex("placa"));

  mapperEnd.addMapping(ui->comboBoxTipoEnd, modelEnd.fieldIndex("descricao"));
  mapperEnd.addMapping(ui->lineEditBairro, modelEnd.fieldIndex("bairro"));
  mapperEnd.addMapping(ui->lineEditCEP, modelEnd.fieldIndex("CEP"));
  mapperEnd.addMapping(ui->lineEditCidade, modelEnd.fieldIndex("cidade"));
  mapperEnd.addMapping(ui->lineEditComp, modelEnd.fieldIndex("complemento"));
  mapperEnd.addMapping(ui->lineEditLogradouro, modelEnd.fieldIndex("logradouro"));
  mapperEnd.addMapping(ui->lineEditNro, modelEnd.fieldIndex("numero"));
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

void CadastroTransportadora::on_pushButtonCadastrar_clicked() { save(); }

void CadastroTransportadora::on_pushButtonAtualizar_clicked() { save(); }

void CadastroTransportadora::on_pushButtonNovoCad_clicked() {
  newRegister();
  modelVeiculo.setFilter("0");

  if (not modelVeiculo.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela veiculo: " + modelVeiculo.lastError().text());
    return;
  }
}

void CadastroTransportadora::on_pushButtonRemover_clicked() { remove(); }

void CadastroTransportadora::on_pushButtonBuscar_clicked() { sdTransportadora->show(); }

void CadastroTransportadora::on_lineEditCNPJ_textEdited(const QString &text) {
  ui->lineEditCNPJ->setStyleSheet(validaCNPJ(QString(text).remove(".").remove("/").remove("-")) ? "background-color: rgb(255, 255, 127)"
                                                                                                : "background-color: rgb(255, 255, 127);color: rgb(255, 0, 0)");
}

void CadastroTransportadora::on_pushButtonAdicionarEnd_clicked() {
  if (not cadastrarEndereco()) {
    QMessageBox::critical(this, "Erro!", "Não foi possível cadastrar este endereço!");
    return;
  }

  novoEndereco();
}

void CadastroTransportadora::on_pushButtonAtualizarEnd_clicked() {
  if (not cadastrarEndereco(true)) {
    QMessageBox::critical(this, "Erro!", "Não foi possível atualizar este endereço!");
    return;
  }

  novoEndereco();
}

void CadastroTransportadora::on_pushButtonEndLimpar_clicked() { novoEndereco(); }

void CadastroTransportadora::on_pushButtonRemoverEnd_clicked() {
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

void CadastroTransportadora::on_checkBoxMostrarInativos_clicked(const bool checked) {
  modelEnd.setFilter("idTransportadora = " + data("idTransportadora").toString() + (checked ? "" : " AND desativado = FALSE"));
}

bool CadastroTransportadora::cadastrarEndereco(const bool isUpdate) {
  for (auto const &line : ui->groupBoxEndereco->findChildren<QLineEdit *>()) {
    if (not verifyRequiredField(line)) return false;
  }

  if (not ui->lineEditCEP->isValid()) {
    ui->lineEditCEP->setFocus();
    QMessageBox::critical(this, "Erro!", "CEP inválido!");
    return false;
  }

  currentRowEnd = isUpdate ? mapperEnd.currentIndex() : modelEnd.rowCount();

  if (not isUpdate) modelEnd.insertRow(currentRowEnd);

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

  isDirty = true;

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

void CadastroTransportadora::on_tableEndereco_clicked(const QModelIndex &index) {
  ui->pushButtonAtualizarEnd->show();
  ui->pushButtonAdicionarEnd->hide();
  mapperEnd.setCurrentModelIndex(index);
}

void CadastroTransportadora::setupUi() {
  ui->lineEditCNPJ->setInputMask("99.999.999/9999-99;_");
  ui->lineEditANTT->setInputMask("99999999;_");
  ui->lineEditPlaca->setInputMask("AAA-9999;_");
  ui->lineEditCEP->setInputMask("99999-999;_");
  ui->lineEditUF->setInputMask(">AA;_");
  ui->lineEditUfPlaca->setInputMask(">AA;_");

  ui->lineEditCarga->setValidator(new QIntValidator(this));
}

bool CadastroTransportadora::viewRegister() {
  if (not RegisterDialog::viewRegister()) return false;

  modelEnd.setFilter("idTransportadora = " + data("idTransportadora").toString() + " AND desativado = FALSE");

  if (not modelEnd.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela endereço transportadora: " + modelEnd.lastError().text());
    return false;
  }

  ui->tableEndereco->resizeColumnsToContents();

  modelVeiculo.setFilter("idTransportadora = " + data("idTransportadora").toString() + " AND desativado = FALSE");

  if (not modelVeiculo.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela veículo: " + modelVeiculo.lastError().text());
    return false;
  }

  ui->tableVeiculo->resizeColumnsToContents();

  return true;
}

void CadastroTransportadora::successMessage() { QMessageBox::information(this, "Atenção!", isUpdate ? "Cadastro atualizado!" : "Transportadora cadastrada com sucesso!"); }

void CadastroTransportadora::on_tableEndereco_entered(const QModelIndex &) { ui->tableEndereco->resizeColumnsToContents(); }

bool CadastroTransportadora::cadastrarVeiculo(const bool isUpdate) {
  for (auto const &line : ui->groupBoxVeiculo->findChildren<QLineEdit *>()) {
    if (not verifyRequiredField(line)) return false;
  }

  const int row = isUpdate ? mapperVeiculo.currentIndex() : modelVeiculo.rowCount();

  if (not isUpdate) modelVeiculo.insertRow(row);

  if (not modelVeiculo.setData(row, "modelo", ui->lineEditModelo->text())) return false;
  if (not modelVeiculo.setData(row, "capacidade", ui->lineEditCarga->text().toInt())) return false;

  if (ui->lineEditPlaca->text() != "-") {
    if (not modelVeiculo.setData(row, "placa", ui->lineEditPlaca->text())) return false;
  }

  if (not modelVeiculo.setData(row, "ufPlaca", ui->lineEditUfPlaca->text())) return false;

  ui->tableVeiculo->resizeColumnsToContents();

  isDirty = true;

  return true;
}

void CadastroTransportadora::on_pushButtonAdicionarVeiculo_clicked() {
  if (not cadastrarVeiculo()) {
    QMessageBox::critical(this, "Erro!", "Não foi possível cadastrar este veículo!");
    return;
  }

  novoVeiculo();
}

void CadastroTransportadora::on_pushButtonAtualizarVeiculo_clicked() {
  if (not cadastrarVeiculo(true)) {
    QMessageBox::critical(this, "Erro!", "Não foi possível atualizar este veículo!");
    return;
  }

  novoVeiculo();
}

void CadastroTransportadora::on_pushButtonVeiculoLimpar_clicked() { novoVeiculo(); }

void CadastroTransportadora::novoVeiculo() {
  ui->pushButtonAtualizarVeiculo->hide();
  ui->pushButtonAdicionarVeiculo->show();
  ui->tableVeiculo->clearSelection();
  clearVeiculo();
}

void CadastroTransportadora::clearVeiculo() {
  ui->lineEditModelo->clear();
  ui->lineEditCarga->clear();
  ui->lineEditPlaca->clear();
  ui->lineEditUfPlaca->clear();
}

void CadastroTransportadora::on_pushButtonRemoverVeiculo_clicked() {
  QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Tem certeza que deseja remover?", QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Remover");
  msgBox.setButtonText(QMessageBox::No, "Voltar");

  if (msgBox.exec() == QMessageBox::Yes) {
    if (not setDataEnd("desativado", true)) {
      QMessageBox::critical(this, "Erro!", "Erro marcando desativado!");
      return;
    }

    if (not modelVeiculo.submitAll()) {
      QMessageBox::critical(this, "Erro!", "Não foi possível remover este item: " + modelVeiculo.lastError().text());
      return;
    }

    novoVeiculo();
  }
}

bool CadastroTransportadora::cadastrar() {
  currentRow = isUpdate ? mapper.currentIndex() : model.rowCount();

  if (currentRow == -1) {
    error = "Erro linha -1";
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
    error = "Erro: " + model.lastError().text();
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

  for (int column = 0, columnCount = modelEnd.columnCount(); column < columnCount; ++column) {
    const QVariant dado = modelEnd.data(currentRow, column);
    if (dado.type() == QVariant::String) {
      if (not modelEnd.setData(currentRow, column, dado.toString().toUpper())) return false;
    }
  }

  if (not modelEnd.submitAll()) {
    error = "Erro: " + modelEnd.lastError().text();
    return false;
  }

  for (int row = 0, rowCount = modelVeiculo.rowCount(); row < rowCount; ++row) {
    if (not modelVeiculo.setData(row, primaryKey, primaryId)) return false;
  }

  for (int column = 0, columnCount = modelVeiculo.columnCount(); column < columnCount; ++column) {
    const QVariant dado = modelVeiculo.data(currentRow, column);
    if (dado.type() == QVariant::String) {
      if (not modelVeiculo.setData(currentRow, column, dado.toString().toUpper())) return false;
    }
  }

  if (not modelVeiculo.submitAll()) {
    error = "Erro: " + modelVeiculo.lastError().text();
    return false;
  }

  return true;
}

bool CadastroTransportadora::save() {
  if (not verifyFields()) return false;

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not cadastrar()) {
    QSqlQuery("ROLLBACK").exec();
    if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
    return false;
  }

  QSqlQuery("COMMIT").exec();

  isDirty = false;

  viewRegisterById(primaryId);

  successMessage();

  return true;
}

void CadastroTransportadora::on_checkBoxMostrarInativosVeiculo_toggled(bool checked) {
  modelVeiculo.setFilter("idTransportadora = " + data("idTransportadora").toString() + (checked ? "" : " AND desativado = FALSE"));
}

void CadastroTransportadora::on_tableVeiculo_clicked(const QModelIndex &index) {
  ui->pushButtonAtualizarVeiculo->show();
  ui->pushButtonAdicionarVeiculo->hide();
  mapperVeiculo.setCurrentModelIndex(index);
}

void CadastroTransportadora::on_tableVeiculo_entered(const QModelIndex &) { ui->tableVeiculo->resizeColumnsToContents(); }
