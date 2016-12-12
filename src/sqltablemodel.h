#ifndef SQLTABLEMODEL_H
#define SQLTABLEMODEL_H

#include <QSqlRelationalTableModel>

class SqlTableModel : public QSqlRelationalTableModel {

public:
  explicit SqlTableModel(QObject *parent = 0);
  bool setData(const int row, const int column, const QVariant &value);
  bool setData(const int row, const QString &column, const QVariant &value);
  bool setHeaderData(const QString &column, const QVariant &value);
  QVariant data(const int row, const int column) const;
  QVariant data(const int row, const QString &column) const;
  virtual Qt::DropActions supportedDropActions() const override;
  virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
  using QSqlRelationalTableModel::data;
  using QSqlRelationalTableModel::setData;
  using QSqlRelationalTableModel::setHeaderData;
};

#endif // SQLTABLEMODEL_H
