#ifndef FINANCEIROPROXYMODEL_H
#define FINANCEIROPROXYMODEL_H

#include <QIdentityProxyModel>

#include "sqltablemodel.h"

class FinanceiroProxyModel : public QIdentityProxyModel {

public:
  FinanceiroProxyModel(SqlTableModel *model, QObject *parent = 0);
  ~FinanceiroProxyModel() = default;
  QVariant data(const QModelIndex &proxyIndex, int role) const;

private:
  const int statusFinanceiro;
  const int prazoEntrega;
  const int novoPrazoEntrega;
};

#endif // FINANCEIROPROXYMODEL_H
