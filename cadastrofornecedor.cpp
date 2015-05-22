#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

#include "cadastrofornecedor.h"
#include "ui_cadastrofornecedor.h"
#include "searchdialog.h"

CadastroFornecedor::CadastroFornecedor(bool closeBeforeUpdate, QWidget *parent)
  : RegisterDialog("Fornecedor", "idFornecedor", parent), ui(new Ui::CadastroFornecedor),
    closeBeforeUpdate(closeBeforeUpdate) {
  ui->setupUi(this);
  ui->lineEditCEP->setInputMask("99999-999;_");
  ui->lineEditUF->setInputMask(">AA;_");
  ui->pushButtonMostrarInativos->hide();
  modelEnd.setTable("Fornecedor_has_Endereco");
  modelEnd.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelEnd.setHeaderData(modelEnd.fieldIndex("descricao"), Qt::Horizontal, "Descrição");
  modelEnd.setHeaderData(modelEnd.fieldIndex("cep"), Qt::Horizontal, "CEP");
  modelEnd.setHeaderData(modelEnd.fieldIndex("logradouro"), Qt::Horizontal, "Logradouro");
  modelEnd.setHeaderData(modelEnd.fieldIndex("numero"), Qt::Horizontal, "Número");
  modelEnd.setHeaderData(modelEnd.fieldIndex("complemento"), Qt::Horizontal, "Compl.");
  modelEnd.setHeaderData(modelEnd.fieldIndex("bairro"), Qt::Horizontal, "Bairro");
  modelEnd.setHeaderData(modelEnd.fieldIndex("cidade"), Qt::Horizontal, "Cidade");
  modelEnd.setHeaderData(modelEnd.fieldIndex("uf"), Qt::Horizontal, "UF");
  modelEnd.setFilter("idFornecedor = '" + data(primaryKey).toString() + "'");
  modelEnd.select();

  ui->tableEndereco->setModel(&modelEnd);
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("idEndereco"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("ativo"));
  ui->tableEndereco->hideColumn(modelEnd.fieldIndex("idFornecedor"));

  mapperEnd.setModel(&modelEnd);
  setupUi();

  setupMapper();
  newRegister();
}

CadastroFornecedor::~CadastroFornecedor() { delete ui; }

void CadastroFornecedor::setupUi() {
  ui->lineEditContatoCPF->setInputMask("999.999.999-99;_");
  ui->lineEditContatoRG->setInputMask("99.999.999-9;_");
  ui->lineEditIdNextel->setInputMask("99*9999999*99999;_");
  ui->lineEditCNPJ->setInputMask("99.999.999/9999-99;_");

  // Placeholders
  ui->lineEditContatoCPF->setPlaceholderText("999.999.999-99");
  ui->lineEditEmail->setPlaceholderText("usuario@email.com");
  ui->lineEditNextel->setPlaceholderText("(99)99999-9999");
}

void CadastroFornecedor::enableEditor() {
  ui->frame->setEnabled(true);
  ui->frame_2->setEnabled(true);
}

void CadastroFornecedor::disableEditor() {
  ui->frame->setEnabled(false);
  ui->frame_2->setEnabled(false);
}

void CadastroFornecedor::show() {
  adjustSize();
  QWidget::show();
}

bool CadastroFornecedor::viewRegister(QModelIndex idx) {
  if (not confirmationMessage()) {
    return false;
  }
  clearFields();
  updateMode();
  model.select();
  mapper.setCurrentModelIndex(idx);

  qDebug() << "filtro endereco: " << data(primaryKey).toString();
  modelEnd.setFilter("idFornecedor = '" + data(primaryKey).toString() + "'");
  if (not modelEnd.select()) {
    qDebug() << modelEnd.lastError();
  }

  show();
  adjustSize();
  return true;
}

