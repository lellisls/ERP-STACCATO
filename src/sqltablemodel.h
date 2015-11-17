#ifndef SQLTABLEMODEL_H
#define SQLTABLEMODEL_H

#include <QSqlRelationalTableModel>

class SqlTableModel : public QSqlRelationalTableModel {
  public:
    explicit SqlTableModel(QObject *parent = 0);
    bool setHeaderData(QString column, const QVariant &value);

    // QAbstractItemModel interface
    QVariant data(const int row, const int column) const;
    QVariant data(const int row, const QString column) const;
    bool setData(const int row, const int column, const QVariant &value);
    bool setData(const int row, const QString column, const QVariant &value);
};

#endif // SQLTABLEMODEL_H
