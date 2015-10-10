#ifndef SQLTABLEMODEL_H
#define SQLTABLEMODEL_H

#include <QObject>
#include <QSqlTableModel>

class SqlTableModel : public QSqlTableModel {
  public:
    explicit SqlTableModel(QObject *parent = 0);

    // QAbstractItemModel interface
  public:
    QVariant data(const int row, const int column) const;
    QVariant data(const int row, const QString column) const;
    bool setData(const int row, const int column, const QVariant &value);
    bool setData(const int row, const QString column, const QVariant &value);
};

#endif // SQLTABLEMODEL_H
