#include "registerdialog.h"

RegisterDialog::RegisterDialog(QString table, QString primaryIdx, QWidget *parent = 0)
  : QDialog(parent), model(this), primaryKey(primaryIdx), table(nullptr) {
  setWindowModality(Qt::WindowModal);

  model.setTable(table);
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  mapper.setModel(&model);
  mapper.setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
  if (!model.select()) {
    qDebug() << "Failed to populate " + table;
    QMessageBox::critical(this, "ERRO!", "Algum erro ocorreu ao acessar a tabela!", QMessageBox::Ok,
                          QMessageBox::NoButton);
  }
  connect(this,SIGNAL(finished(int)),this,SLOT(cancel()));
}

bool RegisterDialog::viewRegisterById(QVariant id) {
  model.select();
  QModelIndexList idxList = model.match(model.index(0, model.fieldIndex(primaryKey)), Qt::DisplayRole, id);
  if (idxList.isEmpty()) {
    QMessageBox::warning(this, "Atenção!", "Item não encontrado!", QMessageBox::Ok, QMessageBox::NoButton);
    newRegister();
    return false;
  }
  viewRegister(idxList.first());
  return true;
}

bool RegisterDialog::viewRegister(QModelIndex idx) {
  qDebug() << idx.row() << " == " << mapper.currentIndex();
  if (!confirmationMessage()) {
    return false;
  }
  clearFields();
  updateMode();
  if (table)
    table->clearSelection();
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
    if(!key.isEmpty()) {
      QVariant val = data(key);
      if(val.isValid()) {
        if(!text.isEmpty())
          text.append(" - ");
        text.append(val.toString());
      }
    }
  }
  emit registerUpdated(data(primaryKey),text);
}

QStringList RegisterDialog::getTextKeys() const {
  return textKeys;
}

void RegisterDialog::setTextKeys(const QStringList & value) {
  textKeys = value;
}

void RegisterDialog::show() {
  QDialog::show();
}

void RegisterDialog::cancel() {
  if (confirmationMessage()) {
    close();
  }else{
    show();
  }
}

void RegisterDialog::accept() {
  QDialog::accept();
}

void RegisterDialog::reject() {
  QDialog::reject();
}

void RegisterDialog::changeItem(QVariant value, QString text) {
  qDebug() << objectName() << " : changeItem : " << __LINE__ << ", value = " << value << ", text = " << text;
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
  if (model.isDirty()) {
    QMessageBox msgBox(QMessageBox::Warning, "Atenção!", "Deseja aplicar as alterações?",
                       QMessageBox::Yes | QMessageBox::No);
    msgBox.setButtonText(QMessageBox::Yes, "Sim");
    msgBox.setButtonText(QMessageBox::No, "Não");

    if (msgBox.exec() == QMessageBox::Yes) {
      if (!save()) {
        return false;
      }
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
  if (!confirmationMessage()) {
    return false;
  }
  clearFields();
  registerMode();
  model.select();
  if (table) {
    qDebug() << "Clearing table selection";
    table->clearSelection();
  }
  return true;
}

bool RegisterDialog::save() {
  qDebug() << "CURRENT INDEX: " << mapper.currentIndex();
  if (!verifyFields()) {
    return false;
  }
  int row = mapper.currentIndex();
  if (row == -1) {
    row = model.rowCount();
    model.insertRow(row);
  }
  QSqlQuery("START TRANSACTION").exec();
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
  viewRegister(model.index(row, 0));
  sendUpdateMessage();
  return true;
}

void RegisterDialog::clearFields() {
  foreach (QLineEdit *line, this->findChildren<QLineEdit *>()) {
    line->clear();
  }
}

void RegisterDialog::remove() {
  QMessageBox msgBox(QMessageBox::Warning, "Atenção!", "Tem certeza que deseja remover?", QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Sim");
  msgBox.setButtonText(QMessageBox::No, "Não");
  if(msgBox.exec() == QMessageBox::Yes) {
    qDebug() << "Yes!";
    if (model.removeRow(mapper.currentIndex()) && model.submitAll()) {
      qDebug() << "REMOVING " << mapper.currentIndex();
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
