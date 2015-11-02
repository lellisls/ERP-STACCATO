#ifndef SQLQUERYMODEL_H
#define SQLQUERYMODEL_H

#include <QSqlQueryModel>

class SqlQueryModel : public QSqlQueryModel {
  public:
    explicit SqlQueryModel(QObject *parent = 0);

    // QAbstractItemModel interface
  public:
    QVariant data(int row, QString column) const;
};

#endif // SQLQUERYMODEL_H
