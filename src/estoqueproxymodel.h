#ifndef ESTOQUEPROXYMODEL_H
#define ESTOQUEPROXYMODEL_H

#include <QIdentityProxyModel>

#include "sqltablemodel.h"

class EstoqueProxyModel : public QIdentityProxyModel {
  public:
    explicit EstoqueProxyModel(SqlTableModel *model, const QString &column, QObject *parent = 0);
    ~EstoqueProxyModel();
    // QAbstractItemModel interface
    QVariant data(const QModelIndex &proxyIndex, const int role) const;

  private:
    const int column;
};

#endif // ESTOQUEPROXYMODEL_H
