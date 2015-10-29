#ifndef SQLTABLEMODEL_H
#define SQLTABLEMODEL_H

#include <QObject>
#include <QSqlRelationalTableModel>

class SqlTableModel : public QSqlRelationalTableModel {
  public:
    explicit SqlTableModel(QObject *parent = 0);

    // QAbstractItemModel interface
  public:
    QVariant data(const int row, const int column) const;
    QVariant data(const int row, const QString column) const;
    QVariant data(const QModelIndex index) const;
    bool setData(const int row, const int column, const QVariant &value);
    bool setData(const int row, const QString column, const QVariant &value);
    bool setData(const QModelIndex index, const QVariant &value);
};

#endif // SQLTABLEMODEL_H
