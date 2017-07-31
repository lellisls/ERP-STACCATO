#ifndef ESTOQUEPRAZOPROXYMODEL_H
#define ESTOQUEPRAZOPROXYMODEL_H

#include <QIdentityProxyModel>

#include "sqltablemodel.h"

class EstoquePrazoProxyModel : public QIdentityProxyModel {

public:
  explicit EstoquePrazoProxyModel(SqlTableModel *model, QObject *parent);
  ~EstoquePrazoProxyModel() = default;
  QVariant data(const QModelIndex &proxyIndex, const int role) const override;

private:
  const int dias;
};

#endif // ESTOQUEPRAZOPROXYMODEL_H
