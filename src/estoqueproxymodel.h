#ifndef ESTOQUEPROXYMODEL_H
#define ESTOQUEPROXYMODEL_H

#include <QIdentityProxyModel>

#include "sqltablemodel.h"

class EstoqueProxyModel : public QIdentityProxyModel {

public:
  explicit EstoqueProxyModel(SqlTableModel *model, QObject *parent = 0);
  ~EstoqueProxyModel() = default;
  QVariant data(const QModelIndex &proxyIndex, const int role) const override;

private:
  const int quantUpd;
};

#endif // ESTOQUEPROXYMODEL_H
