#include <QBrush>
#include <QDebug>

#include "backgroundproxymodel.h"

BackgroundProxyModel::BackgroundProxyModel(int column) : column(column) {}

BackgroundProxyModel::~BackgroundProxyModel() {}

QVariant BackgroundProxyModel::data(const QModelIndex &proxyIndex, const int role) const {
  if ((role == Qt::BackgroundRole) and (proxyIndex.column() == column)) {
    int value = QIdentityProxyModel::data(index(proxyIndex.row(), column), Qt::DisplayRole).toInt();

    if (value >= 5) {
      return QBrush(Qt::green);
    }

    if (value >= 3) {
      return QBrush(Qt::yellow);
    }

    if (value < 3) {
      return QBrush(Qt::red);
    }
  }

  return QIdentityProxyModel::data(proxyIndex, role);
}
