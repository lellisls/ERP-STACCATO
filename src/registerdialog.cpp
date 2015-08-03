#include <QCloseEvent>
#include <QShortcut>
#include <QSqlDriver>
#include <QSqlRecord>
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
    qDebug() << objectName() << "Failed to populate " + table + ": " << model.lastError();
    QMessageBox::critical(this, "Erro!", "Algum erro ocorreu ao acessar a tabela.", QMessageBox::Ok,
                          QMessageBox::NoButton);
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
    qDebug() << "erro model: " << model.lastError();
    return false;
  }

  const QModelIndexList indexList = model.match(model.index(0, model.fieldIndex(primaryKey)), Qt::DisplayRole, id);

  if (indexList.isEmpty()) {
    QMessageBox::warning(this, "Atenção!", "Item não encontrado.", QMessageBox::Ok, QMessageBox::NoButton);
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
  foreach (QLineEdit *line, list) {
    if (not verifyRequiredField(line)) {
      return false;
    }
  }

  return true;
}

void RegisterDialog::setData(const QString &key, QVariant value) {
  if (model.fieldIndex(key) == -1) {
    qDebug() << objectName() << " : Key '" << key << "' not found on table '" << model.tableName() << "'";
  }
  if (not model.setData(model.index(mapper.currentIndex(), model.fieldIndex(key)), value)) {
    qDebug() << key << " error - row: " << mapper.currentIndex() << " - value: " << value;
  }
}

bool RegisterDialog::setData(int row, const QString &key, QVariant value) {
  if (row == -1) {
    qDebug() << "Something wrong on the row!";
    return false;
  }

  if (model.fieldIndex(key) == -1) {
    qDebug() << objectName() << " : Key '" << key << "' not found on table '" << model.tableName() << "'";
    return false;
  }

  if (not value.isNull()) {
    if (not model.setData(model.index(row, model.fieldIndex(key)), value)) {
      qDebug() << key << " error - row: " << row << " - value: " << value;
      return false;
    }
  }

  return true;
}

QVariant RegisterDialog::data(const QString &key) {
  if (model.fieldIndex(key) == -1) {
    qDebug() << objectName() << " : Key " << key << " not found on model!";
  }

  return (model.data(model.index(mapper.currentIndex(), model.fieldIndex(key))));
}

QVariant RegisterDialog::data(int row, const QString &key) {
  if (model.fieldIndex(key) == -1) {
    qDebug() << objectName() << " : Key " << key << " not found on model!";
  }
  return (model.data(model.index(row, model.fieldIndex(key))));
}

void RegisterDialog::addMapping(QWidget *widget, const QString &key) {
  if (model.fieldIndex(key) == -1) {
    qDebug() << objectName() << " : Key " << key << " not found on model!";
  }
  mapper.addMapping(widget, model.fieldIndex(key));
}

void RegisterDialog::addMapping(QWidget *widget, const QString &key, const QByteArray &propertyName) {
  if (model.fieldIndex(key) == -1) {
    qDebug() << objectName() << " : Key " << key << " not found on model!";
  }
  mapper.addMapping(widget, model.fieldIndex(key), propertyName);
}

void RegisterDialog::sendUpdateMessage() {
  QString text;

  foreach (QString key, textKeys) {
    if (not key.isEmpty()) {
      const QVariant val = data(key);

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

QString RegisterDialog::requiredStyle() { return (QString("background-color: rgb(255, 255, 127);")); }

void RegisterDialog::closeEvent(QCloseEvent *event) {
  if (confirmationMessage()) {
    event->accept();
  } else {
    event->ignore();
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
  if (model.isDirty() or isDirty) {
    //    qDebug() << "model is dirty: " << model.isDirty() << "(" << model.tableName() << ")";
    //    qDebug() << "isDirty: " << isDirty;

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

  registerMode();

  if (not model.select()) {
    qDebug() << "erro model: " << model.lastError();
    return false;
  }

  clearFields();

  return true;
}

bool RegisterDialog::save(const bool isUpdate) {
  if (not verifyFields()) {
    return false;
  }

  QSqlQuery("SET SESSION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  const int row = (isUpdate) ? mapper.currentIndex() : model.rowCount();

  if (row == -1) {
    qDebug() << "Something went very wrong!";
    return false;
  }

  if (not isUpdate) {
    model.insertRow(row);
  }

  if (not savingProcedures(row)) {
    errorMessage();
    QSqlQuery("ROLLBACK").exec();
    return false;
  }

  if (not model.submitAll()) {
    qDebug() << objectName() << " : " << model.lastError();
    qDebug() << "Last query: "
             << model.database().driver()->sqlStatement(QSqlDriver::InsertStatement, model.tableName(),
                                                        model.record(row), false);
    errorMessage();
    QSqlQuery("ROLLBACK").exec();
    return false;
  }

  QSqlQuery("COMMIT").exec();
  isDirty = false;

  viewRegister(model.index(row, 0));
  sendUpdateMessage();

  if (not silent) {
    successMessage();
  }

  return true;
}

bool RegisterDialog::update() { return save(true); }

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
      if (not model.select()) {
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

bool RegisterDialog::validaCNPJ(const QString text) {
  if (text.size() == 14) {
    int digito1;
    int digito2;

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

    if (resto < 2) {
      digito1 = 0;
    } else {
      digito1 = 11 - resto;
    }

    sub2.push_back(digito1);

    const QVector<int> multiplicadores2 = {6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2};
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
      return false;
    }
  }

  return true;
}

bool RegisterDialog::validaCPF(const QString text) {
  if (text.size() == 11) {
    if (text == "00000000000" or text == "11111111111" or text == "22222222222" or text == "33333333333" or
        text == "44444444444" or text == "55555555555" or text == "66666666666" or text == "77777777777" or
        text == "88888888888" or text == "99999999999") {
      QMessageBox::warning(this, "Aviso!", "CPF inválido!");
      return false;
    }

    int digito1;
    int digito2;

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

    if (resto < 2) {
      digito1 = 0;
    } else {
      digito1 = 11 - resto;
    }

    sub2.push_back(digito1);

    const QVector<int> multiplicadores2 = {11, 10, 9, 8, 7, 6, 5, 4, 3, 2};
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
      return false;
    }
  }

  return true;
}

void RegisterDialog::marcarDirty() { isDirty = true; }
