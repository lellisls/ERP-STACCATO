#include <QDebug>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

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

void CadastroLoja::setupTables() {
  modelAlcadas.setTable("Alcadas");
  modelAlcadas.setEditStrategy(QSqlRelationalTableModel::OnManualSubmit);
  modelAlcadas.setFilter("idLoja = " + QString::number(UserSession::getLoja()) + "");

  if (not modelAlcadas.select()) {
    qDebug() << "Erro carregando alçadas: " << modelAlcadas.lastError();
    return;
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

  if (not modelEnd.select()) {
    qDebug() << "erro modelEnd: " << modelEnd.lastError();
    return;
  }

  ui->tableEndereco->setModel(&modelEnd);
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("idEndereco"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("desativado"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("idLoja"));
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

  const int idLoja =
      (data(row, primaryKey).isValid()) ? data(row, primaryKey).toInt() : model.query().lastInsertId().toInt();

  for (int row = 0, rowCount = modelEnd.rowCount(); row < rowCount; ++row) {
    modelEnd.setData(model.index(row, modelEnd.fieldIndex(primaryKey)), idLoja);
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

bool CadastroLoja::viewRegister(const QModelIndex index) {
  if (not RegisterDialog::viewRegister(index)) {
    return false;
  }

  modelEnd.setFilter("idLoja = " + data(primaryKey).toString() + " AND desativado = FALSE");

  if (not modelEnd.select()) {
    qDebug() << modelEnd.lastError();
    return false;
  }

  return true;
}

void CadastroLoja::on_pushButtonCadastrar_clicked() { save(); }

void CadastroLoja::on_pushButtonAtualizar_clicked() { update(); }

void CadastroLoja::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroLoja::on_pushButtonRemover_clicked() { remove(); }

void CadastroLoja::on_pushButtonCancelar_clicked() { close(); }

void CadastroLoja::on_pushButtonBuscar_clicked() {
  SearchDialog *sdLoja = SearchDialog::loja(this);
  connect(sdLoja, &SearchDialog::itemSelected, this, &CadastroLoja::changeItem);
  sdLoja->show();
}

void CadastroLoja::changeItem(const QVariant value) { viewRegisterById(value); }

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

int CadastroLoja::getCodigoUF() {
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
