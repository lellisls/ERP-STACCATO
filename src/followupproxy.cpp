#include <QBrush>

#include "followupproxy.h"

FollowUpProxy::FollowUpProxy(SqlTableModel *model, QObject *parent)
    : QIdentityProxyModel(parent), column(model->fieldIndex("semaforo")) {
  setSourceModel(model);
}

FollowUpProxy::~FollowUpProxy() {}

QVariant FollowUpProxy::data(const QModelIndex &proxyIndex, int role) const {
  if (role == Qt::BackgroundRole) {

    const int value = QIdentityProxyModel::data(index(proxyIndex.row(), column), Qt::DisplayRole).toInt();

    if (value == Quente) return QBrush(QColor(255, 66, 66));
    if (value == Morno) return QBrush(QColor(255, 170, 0));
    if (value == Frio) return QBrush(QColor(70, 113, 255));
  }

  return QIdentityProxyModel::data(proxyIndex, role);
}
