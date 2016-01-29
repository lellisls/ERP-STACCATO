#include <QBrush>

#include "searchdialogproxy.h"

SearchDialogProxy::SearchDialogProxy(SqlTableModel *model, const int &column, QObject *parent)
  : QIdentityProxyModel(parent), column(column) {
  setSourceModel(model);
}

SearchDialogProxy::~SearchDialogProxy() {}

QVariant SearchDialogProxy::data(const QModelIndex &proxyIndex, int role) const {
  if (role == Qt::BackgroundRole) {
    const int value = QIdentityProxyModel::data(index(proxyIndex.row(), column), Qt::DisplayRole).toInt();

    if (value == 1) return QBrush(Qt::yellow); // estoque
    if (value == 2) return QBrush(Qt::green);  // promocao
  }

  return QIdentityProxyModel::data(proxyIndex, role);
}
