#include <QSqlRecord>

#include "sqlquerymodel.h"

SqlQueryModel::SqlQueryModel(QObject *parent) : QSqlQueryModel(parent) {}

QVariant SqlQueryModel::data(int row, QString column) const {
  return QSqlQueryModel::data(QSqlQueryModel::index(row, QSqlQueryModel::record().indexOf(column)));
}

bool SqlQueryModel::setHeaderData(QString column, const QVariant &value) {
  return QSqlQueryModel::setHeaderData(QSqlQueryModel::record().indexOf(column), Qt::Horizontal, value);
}
