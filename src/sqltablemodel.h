#ifndef SQLTABLEMODEL_H
#define SQLTABLEMODEL_H

#include <QSqlRelationalTableModel>

class SqlTableModel : public QSqlRelationalTableModel {
  public:
    explicit SqlTableModel(QObject *parent = 0);

    // QAbstractItemModel interface
    QVariant data(const int row, const int column) const;
    QVariant data(const int row, const QString column) const;
    bool setData(const int row, const int column, const QVariant &value);
    bool setData(const int row, const QString column, const QVariant &value);

    // TODO: reimplement setHeaderData to reduce verbosity
    // TODO: reimplement functions that are too verbose (fieldIndex, hideColumn)
};

#endif // SQLTABLEMODEL_H
