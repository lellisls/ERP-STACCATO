#include <QBrush>

#include "searchdialogproxy.h"

SearchDialogProxy::SearchDialogProxy(SqlTableModel *model, QObject *parent)
    : QIdentityProxyModel(parent), column(model->fieldIndex("estoque_promocao")) {
  setSourceModel(model);
}

SearchDialogProxy::~SearchDialogProxy() {}

QVariant SearchDialogProxy::data(const QModelIndex &proxyIndex, int role) const {
  if (role == Qt::BackgroundRole) {
    const int value = QIdentityProxyModel::data(index(proxyIndex.row(), column), Qt::DisplayRole).toInt();

    if (value == 1) return QBrush(Qt::yellow); // estoque
    if (value == 2) return QBrush(Qt::green);  // promocao
  }

  if (role == Qt::ForegroundRole) {
    const int value = QIdentityProxyModel::data(index(proxyIndex.row(), column), Qt::DisplayRole).toInt();

    if (value == 1 or value == 2) return QBrush(Qt::black); // estoque/promocao
  }

  return QIdentityProxyModel::data(proxyIndex, role);
}
