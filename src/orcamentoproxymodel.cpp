#include <QBrush>

#include "orcamentoproxymodel.h"

OrcamentoProxyModel::OrcamentoProxyModel(SqlTableModel *model, QObject *parent)
    : QIdentityProxyModel(parent), column(model->fieldIndex("Dias restantes")), status(model->fieldIndex("status")) {
  setSourceModel(model);
}

OrcamentoProxyModel::~OrcamentoProxyModel() {}

QVariant OrcamentoProxyModel::data(const QModelIndex &proxyIndex, const int role) const {
  if (role == Qt::BackgroundRole) {
    QString statusValue = QIdentityProxyModel::data(index(proxyIndex.row(), status), Qt::DisplayRole).toString();
    if (statusValue == "FECHADO") return QBrush(Qt::green);
  }

  if (role == Qt::BackgroundRole and proxyIndex.column() == column) {
    int value = QIdentityProxyModel::data(index(proxyIndex.row(), column), Qt::DisplayRole).toInt();

    if (value >= 5) return QBrush(Qt::green);
    if (value >= 3) return QBrush(Qt::yellow);
    if (value < 3) return QBrush(Qt::red);
  }

  return QIdentityProxyModel::data(proxyIndex, role);
}
