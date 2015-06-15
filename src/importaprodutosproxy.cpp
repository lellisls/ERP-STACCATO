#include <QBrush>
#include <QDebug>

#include "importaprodutosproxy.h"

ImportaProdutosProxy::ImportaProdutosProxy(int column, QObject *parent) : QIdentityProxyModel(parent), column(column) {}

ImportaProdutosProxy::~ImportaProdutosProxy() {}

QVariant ImportaProdutosProxy::data(const QModelIndex &proxyIndex, int role) const {
  if (role == Qt::BackgroundRole) {

    // verifica se está descontinuado
    int value = QIdentityProxyModel::data(index(proxyIndex.row(), column), Qt::DisplayRole).toInt();

    if (value == 1) {
      return QBrush(Qt::red);
    }

    // verifica cada campo
    for (int i = 0; i < columnCount(); ++i) {
      if (i % 2 == 0 and proxyIndex.column() == i) {
        int value = QIdentityProxyModel::data(index(proxyIndex.row(), i + 1), Qt::DisplayRole).toInt();

        if (value == 1) {
          return QBrush(Qt::green);
        }

        if (value == 2) {
          return QBrush(Qt::yellow);
        }

        if (value == 3) {
          return QBrush(Qt::red);
        }
      }
    }
  }

  return QIdentityProxyModel::data(proxyIndex, role);
}
