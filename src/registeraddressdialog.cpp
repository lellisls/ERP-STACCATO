#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "registeraddressdialog.h"

RegisterAddressDialog::RegisterAddressDialog(const QString &table, const QString &primaryKey, QWidget *parent)
    : RegisterDialog(table, primaryKey, parent) {
  setWindowModality(Qt::NonModal);
  setWindowFlags(Qt::Window);

  setupTables(table);
}

void RegisterAddressDialog::setupTables(const QString &table) {
  modelEnd.setTable(table + "_has_endereco");
  modelEnd.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelEnd.setHeaderData("descricao", "Descrição");
  modelEnd.setHeaderData("cep", "CEP");
  modelEnd.setHeaderData("logradouro", "Logradouro");
  modelEnd.setHeaderData("numero", "Número");
  modelEnd.setHeaderData("complemento", "Compl.");
  modelEnd.setHeaderData("bairro", "Bairro");
  modelEnd.setHeaderData("cidade", "Cidade");
  modelEnd.setHeaderData("uf", "UF");

  modelEnd.setFilter("0");

  if (not modelEnd.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Ocorreu um erro ao acessar a tabela de endereço: " + modelEnd.lastError().text());
  }

  mapperEnd.setModel(&modelEnd);
  mapperEnd.setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
}

bool RegisterAddressDialog::cadastrar() {
  row = isUpdate ? mapper.currentIndex() : model.rowCount();

  if (row == -1) {
    QMessageBox::critical(this, "Erro!", "Erro linha -1");
    return false;
  }

  if (not isUpdate and not model.insertRow(row)) return false;

  if (not savingProcedures()) return false;

  for (int column = 0; column < model.rowCount(); ++column) {
    QVariant dado = model.data(row, column);
    if (dado.type() == QVariant::String) {
      if (not model.setData(row, column, dado.toString().toUpper())) return false;
    }
  }

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro: " + model.lastError().text());
    return false;
  }

  primaryId = data(row, primaryKey).isValid() ? data(row, primaryKey).toString() : getLastInsertId().toString();

  if (primaryId.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "primaryId está vazio!");
    return false;
  }

  for (int row = 0, rowCount = modelEnd.rowCount(); row < rowCount; ++row) {
    if (not modelEnd.setData(row, primaryKey, primaryId)) return false;
  }

  for (int column = 0; column < modelEnd.rowCount(); ++column) {
    QVariant dado = modelEnd.data(row, column);
    if (dado.type() == QVariant::String) {
      if (not modelEnd.setData(row, column, dado.toString().toUpper())) return false;
    }
  }

  if (not modelEnd.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro: " + modelEnd.lastError().text());
    return false;
  }

  return true;
}

bool RegisterAddressDialog::setDataEnd(const QString &key, const QVariant &value) {
  if (value.isNull() or (value.type() == QVariant::String and value.toString().isEmpty())) return true;

  int currentRow = rowEnd != -1 ? rowEnd : mapperEnd.currentIndex();

  return modelEnd.setData(currentRow, key, value);
}

bool RegisterAddressDialog::newRegister() {
  if (not confirmationMessage()) return false;

  isUpdate = false;

  clearFields();
  registerMode();

  modelEnd.setFilter("0");

  if (not modelEnd.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela endereço: " + modelEnd.lastError().text());
    return false;
  }

  return true;
}

int RegisterAddressDialog::getCodigoUF(QString uf) const {
  uf = uf.toLower();

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

bool RegisterAddressDialog::viewRegisterById(const QVariant &id) {
  if (not RegisterDialog::viewRegisterById(id)) return false;

  modelEnd.setFilter(primaryKey + " = " + data(primaryKey).toString() + " AND desativado = FALSE");

  if (not modelEnd.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela endereço: " + modelEnd.lastError().text());
    return false;
  }

  return true;
}
