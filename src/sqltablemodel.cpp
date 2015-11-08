#include <QMessageBox>
#include <QSqlError>
#include <QSqlRecord>

#include "sqltablemodel.h"

SqlTableModel::SqlTableModel(QObject *parent) : QSqlRelationalTableModel(parent) {}

QVariant SqlTableModel::data(const int row, int column) const {
  return QSqlTableModel::data(QSqlTableModel::index(row, column));
}

QVariant SqlTableModel::data(const int row, const QString column) const {
  if (QSqlTableModel::fieldIndex(column) == -1) {
    QMessageBox::critical(0, "Erro!", "Chave " + column + " não encontrada na tabela " + QSqlTableModel::tableName());
    return QVariant();
  }

  return QSqlTableModel::data(QSqlTableModel::index(row, QSqlTableModel::fieldIndex(column)));
}

bool SqlTableModel::setData(const int row, const int column, const QVariant &value) {
  if (not QSqlTableModel::setData(QSqlTableModel::index(row, column), value)) {
    QMessageBox::critical(0, "Erro!", "Erro inserindo " + QSqlTableModel::record().fieldName(column) + " na tabela: " +
                          QSqlTableModel::lastError().text());
    return false;
  }

  return true;
}

bool SqlTableModel::setData(const int row, const QString column, const QVariant &value) {
  if (row == -1) {
    QMessageBox::critical(0, "Erro!", "Erro: linha -1");
    return false;
  }

  if (QSqlTableModel::fieldIndex(column) == -1) {
    QMessageBox::critical(0, "Erro!", "Chave " + column + " não encontrada na tabela " + QSqlTableModel::tableName());
    return false;
  }

  if (not QSqlTableModel::setData(QSqlTableModel::index(row, QSqlTableModel::fieldIndex(column)), value)) {
    QMessageBox::critical(0, "Erro!", "Erro inserindo " + column + " na tabela: " + QSqlTableModel::lastError().text());
    return false;
  }

  return true;
}
