#include <QDebug>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "cadastroloja.h"
#include "ui_cadastroloja.h"
#include "searchdialog.h"
#include "usersession.h"
#include "cepcompleter.h"

CadastroLoja::CadastroLoja(QWidget *parent) : RegisterDialog("Loja", "idLoja", parent), ui(new Ui::CadastroLoja) {
  ui->setupUi(this);

  ui->lineEditCNPJ->setInputMask("99.999.999/9999-99;_");
  ui->lineEditSIGLA->setInputMask(">AANN;_");
  ui->lineEditCEP->setInputMask("99999-999;_");
  ui->lineEditUF->setInputMask(">AA;_");

  modelAlcadas.setTable("Alcadas");
  modelAlcadas.setEditStrategy(QSqlRelationalTableModel::OnManualSubmit);
  modelAlcadas.setFilter("idLoja = " + QString::number(UserSession::getLoja()) + "");

  if (not modelAlcadas.select()) {
    qDebug() << "Erro carregando alçadas: " << modelAlcadas.lastError();
  }

  ui->tableAlcadas->setModel(&modelAlcadas);
  ui->tableAlcadas->hideColumn(modelAlcadas.fieldIndex("idAlcada"));
  ui->tableAlcadas->hideColumn(modelAlcadas.fieldIndex("idLoja"));
  ui->tableAlcadas->resizeColumnsToContents();

  modelEnd.setTable("Loja_has_Endereco");
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
  modelEnd.select();

  ui->tableEndereco->setModel(&modelEnd);
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("idEndereco"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("desativado"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("idLoja"));

  setupMapper();
  newRegister();

  if (UserSession::getTipoUsuario() != "ADMINISTRADOR") {
    ui->pushButtonRemover->setDisabled(true);
    ui->pushButtonRemoverEnd->setDisabled(true);
  }
}

CadastroLoja::~CadastroLoja() { delete ui; }

void CadastroLoja::clearFields() {
  foreach (QLineEdit *line, this->findChildren<QLineEdit *>()) { line->clear(); }
}

bool CadastroLoja::verifyFields(int row) {
  Q_UNUSED(row);

  if (not RegisterDialog::verifyFields({ui->lineEditDescricao, ui->lineEditRazaoSocial, ui->lineEditNomeFantasia,
                                       ui->lineEditSIGLA, ui->lineEditCNPJ, ui->lineEditInscEstadual,
                                       ui->lineEditTel})) {
    return false;
  }

  return true;
}

bool CadastroLoja::savingProcedures(int row) {
  setData(row, "descricao", ui->lineEditDescricao->text());
  setData(row, "razaoSocial", ui->lineEditRazaoSocial->text());
  setData(row, "sigla", ui->lineEditSIGLA->text());
  setData(row, "nomeFantasia", ui->lineEditNomeFantasia->text());
  setData(row, "cnpj", ui->lineEditCNPJ->text());
  setData(row, "inscEstadual", ui->lineEditInscEstadual->text());
  setData(row, "tel", ui->lineEditTel->text());
  setData(row, "valorMinimoFrete", ui->doubleSpinBoxValorMinimoFrete->value());
  setData(row, "porcentagemFrete", ui->doubleSpinBoxPorcFrete->value());
  setData(row, "servidorACBr", ui->lineEditServidorACBr->text());
  setData(row, "portaACBr", ui->lineEditPortaACBr->text().toInt());
  setData(row, "pastaEntACBr", ui->lineEditPastaEntACBr->text());
  setData(row, "pastaSaiACBr", ui->lineEditPastaSaiACBr->text());
  setData(row, "pastaXmlACBr", ui->lineEditPastaXmlACBr->text());

  if (not model.submitAll()) {
    qDebug() << objectName() << " : " << __LINE__ << " : Error on model.submitAll() : " << model.lastError();
    return false;
  }

  int idLoja = data(row, primaryKey).toInt();

  if (not data(row, primaryKey).isValid()) {
    idLoja = model.query().lastInsertId().toInt();
  }

  for (int row = 0; row < modelEnd.rowCount(); ++row) {
    modelEnd.setData(model.index(row, modelEnd.fieldIndex(primaryKey)), idLoja);
  }

  if (not modelEnd.submitAll()) {
    qDebug() << objectName() << " : " << __LINE__ << " : Error on modelEnd.submitAll() : " << modelEnd.lastError();
    qDebug() << "QUERY : " << modelEnd.query().lastQuery();
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
  addMapping(ui->lineEditSIGLA, "sigla");
  addMapping(ui->doubleSpinBoxValorMinimoFrete, "valorMinimoFrete");
  addMapping(ui->doubleSpinBoxPorcFrete, "porcentagemFrete");
  addMapping(ui->lineEditServidorACBr, "servidorACBr");
  addMapping(ui->lineEditPortaACBr, "portaACBr");
  addMapping(ui->lineEditPastaEntACBr, "pastaEntACBr");
  addMapping(ui->lineEditPastaSaiACBr, "pastaSaiACBr");
  addMapping(ui->lineEditPastaXmlACBr, "pastaXmlACBr");

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

bool CadastroLoja::viewRegister(QModelIndex index) {
  if (not RegisterDialog::viewRegister(index)) {
    return false;
  }

  modelEnd.setFilter("idLoja = " + data(primaryKey).toString() + " AND desativado = false");

  if (not modelEnd.select()) {
    qDebug() << modelEnd.lastError();
  }

  return true;
}

void CadastroLoja::on_pushButtonCadastrar_clicked() { save(); }

void CadastroLoja::on_pushButtonAtualizar_clicked() { save(); }

void CadastroLoja::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroLoja::on_pushButtonRemover_clicked() { remove(); }

void CadastroLoja::on_pushButtonCancelar_clicked() { close(); }

void CadastroLoja::on_pushButtonBuscar_clicked() {
  SearchDialog *sdLoja = SearchDialog::loja(this);
  connect(sdLoja, &SearchDialog::itemSelected, this, &CadastroLoja::changeItem);
  sdLoja->show();
}

void CadastroLoja::changeItem(QVariant value, QString text) {
  Q_UNUSED(text);

  viewRegisterById(value);
}

void CadastroLoja::on_lineEditCNPJ_textEdited(const QString &) {
  QString text = ui->lineEditCNPJ->text().remove(".").remove("/").remove("-");
  validaCNPJ(text);
}

void CadastroLoja::validaCNPJ(QString text) {
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

void CadastroLoja::on_pushButtonEntradaNFe_clicked() {
  QString dir = QFileDialog::getExistingDirectory(this, "Pasta entrada NFe", QDir::currentPath());

  if (not dir.isEmpty()) {
    ui->lineEditPastaEntACBr->setText(dir);
  }
}

void CadastroLoja::on_pushButtonSaidaNFe_clicked() {
  QString dir = QFileDialog::getExistingDirectory(this, "Pasta saída NFe", QDir::currentPath());

  if (not dir.isEmpty()) {
    ui->lineEditPastaSaiACBr->setText(dir);
  }
}

void CadastroLoja::on_pushButtonAdicionarEnd_clicked() {
  if (not adicionarEndereco()) {
    QMessageBox::warning(this, "Atenção!", "Não foi possível cadastrar este endereço.", QMessageBox::Ok,
                         QMessageBox::NoButton);
  }
}

void CadastroLoja::on_pushButtonAtualizarEnd_clicked() {
  if (not atualizarEndereco()) {
    QMessageBox::warning(this, "Atenção!", "Não foi possível atualizar este endereço.", QMessageBox::Ok,
                         QMessageBox::NoButton);
  }
}

void CadastroLoja::on_pushButtonEndLimpar_clicked() { novoEnd(); }

void CadastroLoja::on_pushButtonRemoverEnd_clicked() {
  QMessageBox msgBox(QMessageBox::Warning, "Atenção!", "Tem certeza que deseja remover?",
                     QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Sim");
  msgBox.setButtonText(QMessageBox::No, "Não");
  if (msgBox.exec() == QMessageBox::Yes) {
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

void CadastroLoja::on_checkBoxMostrarInativos_clicked(bool checked) {
  if (checked) {
    modelEnd.setFilter("idLoja = " + data(primaryKey).toString());
  } else {
    modelEnd.setFilter("idLoja = " + data(primaryKey).toString() + " AND desativado = false");
  }
}

bool CadastroLoja::adicionarEndereco() {
  if (not RegisterDialog::verifyFields({ui->lineEditCEP, ui->lineEditLogradouro, ui->lineEditNro, ui->lineEditBairro,
                                       ui->lineEditCidade, ui->lineEditUF})) {
    return false;
  }

  if (not ui->lineEditCEP->isValid()) {
    ui->lineEditCEP->setFocus();
    QMessageBox::warning(this, "Atenção!", "CEP inválido!", QMessageBox::Ok, QMessageBox::NoButton);
    return false;
  }

  modelEnd.insertRow(modelEnd.rowCount());
  int row = modelEnd.rowCount() - 1;

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

bool CadastroLoja::atualizarEndereco() {
  if (not RegisterDialog::verifyFields({ui->lineEditCEP, ui->lineEditLogradouro, ui->lineEditNro, ui->lineEditBairro,
                                       ui->lineEditCidade, ui->lineEditUF})) {
    return false;
  }

  if (not ui->lineEditCEP->isValid()) {
    ui->lineEditCEP->setFocus();
    QMessageBox::warning(this, "Atenção!", "CEP inválido!", QMessageBox::Ok, QMessageBox::NoButton);
    return false;
  }

  int row = mapperEnd.currentIndex();

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

void CadastroLoja::novoEnd() {
  ui->pushButtonAtualizarEnd->hide();
  ui->pushButtonAdicionarEnd->show();
  ui->tableEndereco->clearSelection();
  clearEnd();
}

void CadastroLoja::clearEnd() {
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
  if (modelEnd.isDirty()) {
    QMessageBox msgBox(QMessageBox::Warning, "Atenção!", "Deseja aplicar as alterações?",
                       QMessageBox::Yes | QMessageBox::No);
    msgBox.setButtonText(QMessageBox::Yes, "Sim");
    msgBox.setButtonText(QMessageBox::No, "Não");

    if (msgBox.exec() == QMessageBox::Yes) {
      if (not atualizarEndereco()) {
        return;
      }
    }
  }

  ui->pushButtonAtualizarEnd->show();
  ui->pushButtonAdicionarEnd->hide();
  mapperEnd.setCurrentModelIndex(index);
}

void CadastroLoja::show() {
  QWidget::show();
  adjustSize();
}
