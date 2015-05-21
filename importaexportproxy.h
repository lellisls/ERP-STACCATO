#ifndef IMPORTAEXPORTPROXY_H
#define IMPORTAEXPORTPROXY_H

#include <QIdentityProxyModel>

class ImportaExportProxy : public QIdentityProxyModel {
  public:
    ImportaExportProxy(int column, QObject *parent = 0);
    ~ImportaExportProxy();

    // QAbstractItemModel interface
    QVariant data(const QModelIndex &proxyIndex, int role) const;

  private:
    int column;
};

#endif // IMPORTAEXPORTPROXY_H
