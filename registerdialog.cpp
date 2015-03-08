#include "registerdialog.h"
#include <QCloseEvent>

RegisterDialog::RegisterDialog(QString table, QString primaryIdx, QWidget *parent = 0)
  : QDialog(parent), model(this), primaryKey(primaryIdx), table(nullptr) {
  setWindowModality(Qt::WindowModal);
  setWindowFlags(Qt::Window);

  model.setTable(table);
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  mapper.setModel(&model);
//  mapper.setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
  if (!model.select()) {
    qDebug() << "Failed to populate " + table;
    QMessageBox::critical(this, "ERRO!", "Algum erro ocorreu ao acessar a tabela.", QMessageBox::Ok,
                          QMessageBox::NoButton);
  }
}

bool RegisterDialog::viewRegisterById(QVariant id) {
  model.select();
  QModelIndexList idxList = model.match(model.index(0, model.fieldIndex(primaryKey)), Qt::DisplayRole, id);
  if (idxList.isEmpty()) {
    QMessageBox::warning(this, "Atenção!", "Item não encontrado.", QMessageBox::Ok, QMessageBox::NoButton);
    newRegister();
    return false;
  }
  viewRegister(idxList.first());
  return true;
}

bool RegisterDialog::viewRegister(QModelIndex idx) {
  //  qDebug() << idx.row() << " == " << mapper.currentIndex();
  if (!confirmationMessage()) {
    return false;
  }
  clearFields();
  updateMode();
  if (table) {
    table->clearSelection();
  }
  mapper.setCurrentIndex(idx.row());
  return true;
}

bool RegisterDialog::verifyFields(QList<QLineEdit *> list) {
  foreach (QLineEdit *line, list) {
    //    if (line->styleSheet() == requiredStyle()) {
    if (!verifyRequiredField(line)) {
      return false;
    }
    //    }
  }
  return true;
}

void RegisterDialog::sendUpdateMessage() {
  QString text;
  foreach (QString key, textKeys) {
    if (!key.isEmpty()) {
      QVariant val = data(key);
      if (val.isValid()) {
        if (!text.isEmpty())
          text.append(" - ");
        text.append(val.toString());
      }
    }
  }
  emit registerUpdated(data(primaryKey), text);
}

void RegisterDialog::closeEvent(QCloseEvent *event) {
  if (!confirmationMessage()) {
    event->ignore();
  } else {
    event->accept();
    QDialog::closeEvent(event);
    close();
  }
}

void RegisterDialog::keyPressEvent(QKeyEvent * event) {
  if(event->key() == Qt::Key_Escape) {
    event->accept();
    close();
  } else {
    QDialog::keyPressEvent(event);
  }
}

QStringList RegisterDialog::getTextKeys() const {
  return textKeys;
}

void RegisterDialog::setTextKeys(const QStringList &value) {
  textKeys = value;
}

void RegisterDialog::changeItem(QVariant value, QString text) {
  //  qDebug() << objectName() << " : changeItem : " << __LINE__ << ", value = " << value << ", text = " <<
  //  text;
  Q_UNUSED(text)
  viewRegisterById(value);
}

bool RegisterDialog::verifyRequiredField(QLineEdit *line) {
  //  if (line->styleSheet() != requiredStyle()) {
  //    return true;
  //  }
  //  if(line->parent()->isWindowType() && line->parent()->objectName() != objectName() ) {
  //    return true;
  //  }
  if ((line->text().isEmpty()) || line->text() == "0,00" || line->text() == "../-" ||
      (line->text().size() < (line->inputMask().remove(";").remove(">").remove("_").size()) ||
       (line->text().size() < line->placeholderText().size() - 1))) {
    qDebug() << "ObjectName: " << line->parent()->objectName() << ", line: " << line->objectName() << " | "
             << line->text();
    QMessageBox::warning(dynamic_cast<QWidget *>(this), "Atenção!",
                         "Você não preencheu um campo obrigatório!", QMessageBox::Ok, QMessageBox::NoButton);
    line->setFocus();
    return false;
  }
  return true;
}

bool RegisterDialog::confirmationMessage() {
  if(model.isDirty()) {
    QMessageBox msgBox(QMessageBox::Warning, "Atenção!", "Deseja aplicar as alterações?",
                       QMessageBox::Yes | QMessageBox::No);
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setButtonText(QMessageBox::Yes, "Sim");
    msgBox.setButtonText(QMessageBox::No, "Não");
    if (msgBox.exec() == QMessageBox::Yes) {
      if (!save()) {
        return false;
      }
    }
    return true;
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
  if (!confirmationMessage()) {
    return false;
  }
  if (table) {
    table->clearSelection();
  }
  registerMode();
  model.select();
  int row = model.rowCount();
  model.insertRow(row);
  mapper.toLast();
  clearFields();

//  QSqlQuery maxId("SELECT MAX(" + primaryKey + ") FROM " + model.tableName());
//  maxId.first();
//  id = maxId.value(primaryKey) + 1;
//  setData(row,primaryKey,id);
  return true;
}

bool RegisterDialog::save() {
  QSqlQuery("SET SESSION ISOLATION LEVEL SETRIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();
  qDebug() << "CURRENT INDEX: " << mapper.currentIndex();
  int row = mapper.currentIndex();
  QVariant id = data(row,primaryKey);
  if (!verifyFields(row)) {
    QSqlQuery("ROLLBACK").exec();
    return false;
  }
  if (!savingProcedures(row)) {
    errorMessage();
    QSqlQuery("ROLLBACK").exec();
    return false;
  }
  if (!model.submitAll()) {
    qDebug() << objectName() << " : " << model.lastError();
    errorMessage();
    QSqlQuery("ROLLBACK").exec();
    return false;
  }
  QSqlQuery("COMMIT").exec();
  successMessage();
  viewRegister(model.index(row,0));
  sendUpdateMessage();
  return true;
}

void RegisterDialog::clearFields() {
  foreach (QLineEdit *line, this->findChildren<QLineEdit *>()) {
    line->clear();
  }
}

void RegisterDialog::remove() {
  QMessageBox msgBox(QMessageBox::Warning, "Atenção!", "Tem certeza que deseja remover?",
                     QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Sim");
  msgBox.setButtonText(QMessageBox::No, "Não");
  if (msgBox.exec() == QMessageBox::Yes) {
    //    qDebug() << "Yes!";
    if (model.removeRow(mapper.currentIndex()) && model.submitAll()) {
      //      qDebug() << "REMOVING " << mapper.currentIndex();
      model.select();
      newRegister();
    } else {
      QMessageBox::warning(this, "Atenção!", "Não foi possível remover este item.", QMessageBox::Ok,
                           QMessageBox::NoButton);
      qDebug() << model.lastError();
    }
  }
}

void RegisterDialog::setTable(QAbstractItemView *table) {
  this->table = table;
}
