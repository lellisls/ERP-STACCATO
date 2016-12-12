#include <QDate>
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlError>

#include "cadastrofornecedor.h"
#include "cepcompleter.h"
#include "searchdialog.h"
#include "ui_cadastrofornecedor.h"
#include "usersession.h"

CadastroFornecedor::CadastroFornecedor(QWidget *parent)
    : RegisterAddressDialog("fornecedor", "idFornecedor", parent), ui(new Ui::CadastroFornecedor) {
  ui->setupUi(this);

  setAttribute(Qt::WA_DeleteOnClose);

  //  for (auto const *line : findChildren<QLineEdit *>()) {
  //    connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);
  //  }

  setupUi();
  setupTables();
  setupMapper();
  newRegister();

  if (UserSession::tipoUsuario() != "ADMINISTRADOR") {
    ui->pushButtonRemover->setDisabled(true);
    ui->pushButtonRemoverEnd->setDisabled(true);
  }
}

CadastroFornecedor::~CadastroFornecedor() { delete ui; }

void CadastroFornecedor::setupTables() {
  ui->tableEndereco->setModel(&modelEnd);
  ui->tableEndereco->hideColumn("idEndereco");
  ui->tableEndereco->hideColumn("desativado");
  ui->tableEndereco->hideColumn("idFornecedor");
  ui->tableEndereco->hideColumn("codUF");
}

void CadastroFornecedor::setupUi() {
  ui->lineEditContatoCPF->setInputMask("999.999.999-99;_");
  ui->lineEditContatoRG->setInputMask("99.999.999-9;_");
  ui->lineEditIdNextel->setInputMask("99*9999999*99999;_");
  ui->lineEditCNPJ->setInputMask("99.999.999/9999-99;_");
  ui->lineEditCEP->setInputMask("99999-999;_");
  ui->lineEditUF->setInputMask(">AA;_");
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
  for (auto const &line : ui->frameCadastro->findChildren<QLineEdit *>()) {
    if (not verifyRequiredField(line)) return false;
  }

  if (ui->lineEditCNPJ->styleSheet().contains("color: rgb(255, 0, 0)")) {
    QMessageBox::critical(this, "Erro!", "CNPJ inválido!");
    return false;
  }

  return true;
}

bool CadastroFornecedor::savingProcedures() {
  if (not setData("razaoSocial", ui->lineEditFornecedor->text())) return false;
  if (not setData("nomeFantasia", ui->lineEditNomeFantasia->text())) return false;
  if (not setData("contatoNome", ui->lineEditContatoNome->text())) return false;
  if (not setData("contatoCPF", ui->lineEditContatoCPF->text().remove(".").remove("-"))) return false;
  if (not setData("contatoApelido", ui->lineEditContatoApelido->text())) return false;
  if (not setData("contatoRG", ui->lineEditContatoRG->text().remove(".").remove("-"))) return false;
  if (not setData("cnpj", ui->lineEditCNPJ->text().remove(".").remove("/").remove("-"))) return false;
  if (not setData("inscEstadual", ui->lineEditInscEstadual->text())) return false;
  if (not setData("tel", ui->lineEditTel_Res->text())) return false;
  if (not setData("telCel", ui->lineEditTel_Cel->text())) return false;
  if (not setData("telCom", ui->lineEditTel_Com->text())) return false;
  if (not setData("nextel", ui->lineEditNextel->text())) return false;
  if (not setData("email", ui->lineEditEmail->text())) return false;
  if (not setData("aliquotaSt", ui->doubleSpinBoxAliquotaSt->value())) return false;
  if (not setData("st", ui->comboBoxSt->currentText())) return false;

  return true;
}

void CadastroFornecedor::clearFields() {
  RegisterDialog::clearFields();
  novoEndereco();
  setupUi();
}

void CadastroFornecedor::setupMapper() {
  addMapping(ui->lineEditCNPJ, "cnpj");
  addMapping(ui->lineEditContatoApelido, "contatoApelido");
  addMapping(ui->lineEditContatoApelido, "contatoApelido");
  addMapping(ui->lineEditContatoCPF, "contatoCPF");
  addMapping(ui->lineEditContatoNome, "contatoNome");
  addMapping(ui->lineEditContatoNome, "contatoNome");
  addMapping(ui->lineEditContatoRG, "contatoRG");
  addMapping(ui->lineEditContatoRG, "contatoRG");
  addMapping(ui->lineEditEmail, "email");
  addMapping(ui->lineEditFornecedor, "razaoSocial");
  addMapping(ui->lineEditIdNextel, "idNextel");
  addMapping(ui->lineEditInscEstadual, "inscEstadual");
  addMapping(ui->lineEditNextel, "nextel");
  addMapping(ui->lineEditNomeFantasia, "nomeFantasia");
  addMapping(ui->lineEditTel_Cel, "telCel");
  addMapping(ui->lineEditTel_Com, "telCom");
  addMapping(ui->lineEditTel_Res, "tel");
  addMapping(ui->checkBoxRepresentacao, "representacao", "checked");
  addMapping(ui->comboBoxSt, "st");
  addMapping(ui->doubleSpinBoxAliquotaSt, "aliquotaSt");

  mapperEnd.addMapping(ui->comboBoxTipoEnd, modelEnd.fieldIndex("descricao"));
  mapperEnd.addMapping(ui->lineEditBairro, modelEnd.fieldIndex("bairro"));
  mapperEnd.addMapping(ui->lineEditCEP, modelEnd.fieldIndex("CEP"));
  mapperEnd.addMapping(ui->lineEditCidade, modelEnd.fieldIndex("cidade"));
  mapperEnd.addMapping(ui->lineEditComp, modelEnd.fieldIndex("complemento"));
  mapperEnd.addMapping(ui->lineEditEndereco, modelEnd.fieldIndex("logradouro"));
  mapperEnd.addMapping(ui->lineEditNro, modelEnd.fieldIndex("numero"));
  mapperEnd.addMapping(ui->lineEditUF, modelEnd.fieldIndex("uf"));
}

