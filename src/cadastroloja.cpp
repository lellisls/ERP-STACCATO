#include <QFileDialog>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>

#include "cadastroloja.h"
#include "ui_cadastroloja.h"
#include "searchdialog.h"
#include "usersession.h"
#include "cepcompleter.h"

CadastroLoja::CadastroLoja(QWidget *parent)
  : RegisterAddressDialog("loja", "idLoja", parent), ui(new Ui::CadastroLoja) {
  ui->setupUi(this);

  setupUi();
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

CadastroLoja::~CadastroLoja() { delete ui; }

void CadastroLoja::setupUi() {
  ui->lineEditCNPJ->setInputMask("99.999.999/9999-99;_");
  ui->lineEditSIGLA->setInputMask(">AANN;_");
  ui->lineEditCEP->setInputMask("99999-999;_");
  ui->lineEditUF->setInputMask(">AA;_");
}

void CadastroLoja::setupTables() {
  modelAlcadas.setTable("alcadas");
  modelAlcadas.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelAlcadas.setFilter("idLoja = " + QString::number(UserSession::getLoja()) + "");

  if (not modelAlcadas.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela de alçadas: " + modelAlcadas.lastError().text());
    return;
  }

  ui->tableAlcadas->setModel(&modelAlcadas);
  ui->tableAlcadas->hideColumn(modelAlcadas.fieldIndex("idAlcada"));
  ui->tableAlcadas->hideColumn(modelAlcadas.fieldIndex("idLoja"));
  ui->tableAlcadas->resizeColumnsToContents();

  ui->tableEndereco->setModel(&modelEnd);
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("idEndereco"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("desativado"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("idLoja"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("codUF"));
}

void CadastroLoja::clearFields() {
  for (auto *line : this->findChildren<QLineEdit *>()) {
    line->clear();
  }
}

bool CadastroLoja::verifyFields() {
  if (not RegisterDialog::verifyFields({ui->lineEditDescricao, ui->lineEditRazaoSocial, ui->lineEditNomeFantasia,
                                       ui->lineEditSIGLA, ui->lineEditCNPJ, ui->lineEditInscEstadual,
                                       ui->lineEditTel})) {
    return false;
  }

  return true;
}

bool CadastroLoja::savingProcedures(const int row) {
  if (not setData(row, "descricao", ui->lineEditDescricao->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando descricao: " + model.lastError().text());
    return false;
  }

  if (not setData(row, "razaoSocial", ui->lineEditRazaoSocial->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando razaoSocial: " + model.lastError().text());
    return false;
  }

  if (not setData(row, "sigla", ui->lineEditSIGLA->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando sigla: " + model.lastError().text());
    return false;
  }

  if (not setData(row, "nomeFantasia", ui->lineEditNomeFantasia->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando nomeFantasia: " + model.lastError().text());
    return false;
  }

  if (not setData(row, "cnpj", ui->lineEditCNPJ->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando CNPJ: " + model.lastError().text());
    return false;
  }

  if (not setData(row, "inscEstadual", ui->lineEditInscEstadual->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando inscEstadual: " + model.lastError().text());
    return false;
  }

  if (not setData(row, "tel", ui->lineEditTel->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando tel: " + model.lastError().text());
    return false;
  }

  if (not setData(row, "tel2", ui->lineEditTel2->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando tel2: " + model.lastError().text());
    return false;
  }

  if (not setData(row, "valorMinimoFrete", ui->doubleSpinBoxValorMinimoFrete->value())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando valorMinimoFrete: " + model.lastError().text());
    return false;
  }

  if (not setData(row, "porcentagemFrete", ui->doubleSpinBoxPorcFrete->value())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando porcentagemFrete: " + model.lastError().text());
    return false;
  }

  if (not setData(row, "servidorACBr", ui->lineEditServidorACBr->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando servidorACBr: " + model.lastError().text());
    return false;
  }

  if (not setData(row, "portaACBr", ui->lineEditPortaACBr->text().toInt())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando portaACBr: " + model.lastError().text());
    return false;
  }

  if (not setData(row, "pastaEntACBr", ui->lineEditPastaEntACBr->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando pastaEntACBr: " + model.lastError().text());
    return false;
  }

  if (not setData(row, "pastaSaiACBr", ui->lineEditPastaSaiACBr->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando pastaSaiACBr: " + model.lastError().text());
    return false;
  }

  if (not setData(row, "pastaXmlACBr", ui->lineEditPastaXmlACBr->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando pastaXmlACBr: " + model.lastError().text());
    return false;
  }

  if (not setData(row, "servidorSMTP", ui->lineEditServidorSMTP->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando servidorSMTP: " + model.lastError().text());
    return false;
  }

  if (not setData(row, "portaSMTP", ui->lineEditPortaSMTP->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando portaSMTP: " + model.lastError().text());
    return false;
  }

  if (not setData(row, "emailCompra", ui->lineEditEmailCompra->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando emailCompra: " + model.lastError().text());
    return false;
  }

  if (not setData(row, "emailSenha", ui->lineEditEmailSenha->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando emailSenha: " + model.lastError().text());
    return false;
  }

  if (not setData(row, "pastaCompra", ui->lineEditPastaCompra->text())) {
    QMessageBox::critical(this, "Erro!", "Erro guardando pastaCompra: " + model.lastError().text());
    return false;
  }

  return true;
}

void CadastroLoja::registerMode() {
  ui->pushButtonCadastrar->show();
  ui->pushButtonAtualizar->hide();
  ui->pushButtonRemover->hide();
}

void CadastroLoja::updateMode() {
  ui->pushButtonCadastrar->hide();
  ui->pushButtonAtualizar->show();
  ui->pushButtonRemover->show();
}

void CadastroLoja::setupMapper() {
  addMapping(ui->lineEditDescricao, "descricao");
  addMapping(ui->lineEditCNPJ, "cnpj");
  addMapping(ui->lineEditRazaoSocial, "razaoSocial");
  addMapping(ui->lineEditNomeFantasia, "nomeFantasia");
  addMapping(ui->lineEditInscEstadual, "inscEstadual");
  addMapping(ui->lineEditTel, "tel");
  addMapping(ui->lineEditTel2, "tel2");
  addMapping(ui->lineEditSIGLA, "sigla");
  addMapping(ui->doubleSpinBoxValorMinimoFrete, "valorMinimoFrete");
  addMapping(ui->doubleSpinBoxPorcFrete, "porcentagemFrete");
  addMapping(ui->lineEditServidorACBr, "servidorACBr");
  addMapping(ui->lineEditPortaACBr, "portaACBr");
  addMapping(ui->lineEditPastaEntACBr, "pastaEntACBr");
  addMapping(ui->lineEditPastaSaiACBr, "pastaSaiACBr");
  addMapping(ui->lineEditPastaXmlACBr, "pastaXmlACBr");
  addMapping(ui->lineEditServidorSMTP, "servidorSMTP");
  addMapping(ui->lineEditPortaSMTP, "portaSMTP");
  addMapping(ui->lineEditEmailCompra, "emailCompra");
  addMapping(ui->lineEditEmailSenha, "emailSenha");
  addMapping(ui->lineEditPastaCompra, "pastaCompra");

  mapperEnd.addMapping(ui->comboBoxTipoEnd, modelEnd.fieldIndex("descricao"));
  mapperEnd.addMapping(ui->lineEditCEP, modelEnd.fieldIndex("CEP"));
  mapperEnd.addMapping(ui->lineEditLogradouro, modelEnd.fieldIndex("logradouro"));
  mapperEnd.addMapping(ui->lineEditNro, modelEnd.fieldIndex("numero"));
  mapperEnd.addMapping(ui->lineEditComp, modelEnd.fieldIndex("complemento"));
  mapperEnd.addMapping(ui->lineEditBairro, modelEnd.fieldIndex("bairro"));
  mapperEnd.addMapping(ui->lineEditCidade, modelEnd.fieldIndex("cidade"));
  mapperEnd.addMapping(ui->lineEditUF, modelEnd.fieldIndex("uf"));
}

void CadastroLoja::on_pushButtonCadastrar_clicked() { save(); }

void CadastroLoja::on_pushButtonAtualizar_clicked() { update(); }

void CadastroLoja::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroLoja::on_pushButtonRemover_clicked() { remove(); }

void CadastroLoja::on_pushButtonCancelar_clicked() { close(); }

void CadastroLoja::on_pushButtonBuscar_clicked() {
  SearchDialog *sdLoja = SearchDialog::loja(this);
  connect(sdLoja, &SearchDialog::itemSelected, this, &CadastroLoja::viewRegisterById);
  sdLoja->show();
}

void CadastroLoja::on_lineEditCNPJ_textEdited(const QString &text) {
  ui->lineEditCNPJ->setStyleSheet(
        validaCNPJ(QString(text).remove(".").remove("/").remove("-")) ? "" : "color: rgb(255, 0, 0);");
}

void CadastroLoja::on_pushButtonEntradaNFe_clicked() {
  const QString dir = QFileDialog::getExistingDirectory(this, "Pasta entrada NFe", QDir::currentPath());

  if (not dir.isEmpty()) {
    ui->lineEditPastaEntACBr->setText(dir);
  }
}

void CadastroLoja::on_pushButtonSaidaNFe_clicked() {
  const QString dir = QFileDialog::getExistingDirectory(this, "Pasta saída NFe", QDir::currentPath());

  if (not dir.isEmpty()) {
    ui->lineEditPastaSaiACBr->setText(dir);
  }
}

void CadastroLoja::on_pushButtonAdicionarEnd_clicked() {
  if (not cadastrarEndereco(false)) {
    QMessageBox::critical(this, "Atenção!", "Não foi possível cadastrar este endereço.");
  } else {
    novoEndereco();
  }
}

void CadastroLoja::on_pushButtonAtualizarEnd_clicked() {
  if (not cadastrarEndereco(true)) {
    QMessageBox::critical(this, "Atenção!", "Não foi possível atualizar este endereço.");
  } else {
    novoEndereco();
  }
}

void CadastroLoja::on_pushButtonEndLimpar_clicked() { novoEndereco(); }

void CadastroLoja::on_pushButtonRemoverEnd_clicked() {
  QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Tem certeza que deseja remover?",
                     QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Sim");
  msgBox.setButtonText(QMessageBox::No, "Não");

  // TODO: is this working? where is setdata?

  if (msgBox.exec() == QMessageBox::Yes) {
    if (not modelEnd.submitAll()) {
      QMessageBox::critical(this, "Erro!", "Não foi possível remover este item: " + modelEnd.lastError().text());
      return;
    }

    if (not modelEnd.select()) {
      QMessageBox::critical(this, "Erro!", "Erro lendo tabela endereço da loja: " + modelEnd.lastError().text());
      return;
    }

    novoEndereco();
  }
}

void CadastroLoja::on_checkBoxMostrarInativos_clicked(const bool checked) {
  modelEnd.setFilter("idLoja = " + data(primaryKey).toString() + (checked ? "" : " AND desativado = FALSE"));
}

bool CadastroLoja::cadastrarEndereco(const bool isUpdate) {
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

void CadastroLoja::novoEndereco() {
  ui->pushButtonAtualizarEnd->hide();
  ui->pushButtonAdicionarEnd->show();
  ui->tableEndereco->clearSelection();
  clearEndereco();
}

void CadastroLoja::clearEndereco() {
  ui->lineEditBairro->clear();
  ui->lineEditCEP->clear();
  ui->lineEditCidade->clear();
  ui->lineEditComp->clear();
  ui->lineEditLogradouro->clear();
  ui->lineEditNro->clear();
  ui->lineEditUF->clear();
}

void CadastroLoja::on_lineEditCEP_textChanged(const QString &cep) {
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

void CadastroLoja::on_tableEndereco_clicked(const QModelIndex &index) {
  ui->pushButtonAtualizarEnd->show();
  ui->pushButtonAdicionarEnd->hide();
  mapperEnd.setCurrentModelIndex(index);
}

void CadastroLoja::show() {
  QWidget::show();
  adjustSize();
}

bool CadastroLoja::viewRegister(const QModelIndex index) {
  if (not RegisterDialog::viewRegister(index)) {
    return false;
  }

  modelEnd.setFilter("idLoja = " + data(primaryKey).toString() + " AND desativado = FALSE");

  if (not modelEnd.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela endereço da loja: " + modelEnd.lastError().text());
    return false;
  }

  ui->tableEndereco->resizeColumnsToContents();

  return true;
}

void CadastroLoja::on_pushButtonPastaCompra_clicked() {
  const QString dir = QFileDialog::getExistingDirectory(this, "Pasta Excel Compras", QDir::currentPath());

  if (not dir.isEmpty()) {
    ui->lineEditPastaCompra->setText(dir);
  }
}

void CadastroLoja::on_pushButtonXmlNFe_clicked() {
  const QString dir = QFileDialog::getExistingDirectory(this, "Pasta xml NFe", QDir::currentPath());

  if (not dir.isEmpty()) {
    ui->lineEditPastaXmlACBr->setText(dir);
  }
}

// TODO: remove this?
void CadastroLoja::on_pushButtonPastaExcel_clicked() {}

// TODO: parametrizar taxa cartao credito
