#ifndef SQLQUERYMODEL_H
#define SQLQUERYMODEL_H

#include <QSqlQueryModel>

class SqlQueryModel : public QSqlQueryModel {

public:
  explicit SqlQueryModel(QObject *parent = 0);
  bool setHeaderData(const QString &column, const QVariant &value);
  QVariant data(const int row, const QString &column) const;

private:
  using QSqlQueryModel::setHeaderData;
  using QSqlQueryModel::data;
};

#endif // SQLQUERYMODEL_H
