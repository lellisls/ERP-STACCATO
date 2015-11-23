#include <QBrush>

#include "estoqueproxymodel.h"

EstoqueProxyModel::EstoqueProxyModel(SqlTableModel *model, const QString &column, QObject *parent)
  : QIdentityProxyModel(parent), column(model->fieldIndex(column)) {
  setSourceModel(model);
}

EstoqueProxyModel::~EstoqueProxyModel() {}

QVariant EstoqueProxyModel::data(const QModelIndex &proxyIndex, const int &role) const {
  if (role == Qt::BackgroundRole) {
    const int value = QIdentityProxyModel::data(index(proxyIndex.row(), column), Qt::DisplayRole).toInt();

    if (value == 1) { // Ok
      return QBrush(Qt::green);
    }

    if (value == 2) { // Quant difere
      return QBrush(Qt::yellow);
    }

    if (value == 3) { // Não encontrado
      return QBrush(Qt::red);
    }
  }

  return QIdentityProxyModel::data(proxyIndex, role);
}
