#include <QCloseEvent>
#include <QShortcut>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>

#include "registerdialog.h"

RegisterDialog::RegisterDialog(QString table, QString primaryKey, QWidget *parent = 0)
  : QDialog(parent), model(this), primaryKey(primaryKey) {
  setWindowModality(Qt::NonModal);
  setWindowFlags(Qt::Window);

  model.setTable(table);
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro ao acessar a tabela " + table + ": " + model.lastError().text());
    return;
  }

  mapper.setModel(&model);
  mapper.setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

  connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this), &QShortcut::activated, this, &QWidget::close);
  connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this), &QShortcut::activated, this,
          &RegisterDialog::saveSlot);
}

bool RegisterDialog::viewRegisterById(const QVariant id) {
  if (not model.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro ao acessar a tabela " + model.tableName() + ": " + model.lastError().text());
    return false;
  }

  const QModelIndexList indexList = model.match(model.index(0, model.fieldIndex(primaryKey)), Qt::DisplayRole, id, 1,
                                                Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap));

  if (indexList.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Item não encontrado.");
    close();
    return false;
  }

  viewRegister(indexList.first());

  return true;
}

bool RegisterDialog::viewRegister(const QModelIndex index) {
  if (not confirmationMessage()) {
    return false;
  }

  clearFields();
  updateMode();

  mapper.setCurrentIndex(index.row());

  return true;
}

bool RegisterDialog::verifyFields(const QList<QLineEdit *> list) {
  for (auto *line : list) {
    if (not verifyRequiredField(line)) {
      return false;
    }
  }

  return true;
}

void RegisterDialog::setData(const QString &key, QVariant value) {
  if (value.isNull() or (value.type() == QVariant::String and value.toString().isEmpty())) {
    return;
  }

  if (value.type() == QVariant::String and value.toString().remove(".").remove("/").remove("-").isEmpty()) {
    return;
  }

  if (value.type() == QVariant::String and value.toString() == "1900-01-01") {
    return;
  }

  isOk = model.setData(row, key, value);
}

QVariant RegisterDialog::data(const QString &key) { return model.data(mapper.currentIndex(), key); }

QVariant RegisterDialog::data(int row, const QString &key) { return model.data(row, key); }

void RegisterDialog::addMapping(QWidget *widget, const QString &key) {
  if (model.fieldIndex(key) == -1) {
    QMessageBox::critical(this, "Erro!", "Chave " + key + " não encontrada na tabela " + model.tableName());
    return;
  }

  mapper.addMapping(widget, model.fieldIndex(key));
}

void RegisterDialog::addMapping(QWidget *widget, const QString &key, const QByteArray &propertyName) {
  if (model.fieldIndex(key) == -1) {
    QMessageBox::critical(this, "Erro!", "Chave " + key + " não encontrada na tabela " + model.tableName());
    return;
  }

  mapper.addMapping(widget, model.fieldIndex(key), propertyName);
}

void RegisterDialog::sendUpdateMessage() {
  QString text;

  for (const auto key : textKeys) {
    if (key.isEmpty() or not data(key).isValid()) {
      continue;
    }

    text += (text.isEmpty() ? "" : " - ") + data(key).toString();
  }

  emit registerUpdated(data(primaryKey), text);
}

QString RegisterDialog::requiredStyle() { return (QString("background-color: rgb(255, 255, 127);")); }

void RegisterDialog::closeEvent(QCloseEvent *event) { confirmationMessage() ? event->accept() : event->ignore(); }

void RegisterDialog::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Escape) {
    event->accept();
    close();
    return;
  }

  QDialog::keyPressEvent(event);
}

QStringList RegisterDialog::getTextKeys() const { return textKeys; }

void RegisterDialog::setTextKeys(const QStringList &value) { textKeys = value; }

void RegisterDialog::saveSlot() { save(); }

bool RegisterDialog::verifyRequiredField(QLineEdit *line, const bool silent) {
  if (line->styleSheet() != requiredStyle()) {
    return true;
  }

  if (not line->isVisible()) {
    return true;
  }

  if ((line->text().isEmpty()) or (line->text() == "0,00") or (line->text() == "../-") or
      (line->text().size() < line->inputMask().remove(";").remove(">").remove("_").size()) or
      (line->text().size() < line->placeholderText().size() - 1)) {
    if (not silent) {
      QMessageBox::critical(this, "Erro!", "Você não preencheu um campo obrigatório: " + line->accessibleName());
      line->setFocus();
    }

    return false;
  }

  return true;
}

bool RegisterDialog::confirmationMessage() {
  if (not model.isDirty() and not isDirty) {
    return true;
  }

  QMessageBox msgBox;
  msgBox.setParent(this);
  msgBox.setLocale(QLocale::Portuguese);
  msgBox.setText("<strong>O cadastro foi alterado!</strong>");
  msgBox.setInformativeText("Se não tinha intenção de fechar, clique em cancelar.");
  msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
  msgBox.setWindowModality(Qt::WindowModal);
  msgBox.setButtonText(QMessageBox::Save, "Salvar");
  msgBox.setButtonText(QMessageBox::Discard, "Fechar sem salvar");
  msgBox.setButtonText(QMessageBox::Cancel, "Cancelar");
  msgBox.setDefaultButton(QMessageBox::Save);

  const int ret = msgBox.exec();

  return ret == QMessageBox::Save ? save() : ret == QMessageBox::Discard ? true : false;
}

