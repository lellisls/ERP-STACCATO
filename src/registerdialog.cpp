#include <QCloseEvent>
#include <QDebug>
#include <QMessageBox>
#include <QShortcut>
#include <QSqlError>
#include <QSqlQuery>

#include "registerdialog.h"

RegisterDialog::RegisterDialog(const QString &table, const QString &primaryKey, QWidget *parent = nullptr) : QDialog(parent), primaryKey(primaryKey), model(this) {
  setWindowModality(Qt::NonModal);
  setWindowFlags(Qt::Window);

  model.setTable(table);
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);
  model.setFilter("0");

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());

  mapper.setModel(&model);
  mapper.setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

  connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this), &QShortcut::activated, this, &QWidget::close);
  connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this), &QShortcut::activated, this, &RegisterDialog::saveSlot);
}

bool RegisterDialog::viewRegisterById(const QVariant &id) {
  primaryId = id.toString();

  if (primaryId.isEmpty()) {
    QMessageBox::critical(this, "Erro", "primaryId vazio!");
    return false;
  }

  model.setFilter(primaryKey + " = '" + primaryId + "'");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro ao acessar a tabela " + model.tableName() + ": " + model.lastError().text());
    return false;
  }

  if (model.rowCount() == 0) {
    QMessageBox::critical(this, "Erro!", "Item não encontrado.");
    close();
    return false;
  }

  return viewRegister();
}

bool RegisterDialog::viewRegister() {
  isUpdate = true;

  if (not confirmationMessage()) return false;

  clearFields();
  updateMode();

  if (not primaryId.isEmpty()) {
    model.setFilter(primaryKey + " = '" + primaryId + "'");

    if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
  }

  mapper.setCurrentIndex(0);

  return true;
}

bool RegisterDialog::verifyFields(const QList<QLineEdit *> &list) {
  for (auto const &line : list) {
    if (not verifyRequiredField(line)) return false;
  }

  return true;
}

bool RegisterDialog::setData(const QString &key, const QVariant &value) {
  if (value.isNull() or (value.type() == QVariant::String and value.toString().isEmpty())) return true;
  if (value.type() == QVariant::String and value.toString().remove(".").remove("/").remove("-").isEmpty()) return true;
  if (value.type() == QVariant::Date and value.toString() == "1900-01-01") return true;

  int row = currentRow != -1 ? currentRow : mapper.currentIndex();

  return model.setData(row, key, value);
}

QVariant RegisterDialog::data(const QString &key) { return model.data(mapper.currentIndex(), key); }

QVariant RegisterDialog::data(const int row, const QString &key) { return model.data(row, key); }

void RegisterDialog::addMapping(QWidget *widget, const QString &key, const QByteArray &propertyName) {
  if (model.fieldIndex(key) == -1) {
    QMessageBox::critical(this, "Erro!", "Chave " + key + " não encontrada na tabela " + model.tableName());
    return;
  }

  propertyName.isNull() ? mapper.addMapping(widget, model.fieldIndex(key)) : mapper.addMapping(widget, model.fieldIndex(key), propertyName);
}

QString RegisterDialog::requiredStyle() { return (QString("background-color: rgb(255, 255, 127)")); }

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

void RegisterDialog::show() {
  QWidget::show();
  adjustSize();
}

bool RegisterDialog::verifyRequiredField(QLineEdit *line, const bool silent) {
  if (not line->styleSheet().contains(requiredStyle())) return true;
  if (not line->isVisible()) return true;

  if ((line->text().isEmpty()) or (line->text() == "0,00") or (line->text() == "../-") or (line->text().size() < line->inputMask().remove(";").remove(">").remove("_").size()) or
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
  // when the user press a 'add' or 'update' button to insert a address or item set a bool isDirty to true, when the
  // user register or update the base model set the bool to false (verify only if there are unsaved rows but not if they
  // are edited)
  // and connect widgets edited signals too, just dont use model.isDirty

  // NOTE: implement the better way
  // if isDirty

  //  if (not isDirty) return true;

  //  QMessageBox msgBox;
  //  msgBox.setParent(this);
  //  msgBox.setLocale(QLocale::Portuguese);
  //  msgBox.setText("<strong>O cadastro foi alterado!</strong>");
  //  msgBox.setInformativeText("Se não tinha intenção de fechar, clique em cancelar.");
  //  msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
  //  msgBox.setWindowModality(Qt::WindowModal);
  //  msgBox.setButtonText(QMessageBox::Save, "Salvar");
  //  msgBox.setButtonText(QMessageBox::Discard, "Fechar sem salvar");
  //  msgBox.setButtonText(QMessageBox::Cancel, "Cancelar");
  //  msgBox.setDefaultButton(QMessageBox::Save);

  //  const int escolha = msgBox.exec();

  //  if (escolha == QMessageBox::Save) return save();
  //  if (escolha == QMessageBox::Discard) return true;
  //  if (escolha == QMessageBox::Cancel) return false;

  return true;
}

void RegisterDialog::errorMessage() { QMessageBox::critical(this, "Erro!", "Não foi possível cadastrar este item."); }

bool RegisterDialog::newRegister() {
  if (not confirmationMessage()) return false;

  model.setFilter("0");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
    return false;
  }

  clearFields();
  registerMode();

  return true;
}

