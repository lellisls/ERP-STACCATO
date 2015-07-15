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
  : RegisterAddressDialog("Loja", "idLoja", parent), ui(new Ui::CadastroLoja) {
  ui->setupUi(this);

  setupUi();
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

CadastroLoja::~CadastroLoja() { delete ui; }

void CadastroLoja::setupUi() {
  ui->lineEditCNPJ->setInputMask("99.999.999/9999-99;_");
  ui->lineEditSIGLA->setInputMask(">AANN;_");
  ui->lineEditCEP->setInputMask("99999-999;_");
  ui->lineEditUF->setInputMask(">AA;_");
}

void CadastroLoja::setupTables() {
  modelAlcadas.setTable("Alcadas");
  modelAlcadas.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelAlcadas.setFilter("idLoja = " + QString::number(UserSession::getLoja()) + "");

  if (not modelAlcadas.select()) {
    qDebug() << "Erro carregando alçadas: " << modelAlcadas.lastError();
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
  foreach (QLineEdit *line, this->findChildren<QLineEdit *>()) { line->clear(); }
}

bool CadastroLoja::verifyFields(const int row) {
  Q_UNUSED(row);

  if (not RegisterDialog::verifyFields({ui->lineEditDescricao, ui->lineEditRazaoSocial, ui->lineEditNomeFantasia,
                                       ui->lineEditSIGLA, ui->lineEditCNPJ, ui->lineEditInscEstadual,
                                       ui->lineEditTel})) {
    return false;
  }

  return true;
}

bool CadastroLoja::savingProcedures(const int row) {
  if (not setData(row, "descricao", ui->lineEditDescricao->text())) {
    qDebug() << "erro setando descricao";
    return false;
  }

  if (not setData(row, "razaoSocial", ui->lineEditRazaoSocial->text())) {
    qDebug() << "erro setando razaoSocial";
    return false;
  }

  if (not setData(row, "sigla", ui->lineEditSIGLA->text())) {
    qDebug() << "erro setando sigla";
    return false;
  }

  if (not setData(row, "nomeFantasia", ui->lineEditNomeFantasia->text())) {
    qDebug() << "erro setando nomeFantasia";
    return false;
  }

  if (not setData(row, "cnpj", ui->lineEditCNPJ->text())) {
    qDebug() << "erro setando cnpj";
    return false;
  }

  if (not setData(row, "inscEstadual", ui->lineEditInscEstadual->text())) {
    qDebug() << "erro setando inscEstadual";
    return false;
  }

  if (not setData(row, "tel", ui->lineEditTel->text())) {
    qDebug() << "erro setando tel";
    return false;
  }

  if (not setData(row, "tel2", ui->lineEditTel2->text())) {
    qDebug() << "erro setando tel2";
    return false;
  }

  if (not setData(row, "valorMinimoFrete", ui->doubleSpinBoxValorMinimoFrete->value())) {
    qDebug() << "erro setando valorMinimoFrete";
    return false;
  }

  if (not setData(row, "porcentagemFrete", ui->doubleSpinBoxPorcFrete->value())) {
    qDebug() << "erro setando porcentagemFrete";
    return false;
  }

  if (not setData(row, "servidorACBr", ui->lineEditServidorACBr->text())) {
    qDebug() << "erro setando servidorACBr";
    return false;
  }

  if (not setData(row, "portaACBr", ui->lineEditPortaACBr->text().toInt())) {
    qDebug() << "erro setando portaACBr";
    return false;
  }

  if (not setData(row, "pastaEntACBr", ui->lineEditPastaEntACBr->text())) {
    qDebug() << "erro setando pastaEntACBr";
    return false;
  }

  if (not setData(row, "pastaSaiACBr", ui->lineEditPastaSaiACBr->text())) {
    qDebug() << "erro setando pastaSaiACBr";
    return false;
  }

  if (not setData(row, "pastaXmlACBr", ui->lineEditPastaXmlACBr->text())) {
    qDebug() << "erro setando pastaXmlACBr";
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
  if (not validaCNPJ(QString(text).remove(".").remove("/").remove("-"))) {
    ui->lineEditCNPJ->setStyleSheet("color: rgb(255, 0, 0);");
  } else {
    ui->lineEditCNPJ->setStyleSheet("");
  }
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
    QMessageBox::warning(this, "Atenção!", "Não foi possível cadastrar este endereço.", QMessageBox::Ok,
                         QMessageBox::NoButton);
  }
}

void CadastroLoja::on_pushButtonAtualizarEnd_clicked() {
  if (not cadastrarEndereco(true)) {
    QMessageBox::warning(this, "Atenção!", "Não foi possível atualizar este endereço.", QMessageBox::Ok,
                         QMessageBox::NoButton);
  }
}

void CadastroLoja::on_pushButtonEndLimpar_clicked() { novoEndereco(); }

void CadastroLoja::on_pushButtonRemoverEnd_clicked() {
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

void CadastroLoja::on_checkBoxMostrarInativos_clicked(const bool checked) {
  if (checked) {
    modelEnd.setFilter("idLoja = " + data(primaryKey).toString());
  } else {
    modelEnd.setFilter("idLoja = " + data(primaryKey).toString() + " AND desativado = FALSE");
  }
}

bool CadastroLoja::cadastrarEndereco(const bool isUpdate) {
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

  if (not setDataEnd(row, "codUF", getCodigoUF(ui->lineEditUF->text().toLower()))) {
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
    QMessageBox::warning(this, "Aviso!", "CEP não encontrado!", QMessageBox::Ok);
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
    qDebug() << "erro: " << modelEnd.lastError();
  }

  ui->tableEndereco->resizeColumnsToContents();

  return true;
}