void CadastroFornecedor::registerMode() {
  ui->pushButtonCadastrar->show();
  ui->pushButtonAtualizar->hide();
  ui->pushButtonRemover->hide();
  ui->pushButtonValidade->hide();
}

void CadastroFornecedor::updateMode() {
  ui->pushButtonCadastrar->hide();
  ui->pushButtonAtualizar->show();
  ui->pushButtonRemover->show();
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

void CadastroFornecedor::on_lineEditCNPJ_textEdited(const QString &text) {
  ui->lineEditCNPJ->setStyleSheet(validaCNPJ(QString(text).remove(".").remove("/").remove("-"))
                                      ? "background-color: rgb(255, 255, 127)"
                                      : "background-color: rgb(255, 255, 127);color: rgb(255, 0, 0)");
}

void CadastroFornecedor::on_lineEditContatoCPF_textEdited(const QString &text) {
  ui->lineEditContatoCPF->setStyleSheet(validaCPF(QString(text).remove(".").remove("-")) ? ""
                                                                                         : "color: rgb(255, 0, 0)");
}

void CadastroFornecedor::on_pushButtonAdicionarEnd_clicked() {
  cadastrarEndereco(false)
      ? novoEndereco()
      : static_cast<void>(QMessageBox::critical(this, "Erro!", "Não foi possível cadastrar este endereço."));
}

bool CadastroFornecedor::cadastrarEndereco(const bool isUpdate) {
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
  if (not setDataEnd("logradouro", ui->lineEditEndereco->text())) return false;
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

void CadastroFornecedor::on_lineEditCEP_textChanged(const QString &cep) {
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
  ui->lineEditEndereco->setText(cc.getEndereco());
  ui->lineEditBairro->setText(cc.getBairro());
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

bool CadastroFornecedor::viewRegister() {
  if (not RegisterDialog::viewRegister()) return false;

  modelEnd.setFilter("idFornecedor = " + data("idFornecedor").toString() + " AND desativado = FALSE");

  if (not modelEnd.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela endereço do fornecedor: " + modelEnd.lastError().text());
    return false;
  }

  ui->tableEndereco->resizeColumnsToContents();

  ui->pushButtonValidade->show();

  return true;
}

void CadastroFornecedor::on_pushButtonRemoverEnd_clicked() {
  QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Tem certeza que deseja remover?",
                     QMessageBox::Yes | QMessageBox::No, this);
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

void CadastroFornecedor::successMessage() {
  QMessageBox::information(this, "Atenção!", isUpdate ? "Cadastro atualizado!" : "Fornecedor cadastrado com sucesso!");
}

void CadastroFornecedor::on_tableEndereco_entered(const QModelIndex &) { ui->tableEndereco->resizeColumnsToContents(); }

bool CadastroFornecedor::ajustarValidade() {
  const QString fornecedor = model.data(mapper.currentIndex(), "razaoSocial").toString();

  bool ok = true;

  const int newValidade =
      QInputDialog::getInt(this, "Validade", "Quantos dias de validade para os produtos: ", 0, 0, 1000, 1, &ok);

  if (not ok) return false;

  QSqlQuery query;
  query.prepare("UPDATE produto SET validade = :newValidade WHERE fornecedor = :fornecedor AND descontinuado = 0");
  query.bindValue(":newValidade", QDate::currentDate().addDays(newValidade));
  query.bindValue(":fornecedor", fornecedor);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando validade nos produtos: " + query.lastError().text());
    return false;
  }

  query.prepare("UPDATE fornecedor SET validadeProdutos = :newValidade WHERE razaoSocial = :fornecedor");
  query.bindValue(":newValidade", QDate::currentDate().addDays(newValidade));
  query.bindValue(":fornecedor", fornecedor);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando validade no fornecedor: " + query.lastError().text());
    return false;
  }

  return true;
}

void CadastroFornecedor::on_pushButtonValidade_clicked() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not ajustarValidade()) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  QMessageBox::information(this, "Aviso!", "Validade alterada com sucesso!");
}
