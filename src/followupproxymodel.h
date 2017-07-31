#ifndef FOLLOWUPPROXYMODEL_H
#define FOLLOWUPPROXYMODEL_H

#include <QIdentityProxyModel>

#include "sqltablemodel.h"

class FollowUpProxyModel : public QIdentityProxyModel {

public:
  FollowUpProxyModel(SqlTableModel *model, QObject *parent = 0);
  ~FollowUpProxyModel() = default;
  QVariant data(const QModelIndex &proxyIndex, int role) const override;

private:
  const int semaforo;

  enum FieldColors { Quente = 1, Morno = 2, Frio = 3 };
};

#endif // FOLLOWUPPROXYMODEL_H