void CadastroFornecedor::clearEnd() {
  ui->lineEditBairro->clear();
  ui->lineEditCEP->clear();
  ui->lineEditCidade->clear();
  ui->lineEditComp->clear();
  ui->lineEditEndereco->clear();
  ui->lineEditNro->clear();
  ui->lineEditUF->clear();
}

void CadastroFornecedor::novoEnd() {
  ui->pushButtonAtualizarEnd->hide();
  ui->pushButtonAdicionarEnd->show();
  ui->tableEndereco->clearSelection();
  mapper.setCurrentIndex(-1);
  clearEnd();
}

bool CadastroFornecedor::verifyFields(int row) {
  //  if (not RegisterDialog::verifyFields({ui->lineEditNome, ui->lineEditCPF}))
  //    return false;

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
  //  qDebug() << "size: " << ui->groupBoxContatos->findChildren<QLineEdit *>().size();
  //  qDebug() << "ok: " << ok;

  if (ok == ui->groupBoxContatos->findChildren<QLineEdit *>().size()) {
    setData(row, "incompleto", false);
  } else {
    setData(row, "incompleto", true);
    return true;
  }

  ok = 0;
  foreach (QLineEdit *line, ui->groupBoxPJuridica->findChildren<QLineEdit *>()) {
    if (not verifyRequiredField(line, true)) {
      //      return false;
      qDebug() << "Faltou " << line->objectName();
    } else {
      ok++;
    }
  }

  //  qDebug() << "size: " << ui->groupBoxPJuridica->findChildren<QLineEdit *>().size();
  //  qDebug() << "ok: " << ok;

  if (ok == ui->groupBoxPJuridica->findChildren<QLineEdit *>().size()) {
    setData(row, "incompleto", false);
  } else {
    setData(row, "incompleto", true);
    return true;
  }

  return true;
}

bool CadastroFornecedor::savingProcedures(int row) {
  if (data(primaryKey).isNull()) {
    QSqlQuery queryKey("SELECT MAX(" + primaryKey + ") FROM " + model.tableName());
    double key;
    if (queryKey.first()) {
      key = qMax(queryKey.value(0).toInt() + 1, 1000);
      setData(row, primaryKey, key);
    }
  }

  if (not ui->lineEditFornecedor->text().isEmpty()) {
    setData(row, "razaoSocial", ui->lineEditFornecedor->text());
  }
  if (not ui->lineEditNomeFantasia->text().isEmpty()) {
    setData(row, "nomeFantasia", ui->lineEditNomeFantasia->text());
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

  if (not model.submitAll()) {
    qDebug() << objectName() << " : " << __LINE__ << " : Error on model.submitAll() : " << model.lastError();
    return false;
  }

  int idFornecedor = data(row, primaryKey).toInt();
  if (not data(row, primaryKey).isValid()) {
    QSqlQuery qryLastId("SELECT LAST_INSERT_ID() AS lastId;");
    qryLastId.exec();
    qryLastId.first();
    idFornecedor = qryLastId.value("lastId").toInt();
  }
  qDebug() << "modelEnd.rowCount() = " << modelEnd.rowCount();

  qDebug() << "ID Fornecedor = " << idFornecedor;
  for (int end = 0; end < modelEnd.rowCount(); ++end) {
    modelEnd.setData(modelEnd.index(end, modelEnd.fieldIndex(primaryKey)), idFornecedor);
  }
  if (not modelEnd.submitAll()) {
    qDebug() << objectName() << " : " << __LINE__
             << " : Error on modelEnd.submitAll() : " << modelEnd.lastError();
    qDebug() << "QUERY : " << modelEnd.query().lastQuery();
    return false;
  }
  qDebug() << "modelEnd.rowCount() = " << modelEnd.rowCount();

  return true;
}

void CadastroFornecedor::clearFields() {
  RegisterDialog::clearFields();
  novoEnd();
  setupUi();
}

void CadastroFornecedor::setupMapper() {
  addMapping(ui->lineEditFornecedor, "razaoSocial");
  addMapping(ui->lineEditContatoNome, "contatoNome");
  addMapping(ui->lineEditContatoApelido, "contatoApelido");
  addMapping(ui->lineEditContatoRG, "contatoRG");
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

  mapperEnd.addMapping(ui->comboBoxTipoEnd, modelEnd.fieldIndex("descricao"));
  mapperEnd.addMapping(ui->lineEditCEP, modelEnd.fieldIndex("CEP"));
  mapperEnd.addMapping(ui->lineEditEndereco, modelEnd.fieldIndex("logradouro"));
  mapperEnd.addMapping(ui->lineEditNro, modelEnd.fieldIndex("numero"));
  mapperEnd.addMapping(ui->lineEditComp, modelEnd.fieldIndex("complemento"));
  mapperEnd.addMapping(ui->lineEditBairro, modelEnd.fieldIndex("bairro"));
  mapperEnd.addMapping(ui->lineEditCidade, modelEnd.fieldIndex("cidade"));
  mapperEnd.addMapping(ui->lineEditUF, modelEnd.fieldIndex("uf"));
}

void CadastroFornecedor::registerMode() {
  ui->pushButtonCadastrar->show();
  ui->pushButtonAtualizar->hide();
  //  ui->pushButtonNovoCad->show();
  ui->pushButtonRemover->hide();
}

void CadastroFornecedor::updateMode() {
  ui->pushButtonCadastrar->hide();
  ui->pushButtonAtualizar->show();
  //  ui->pushButtonNovoCad->show();
  ui->pushButtonRemover->show();
}

void CadastroFornecedor::validaCNPJ(QString text) {
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

    if (digito1 == text.at(12).digitValue() and digito2 == text.at(13).digitValue()) {
      qDebug() << "Válido!";
    } else {
      QMessageBox::warning(this, "Aviso!", "CNPJ inválido!");
      return;
    }
  }
}

