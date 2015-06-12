#include <QBrush>
#include <QDebug>

#include "importaprodutosproxy.h"

ImportaProdutosProxy::ImportaProdutosProxy(QObject *parent) : QIdentityProxyModel(parent) {}

ImportaProdutosProxy::~ImportaProdutosProxy() {}

QVariant ImportaProdutosProxy::data(const QModelIndex &proxyIndex, int role) const {
  if (role == Qt::BackgroundRole) {

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
      }
    }
  }

  return QIdentityProxyModel::data(proxyIndex, role);
}
