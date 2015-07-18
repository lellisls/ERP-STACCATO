#include <QSqlDriver>
#include <QSqlRecord>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>

#include "registeraddressdialog.h"

RegisterAddressDialog::RegisterAddressDialog(QString table, QString primaryKey, QWidget *parent)
  : RegisterDialog(table, primaryKey, parent) {
  setWindowModality(Qt::NonModal);
  setWindowFlags(Qt::Window);

  modelEnd.setTable(table + "_has_endereco");
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
    qDebug() << objectName() << "Failed to populate " + table + "_has_endereco: " << modelEnd.lastError();
    QMessageBox::critical(this, "Erro!", "Algum erro ocorreu ao acessar a tabela de endereço.", QMessageBox::Ok,
                          QMessageBox::NoButton);
    return;
  }

  mapperEnd.setModel(&modelEnd);
  mapperEnd.setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
}

bool RegisterAddressDialog::save(const bool isUpdate) {
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
    qDebug() << "Last query: "
             << model.database().driver()->sqlStatement(QSqlDriver::InsertStatement, model.tableName(),
                                                        model.record(row), false);
    errorMessage();
    QSqlQuery("ROLLBACK").exec();
    return false;
  }

  const int id = data(row, primaryKey).isValid() ? data(row, primaryKey).toInt() : model.query().lastInsertId().toInt();

  for (int row = 0, rowCount = modelEnd.rowCount(); row < rowCount; ++row) {
    if (not setDataEnd(row, primaryKey, id)) {
      qDebug() << "erro setando id no endereco";
    }
  }

  if (not modelEnd.submitAll()) {
    qDebug() << objectName() << " : " << __LINE__ << " : Error on modelEnd.submitAll() : " << modelEnd.lastError();
    qDebug() << "Last query: "
             << modelEnd.database().driver()->sqlStatement(QSqlDriver::InsertStatement, modelEnd.tableName(),
                                                           modelEnd.record(row), false);
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

bool RegisterAddressDialog::setDataEnd(int row, const QString &key, QVariant value) {
  if (row == -1) {
    qDebug() << "Something wrong on the row!";
    return false;
  }

  if (modelEnd.fieldIndex(key) == -1) {
    qDebug() << objectName() << " : Key '" << key << "' not found on table '" << modelEnd.tableName() << "'";
    return false;
  }

  if (not value.isNull()) {
    if (not modelEnd.setData(modelEnd.index(row, modelEnd.fieldIndex(key)), value)) {
      qDebug() << "row: " << row;
      qDebug() << "column: " << modelEnd.fieldIndex(key);
      qDebug() << "key: " << key;
      qDebug() << "index: " << modelEnd.index(row, modelEnd.fieldIndex(key)).isValid();
      return false;
    }
  }

  return true;
}

bool RegisterAddressDialog::newRegister() {
  if (not confirmationMessage()) {
    return false;
  }

  registerMode();

  if (not model.select()) {
    qDebug() << "erro model: " << model.lastError();
    return false;
  }

  modelEnd.setFilter("idEndereco = 0");

  if (not modelEnd.select()) {
    qDebug() << "erro modelEnd: " << modelEnd.lastError();
    return false;
  }

  clearFields();

  return true;
}

int RegisterAddressDialog::getCodigoUF(QString uf) const {
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

bool RegisterAddressDialog::viewRegisterById(const QVariant id) {
  if (not RegisterDialog::viewRegisterById(id)) {
    qDebug() << "erro";
    return false;
  }

  if (data(primaryKey).isValid()) {
    modelEnd.setFilter(primaryKey + " = " + data(primaryKey).toString() + " AND desativado = FALSE");
  } else {
    qDebug() << "data not valid";
  }

  if (not modelEnd.select()) {
    qDebug() << modelEnd.lastError();
    qDebug() << "query: " << modelEnd.query().lastQuery();
    return false;
  }

  return true;
}

bool RegisterAddressDialog::viewRegister(const QModelIndex index) {
  if (not RegisterDialog::viewRegister(index)) {
    qDebug() << "erro";
    return false;
  }

  return true;
}
