#ifndef SEARCHDIALOGPROXY_H
#define SEARCHDIALOGPROXY_H

#include <QIdentityProxyModel>

#include "sqltablemodel.h"

class SearchDialogProxy : public QIdentityProxyModel {

public:
  SearchDialogProxy(SqlTableModel *model, QObject *parent = 0);
  ~SearchDialogProxy() = default;
  QVariant data(const QModelIndex &proxyIndex, int role) const override;

private:
  const int estoque_promocao;
  const int descontinuado;
  const int validade;
};

#endif // SEARCHDIALOGPROXY_H
