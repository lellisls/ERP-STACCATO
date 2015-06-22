#include <QBrush>
#include <QDebug>

#include "importaprodutosproxy.h"

ImportaProdutosProxy::ImportaProdutosProxy(const int column, QObject *parent)
  : QIdentityProxyModel(parent), column(column) {}

ImportaProdutosProxy::~ImportaProdutosProxy() {}

QVariant ImportaProdutosProxy::data(const QModelIndex &proxyIndex, int role) const {
  if (role == Qt::BackgroundRole) {

    // verifica se está descontinuado
    const int value = QIdentityProxyModel::data(index(proxyIndex.row(), column), Qt::DisplayRole).toInt();

    if (value == 1) {
      return QBrush(Qt::cyan);
    }

    // verifica cada campo
    for (int i = 0, columns = columnCount(); i < columns; ++i) {
      if (i % 2 == 0 and proxyIndex.column() == i) {
        const int value = QIdentityProxyModel::data(index(proxyIndex.row(), i + 1), Qt::DisplayRole).toInt();

        if (value == 1) {
          return QBrush(Qt::green);
        }

        if (value == 2) {
          return QBrush(Qt::yellow);
        }

        if (value == 3) {
          return QBrush(Qt::gray);
        }

        if (value == 4) {
          return QBrush(Qt::red);
        }
      }
    }
  }

  return QIdentityProxyModel::data(proxyIndex, role);
}
