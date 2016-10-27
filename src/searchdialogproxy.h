#ifndef SEARCHDIALOGPROXY_H
#define SEARCHDIALOGPROXY_H

#include <QIdentityProxyModel>

#include "sqltablemodel.h"

class SearchDialogProxy : public QIdentityProxyModel {

public:
  SearchDialogProxy(SqlTableModel *model, QObject *parent = 0);
  ~SearchDialogProxy();
  QVariant data(const QModelIndex &proxyIndex, int role) const override;

private:
  const int column;
  const int column2;
};

#endif // SEARCHDIALOGPROXY_H