void CadastroFornecedor::validaCPF(QString text) {
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
    //    qDebug() << "sub: " << sub;

    QVector<int> sub2;
    for (int i = 0; i < sub.size(); ++i) {
      sub2.push_back(sub.at(i).digitValue());
    }
    //    qDebug() << "sub2: " << sub2;

    QVector<int> multiplicadores = {10, 9, 8, 7, 6, 5, 4, 3, 2};

    int soma = 0;
    for (int i = 0; i < 9; ++i) {
      soma += sub2.at(i) * multiplicadores.at(i);
      //      qDebug() << "sub[i]: " << sub2.at(i);
    }
    //    qDebug() << "soma: " << soma;

    int resto = soma % 11;

    if (resto < 2) {
      digito1 = 0;
    } else {
      digito1 = 11 - resto;
    }
    //    qDebug() << "digito1: " << digito1;
    sub2.push_back(digito1);

    QVector<int> multiplicadores2 = {11, 10, 9, 8, 7, 6, 5, 4, 3, 2};
    soma = 0;

    for (int i = 0; i < 10; ++i) {
      soma += sub2.at(i) * multiplicadores2.at(i);
      //      qDebug() << "sub2[i]: " << sub2.at(i);
    }

    resto = soma % 11;

    if (resto < 2) {
      digito2 = 0;
    } else {
      digito2 = 11 - resto;
    }
    //    qDebug() << "digito2: " << digito2;
    //    qDebug() << "at(9): " << teste.at(9);
    //    qDebug() << "at(10): " << teste.at(10);

    if (digito1 == text.at(9).digitValue() and digito2 == text.at(10).digitValue()) {
      //      qDebug() << "Válido!";
    } else {
      QMessageBox::warning(this, "Aviso!", "CPF inválido!");
      return;
    }
  }
}

