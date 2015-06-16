#include <QCloseEvent>
#include <QShortcut>

#include "registerdialog.h"

RegisterDialog::RegisterDialog(QString table, QString primaryKey, QWidget *parent = 0)
  : QDialog(parent), model(this), primaryKey(primaryKey), table(nullptr) {
  setWindowModality(Qt::NonModal);
  setWindowFlags(Qt::Window);

  model.setTable(table);
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not model.select()) {
    qDebug() << objectName() << "Failed to populate " + table + ": " << model.lastError();
    QMessageBox::critical(this, "ERRO!", "Algum erro ocorreu ao acessar a tabela.", QMessageBox::Ok,
                          QMessageBox::NoButton);
    return;
  }

  mapper.setModel(&model);

  QShortcut *shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this);
  connect(shortcut, &QShortcut::activated, this, &QWidget::close);
  shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this);
  connect(shortcut, &QShortcut::activated, this, &RegisterDialog::saveSlot);
}

bool RegisterDialog::viewRegisterById(QVariant id) {
  if (not model.select()){
    qDebug() << "erro model: " << model.lastError();
    return false;
  }

  QModelIndexList indexList = model.match(model.index(0, model.fieldIndex(primaryKey)), Qt::DisplayRole, id);

  if (indexList.isEmpty()) {
    QMessageBox::warning(this, "Atenção!", "Item não encontrado.", QMessageBox::Ok, QMessageBox::NoButton);
    close();
    return false;
  }

  viewRegister(indexList.first());

  return true;
}

bool RegisterDialog::viewRegister(QModelIndex index) {
  if (not confirmationMessage()) {
    return false;
  }

  clearFields();
  updateMode();

  if (table) {
    table->clearSelection();
  }

  mapper.setCurrentIndex(index.row());

  return true;
}

bool RegisterDialog::verifyFields(QList<QLineEdit *> list) {
  foreach (QLineEdit *line, list) {
    if (not verifyRequiredField(line)) {
      return false;
    }
  }

  return true;
}

void RegisterDialog::sendUpdateMessage() {
  QString text;

  foreach (QString key, textKeys) {
    if (not key.isEmpty()) {
      QVariant val = data(key);

      if (val.isValid()) {
        if (not text.isEmpty()) {
          text.append(" - ");
        }

        text.append(val.toString());
      }
    }
  }

  emit registerUpdated(data(primaryKey), text);
}

void RegisterDialog::closeEvent(QCloseEvent *event) {
  if (not confirmationMessage()) {
    event->ignore();
  } else {
    event->accept();
    QDialog::closeEvent(event);
    close();
  }
}

void RegisterDialog::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Escape) {
    event->accept();
    close();
  } else {
    QDialog::keyPressEvent(event);
  }
}

QStringList RegisterDialog::getTextKeys() const { return textKeys; }

void RegisterDialog::setTextKeys(const QStringList &value) { textKeys = value; }

void RegisterDialog::changeItem(QVariant value) {
  viewRegisterById(value);
}

void RegisterDialog::saveSlot() { save(); }

bool RegisterDialog::verifyRequiredField(QLineEdit *line) {
  if ((line->text().isEmpty()) or line->text() == "0,00" or line->text() == "../-" or
      (line->text().size() < (line->inputMask().remove(";").remove(">").remove("_").size()) or
       (line->text().size() < line->placeholderText().size() - 1))) {
    qDebug() << "ObjectName: " << line->parent()->objectName() << ", line: " << line->objectName() << " | "
             << line->text();
    QMessageBox::warning(this, "Atenção!", "Você não preencheu um campo obrigatório!", QMessageBox::Ok,
                         QMessageBox::NoButton);
    line->setFocus();
    return false;
  }

  return true;
}

bool RegisterDialog::confirmationMessage() {
  if (model.isDirty()) {
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

    int ret = msgBox.exec();

    if (ret == QMessageBox::Save) {
      if (not save()) {
        qDebug() << objectName() << " : "
                 << " save failed!";
        return false;
      }
    } else if (ret == QMessageBox::Cancel) {
      return false;
    }
  }

  return true;
}

