#include "sqltablemodel.h"

SqlTableModel::SqlTableModel(QObject *parent) : QSqlTableModel(parent) {}

QVariant SqlTableModel::data(const int row, int column) const {
  return QSqlTableModel::data(QSqlTableModel::index(row, column));
}

QVariant SqlTableModel::data(const int row, const QString column) const {
  return QSqlTableModel::data(QSqlTableModel::index(row, QSqlTableModel::fieldIndex(column)));
}

bool SqlTableModel::setData(const int row, const int column, const QVariant &value) {
  return QSqlTableModel::setData(QSqlTableModel::index(row, column), value);
}

bool SqlTableModel::setData(const int row, const QString column, const QVariant &value) {
  return QSqlTableModel::setData(QSqlTableModel::index(row, QSqlTableModel::fieldIndex(column)), value);
}