bool CadastroFornecedor::verifyRequiredField(QLineEdit *line, bool silent) {
  if (line->styleSheet() != requiredStyle()) {
    return true;
  }
  //  if (not line->isEnabled()) {
  //    return true;
  //  }
  if (not line->isVisible()) {
    return true;
  }
  //  if (line->parent()->isWindowType() and line->parent()->objectName() != objectName()) {
  //    return true;
  //  }
  if ((line->text().isEmpty()) or line->text() == "0,00" or line->text() == "../-" or
      (line->text().size() < (line->inputMask().remove(";").remove(">").remove("_").size()) or
       (line->text().size() < line->placeholderText().size() - 1))) {
    qDebug() << "ObjectName: " << line->parent()->objectName() << ", line: " << line->objectName() << " | "
             << line->text();
    if (not silent) {
      QMessageBox::warning(dynamic_cast<QWidget *>(this), "Atenção!",
                           "Você não preencheu um campo obrigatório!", QMessageBox::Ok,
                           QMessageBox::NoButton);
      line->setFocus();
    }
    return false;
  }
  return true;
}

void CadastroFornecedor::on_pushButtonCadastrar_clicked() {
  if (save()) {
    if (closeBeforeUpdate)
      accept();
  }
}

void CadastroFornecedor::on_pushButtonAtualizar_clicked() {
  if (save()) {
    if (closeBeforeUpdate)
      accept();
  }
}

void CadastroFornecedor::on_pushButtonBuscar_clicked() {
  SearchDialog *sdFornecedor = SearchDialog::fornecedor(this);
  sdFornecedor->show();
  connect(sdFornecedor, &SearchDialog::itemSelected, this, &CadastroFornecedor::changeItem);
}

void CadastroFornecedor::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroFornecedor::on_pushButtonRemover_clicked() { remove(); }

void CadastroFornecedor::on_pushButtonCancelar_clicked() { close(); }

void CadastroFornecedor::on_lineEditCNPJ_textEdited(const QString &) {
  QString text = ui->lineEditCNPJ->text().remove(".").remove("/").remove("-");
  validaCNPJ(text);
}

void CadastroFornecedor::on_lineEditContatoCPF_textEdited(const QString &) {
  QString text = ui->lineEditContatoCPF->text().remove(".").remove("-");
  validaCPF(text);
}

void CadastroFornecedor::on_pushButtonAdicionarEnd_clicked() {
  if (atualizarEnd()) { // TODO: reimplement
    qDebug() << "cadastrou endereço!";
  } else {
    qDebug() << "não cadastrou endereço :(";
  }
}

bool CadastroFornecedor::atualizarEnd() {
  if (not RegisterDialog::verifyFields({ui->lineEditCEP, ui->lineEditEndereco, ui->lineEditNro,
                                       ui->lineEditBairro, ui->lineEditCidade, ui->lineEditUF})) {
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
      QMessageBox::warning(this, "Atenção!", "Não foi possível cadastrar este endereço.", QMessageBox::Ok,
                           QMessageBox::NoButton);
      return false;
    }
  }
  modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("descricao")), ui->comboBoxTipoEnd->currentText());
  if (not ui->lineEditCEP->text().isEmpty())
    modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("CEP")), ui->lineEditCEP->text());
  if (not ui->lineEditEndereco->text().isEmpty())
    modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("logradouro")), ui->lineEditEndereco->text());
  if (not ui->lineEditNro->text().isEmpty())
    modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("numero")), ui->lineEditNro->text());
  if (not ui->lineEditComp->text().isEmpty())
    modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("complemento")), ui->lineEditComp->text());
  if (not ui->lineEditBairro->text().isEmpty())
    modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("bairro")), ui->lineEditBairro->text());
  if (not ui->lineEditCidade->text().isEmpty())
    modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("cidade")), ui->lineEditCidade->text());
  if (not ui->lineEditUF->text().isEmpty())
    modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("uf")), ui->lineEditUF->text());
  if (not modelEnd.data(modelEnd.index(row, modelEnd.fieldIndex("valid"))).isValid()) {
    modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex("valid")), true);
  }
  return true;
}
