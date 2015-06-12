#ifndef IMPORTAPRODUTOSPROXY_H
#define IMPORTAPRODUTOSPROXY_H

#include <QIdentityProxyModel>

class ImportaProdutosProxy : public QIdentityProxyModel {

  public:
    ImportaProdutosProxy(int column, QObject *parent = 0);
    ~ImportaProdutosProxy();

    // QAbstractItemModel interface
    QVariant data(const QModelIndex &proxyIndex, int role) const;

  private:
    int column;
};

#endif // IMPORTAPRODUTOSPROXY_H
