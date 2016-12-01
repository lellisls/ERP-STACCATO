#include <QBrush>

#include "estoqueprazoproxymodel.h"

EstoquePrazoProxyModel::EstoquePrazoProxyModel(SqlTableModel *model, QObject *parent)
    : QIdentityProxyModel(parent), dias(model->fieldIndex("Dias restantes")) {
  setSourceModel(model);
}

EstoquePrazoProxyModel::~EstoquePrazoProxyModel() {}

QVariant EstoquePrazoProxyModel::data(const QModelIndex &proxyIndex, const int role) const {
  if (role == Qt::BackgroundRole) {
    if (proxyIndex.column() == this->dias) {
      const int dias = QIdentityProxyModel::data(index(proxyIndex.row(), this->dias), Qt::DisplayRole).toInt();

      if (dias >= 5) return QBrush(Qt::green);
      if (dias >= 3) return QBrush(Qt::yellow);
      if (dias < 3) return QBrush(Qt::red);
    }
  }

  return QIdentityProxyModel::data(proxyIndex, role);
}
