#ifndef VENDAPROXYMODEL_H
#define VENDAPROXYMODEL_H

#include <QIdentityProxyModel>

#include "sqltablemodel.h"

class VendaProxyModel : public QIdentityProxyModel {

public:
  explicit VendaProxyModel(SqlTableModel *model, QObject *parent);
  ~VendaProxyModel() = default;
  QVariant data(const QModelIndex &proxyIndex, const int role) const override;

private:
  const int dias;
  const int status;
  const int followup;
  const int semaforo;
  const int financeiro;

  enum FieldColors { Quente = 1, Morno = 2, Frio = 3 };
};

#endif // VENDAPROXYMODEL_H