void RegisterDialog::errorMessage() { QMessageBox::critical(this, "Erro!", "Não foi possível cadastrar este item."); }

void RegisterDialog::successMessage() {
  QMessageBox::information(this, "Atenção!", "Cadastro atualizado com sucesso!");
}

bool RegisterDialog::newRegister() {
  if (not confirmationMessage()) {
    return false;
  }

  clearFields();
  registerMode();

  return model.select();
}

bool RegisterDialog::save(const bool isUpdate) {
  if (not verifyFields()) {
    return false;
  }

  QSqlQuery("SET SESSION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  row = (isUpdate) ? mapper.currentIndex() : model.rowCount();

  if (row == -1) {
    QMessageBox::critical(this, "Erro!", "Linha errada!");
    QSqlQuery("ROLLBACK").exec();
    return false;
  }

  if (not isUpdate) {
    model.insertRow(row);
  }

  if (not savingProcedures()) {
    errorMessage();
    QSqlQuery("ROLLBACK").exec();
    return false;
  }

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro salvando dados na tabela " + model.tableName() + ": " + model.lastError().text());
    errorMessage();
    QSqlQuery("ROLLBACK").exec();
    return false;
  }

  QSqlQuery("COMMIT").exec();
  isDirty = false;
  isOk = true;

  viewRegister(model.index(row, 0));
  sendUpdateMessage();

  if (not silent) {
    successMessage();
  }

  return true;
}

bool RegisterDialog::update() { return save(true); }

void RegisterDialog::clearFields() {
  for (auto *line : this->findChildren<QLineEdit *>()) {
    line->clear();
  }
}

void RegisterDialog::remove() {
  QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Tem certeza que deseja remover?",
                     QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Sim");
  msgBox.setButtonText(QMessageBox::No, "Não");

  if (msgBox.exec() == QMessageBox::Yes) {
    setData("desativado", true);

    if (not model.submitAll()) {
      QMessageBox::critical(this, "Erro!", "Não foi possível remover este item: " + model.lastError().text());
      return;
    }

    newRegister();
  }
}

bool RegisterDialog::validaCNPJ(const QString text) {
  if (text.size() != 14) {
    return false;
  }

  const QString sub = text.left(12);

  QVector<int> sub2;

  for (int i = 0, size = sub.size(); i < size; ++i) {
    sub2.push_back(sub.at(i).digitValue());
  }

  const QVector<int> multiplicadores = {5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2};

  int soma = 0;

  for (int i = 0; i < 12; ++i) {
    soma += sub2.at(i) * multiplicadores.at(i);
  }

  int resto = soma % 11;

  int digito1 = resto < 2 ? 0 : 11 - resto;

  sub2.push_back(digito1);

  const QVector<int> multiplicadores2 = {6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2};
  soma = 0;

  for (int i = 0; i < 13; ++i) {
    soma += sub2.at(i) * multiplicadores2.at(i);
  }

  resto = soma % 11;

  int digito2 = resto < 2 ? 0 : 11 - resto;

  if (digito1 != text.at(12).digitValue() or digito2 != text.at(13).digitValue()) {
    QMessageBox::critical(this, "Erro!", "CNPJ inválido!");
    return false;
  }

  return true;
}

bool RegisterDialog::validaCPF(const QString text) {
  if (text.size() != 11) {
    return false;
  }

  if (text == "00000000000" or text == "11111111111" or text == "22222222222" or text == "33333333333" or
      text == "44444444444" or text == "55555555555" or text == "66666666666" or text == "77777777777" or
      text == "88888888888" or text == "99999999999") {
    QMessageBox::warning(this, "Aviso!", "CPF inválido!");
    return false;
  }

  const QString sub = text.left(9);

  QVector<int> sub2;

  for (int i = 0, size = sub.size(); i < size; ++i) {
    sub2.push_back(sub.at(i).digitValue());
  }

  const QVector<int> multiplicadores = {10, 9, 8, 7, 6, 5, 4, 3, 2};

  int soma = 0;

  for (int i = 0; i < 9; ++i) {
    soma += sub2.at(i) * multiplicadores.at(i);
  }

  int resto = soma % 11;

  const int digito1 = resto < 2 ? 0 : 11 - resto;

  sub2.push_back(digito1);

  const QVector<int> multiplicadores2 = {11, 10, 9, 8, 7, 6, 5, 4, 3, 2};
  soma = 0;

  for (int i = 0; i < 10; ++i) {
    soma += sub2.at(i) * multiplicadores2.at(i);
  }

  resto = soma % 11;

  const int digito2 = resto < 2 ? 0 : 11 - resto;

  if (digito1 != text.at(9).digitValue() or digito2 != text.at(10).digitValue()) {
    QMessageBox::critical(this, "Erro!", "CPF inválido!");
    return false;
  }

  return true;
}

void RegisterDialog::marcarDirty() { isDirty = true; }

// TODO: alguma forma de operar com cadastros desativados
