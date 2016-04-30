#ifndef FOLLOWUPPROXY_H
#define FOLLOWUPPROXY_H

#include <QIdentityProxyModel>

#include "sqltablemodel.h"

class FollowUpProxy : public QIdentityProxyModel {

public:
  FollowUpProxy(SqlTableModel *model, QObject *parent = 0);
  ~FollowUpProxy();
  QVariant data(const QModelIndex &proxyIndex, int role) const override;

private:
  const int column;

  enum FieldColors { Quente = 1, Morno = 2, Frio = 3 };
};

#endif // FOLLOWUPPROXY_H
