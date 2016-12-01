#ifndef FINANCEIROPROXYMODEL_H
#define FINANCEIROPROXYMODEL_H

#include <QIdentityProxyModel>

#include "sqltablemodel.h"

class FinanceiroProxyModel : public QIdentityProxyModel {

public:
  FinanceiroProxyModel(SqlTableModel *model, QObject *parent = 0);
  ~FinanceiroProxyModel();
  QVariant data(const QModelIndex &proxyIndex, int role) const;

private:
  const int statusFinanceiro;
};

#endif // FINANCEIROPROXYMODEL_H