void RegisterDialog::clearFields() {
  for (auto const &line : findChildren<QLineEdit *>()) line->clear();
}

void RegisterDialog::remove() {
  QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Tem certeza que deseja remover?", QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Remover");
  msgBox.setButtonText(QMessageBox::No, "Voltar");

  if (msgBox.exec() == QMessageBox::Yes) {
    setData("desativado", true);

    if (not model.submitAll()) {
      QMessageBox::critical(this, "Erro!", "Não foi possível remover este item: " + model.lastError().text());
      return;
    }

    newRegister();
  }
}

bool RegisterDialog::validaCNPJ(const QString &text) {
  if (text.size() != 14) return false;

  const QString sub = text.left(12);

  QVector<int> sub2;

  for (const auto i : sub) sub2.push_back(i.digitValue());

  const QVector<int> multiplicadores = {5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2};

  int soma = 0;

  for (int i = 0; i < 12; ++i) soma += sub2.at(i) * multiplicadores.at(i);

  int resto = soma % 11;

  const int digito1 = resto < 2 ? 0 : 11 - resto;

  sub2.push_back(digito1);

  const QVector<int> multiplicadores2 = {6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2};
  soma = 0;

  for (int i = 0; i < 13; ++i) soma += sub2.at(i) * multiplicadores2.at(i);

  resto = soma % 11;

  const int digito2 = resto < 2 ? 0 : 11 - resto;

  if (digito1 != text.at(12).digitValue() or digito2 != text.at(13).digitValue()) {
    QMessageBox::critical(this, "Erro!", "CNPJ inválido!");
    return false;
  }

  return true;
}

bool RegisterDialog::validaCPF(const QString &text) {
  if (text.size() != 11) return false;

  if (text == "00000000000" or text == "11111111111" or text == "22222222222" or text == "33333333333" or text == "44444444444" or text == "55555555555" or text == "66666666666" or
      text == "77777777777" or text == "88888888888" or text == "99999999999") {
    QMessageBox::critical(this, "Erro!", "CPF inválido!");
    return false;
  }

  const QString sub = text.left(9);

  QVector<int> sub2;

  for (const auto i : sub) sub2.push_back(i.digitValue());

  const QVector<int> multiplicadores = {10, 9, 8, 7, 6, 5, 4, 3, 2};

  int soma = 0;

  for (int i = 0; i < 9; ++i) soma += sub2.at(i) * multiplicadores.at(i);

  int resto = soma % 11;

  const int digito1 = resto < 2 ? 0 : 11 - resto;

  sub2.push_back(digito1);

  const QVector<int> multiplicadores2 = {11, 10, 9, 8, 7, 6, 5, 4, 3, 2};
  soma = 0;

  for (int i = 0; i < 10; ++i) soma += sub2.at(i) * multiplicadores2.at(i);

  resto = soma % 11;

  const int digito2 = resto < 2 ? 0 : 11 - resto;

  if (digito1 != text.at(9).digitValue() or digito2 != text.at(10).digitValue()) {
    QMessageBox::critical(this, "Erro!", "CPF inválido!");
    return false;
  }

  return true;
}

void RegisterDialog::marcarDirty() { isDirty = true; }

QVariant RegisterDialog::getLastInsertId() {
  QSqlQuery query;

  if (not query.exec("SELECT LAST_INSERT_ID() AS id") or not query.first()) {
    QMessageBox::critical(nullptr, "Erro!", "Erro buscando último id: " + query.lastError().text());
    return QVariant();
  }

  return query.value("id");
}
