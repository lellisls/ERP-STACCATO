#ifndef IMPORTAPRODUTOSPROXY_H
#define IMPORTAPRODUTOSPROXY_H

#include <QIdentityProxyModel>

#include "sqltablemodel.h"

class ImportaProdutosProxy : public QIdentityProxyModel {

  public:
    ImportaProdutosProxy(SqlTableModel *model, const int &column, QObject *parent = 0);
    ~ImportaProdutosProxy();
    // QAbstractItemModel interface
    QVariant data(const QModelIndex &proxyIndex, int &role) const;

  private:
    const int column;
};

#endif // IMPORTAPRODUTOSPROXY_H