void RegisterDialog::errorMessage() {
  QMessageBox::warning(this, "Atenção!", "Não foi possível cadastrar este item.", QMessageBox::Ok,
                       QMessageBox::NoButton);
}

void RegisterDialog::successMessage() {
  QMessageBox::information(this, "Atenção!", "Cadastro atualizado com sucesso!", QMessageBox::Ok,
                           QMessageBox::NoButton);
}

bool RegisterDialog::newRegister() {
  if (not confirmationMessage()) {
    return false;
  }

  if (table) {
    table->clearSelection();
  }

  registerMode();

  if (not model.select()){
    qDebug() << "erro model: " << model.lastError();
    return false;
  }

  int row = model.rowCount();
  model.insertRow(row);
  mapper.toLast();
  clearFields();

  return true;
}

bool RegisterDialog::save(bool silent) {
  QSqlQuery("SET SESSION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  int row = mapper.currentIndex();

  if (not verifyFields(row)) {
    QSqlQuery("ROLLBACK").exec();
    return false;
  }

  if (not savingProcedures(row)) {
    errorMessage();
    QSqlQuery("ROLLBACK").exec();
    return false;
  }

  if (not model.submitAll()) {
    qDebug() << objectName() << " : " << model.lastError();
    qDebug() << "qry: " << model.query().lastQuery();
    errorMessage();
    QSqlQuery("ROLLBACK").exec();
    return false;
  }

  QSqlQuery("COMMIT").exec();

  if (not silent) {
    successMessage();
  }

  viewRegister(model.index(row, 0));
  sendUpdateMessage();

  return true;
}

void RegisterDialog::clearFields() {
  foreach (QLineEdit *line, this->findChildren<QLineEdit *>()) { line->clear(); }
}

void RegisterDialog::remove() {
  QMessageBox msgBox(QMessageBox::Warning, "Atenção!", "Tem certeza que deseja remover?",
                     QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Sim");
  msgBox.setButtonText(QMessageBox::No, "Não");

  if (msgBox.exec() == QMessageBox::Yes) {
    model.setData(model.index(mapper.currentIndex(), model.fieldIndex("desativado")), 1);

    if (model.submitAll()) {
      if (not model.select()){
        qDebug() << "erro model: " << model.lastError();
        return;
      }

      newRegister();
    } else {
      QMessageBox::warning(this, "Atenção!", "Não foi possível remover este item.", QMessageBox::Ok,
                           QMessageBox::NoButton);
      qDebug() << "model error: " << model.lastError();
    }
  }
}

void RegisterDialog::setTable(QAbstractItemView *table) { this->table = table; }

void RegisterDialog::validaCNPJ(QString text){
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

void RegisterDialog::validaCPF(QString text){
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

    QVector<int> sub2;

    for (int i = 0; i < sub.size(); ++i) {
      sub2.push_back(sub.at(i).digitValue());
    }

    QVector<int> multiplicadores = {10, 9, 8, 7, 6, 5, 4, 3, 2};

    int soma = 0;

    for (int i = 0; i < 9; ++i) {
      soma += sub2.at(i) * multiplicadores.at(i);
    }

    int resto = soma % 11;

    if (resto < 2) {
      digito1 = 0;
    } else {
      digito1 = 11 - resto;
    }

    sub2.push_back(digito1);

    QVector<int> multiplicadores2 = {11, 10, 9, 8, 7, 6, 5, 4, 3, 2};
    soma = 0;

    for (int i = 0; i < 10; ++i) {
      soma += sub2.at(i) * multiplicadores2.at(i);
    }

    resto = soma % 11;

    if (resto < 2) {
      digito2 = 0;
    } else {
      digito2 = 11 - resto;
    }

    if (digito1 != text.at(9).digitValue() or digito2 != text.at(10).digitValue()) {
      QMessageBox::warning(this, "Aviso!", "CPF inválido!");
      return;
    }
  }
}
