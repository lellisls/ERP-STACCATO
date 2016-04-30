#include <QBrush>

#include "estoqueproxymodel.h"

EstoqueProxyModel::EstoqueProxyModel(SqlTableModel *model, QObject *parent)
    : QIdentityProxyModel(parent), column(model->fieldIndex("quantUpd")) {
  setSourceModel(model);
}

EstoqueProxyModel::~EstoqueProxyModel() {}

QVariant EstoqueProxyModel::data(const QModelIndex &proxyIndex, const int role) const {
  if (role == Qt::BackgroundRole) {
    const int value = QIdentityProxyModel::data(index(proxyIndex.row(), column), Qt::DisplayRole).toInt();

    if (value == 1) return QBrush(Qt::green);     // Ok
    if (value == 2) return QBrush(Qt::yellow);    // Quant difere
    if (value == 3) return QBrush(Qt::red);       // Não encontrado
    if (value == 4) return QBrush(Qt::darkGreen); // Consumo
    if (value == 5) return QBrush(Qt::cyan);      // Devolução
  }

  return QIdentityProxyModel::data(proxyIndex, role);
}
