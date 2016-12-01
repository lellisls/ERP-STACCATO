#include <QBrush>

#include "financeiroproxymodel.h"

FinanceiroProxyModel::FinanceiroProxyModel(SqlTableModel *model, QObject *parent)
    : QIdentityProxyModel(parent), statusFinanceiro(model->fieldIndex("statusFinanceiro")) {
  setSourceModel(model);
}

FinanceiroProxyModel::~FinanceiroProxyModel() {}

QVariant FinanceiroProxyModel::data(const QModelIndex &proxyIndex, int role) const {
  if (role == Qt::BackgroundRole and proxyIndex.column() == this->statusFinanceiro) {
    const QString status =
        QIdentityProxyModel::data(index(proxyIndex.row(), this->statusFinanceiro), Qt::DisplayRole).toString();

    if (status == "PENDENTE") return QBrush(Qt::red);
    if (status == "CONFERIDO") return QBrush(Qt::yellow);
    if (status == "LIBERADO") return QBrush(Qt::green);
  }

  return QIdentityProxyModel::data(proxyIndex, role);
}
