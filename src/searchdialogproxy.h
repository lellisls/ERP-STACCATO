#ifndef SEARCHDIALOGPROXY_H
#define SEARCHDIALOGPROXY_H

#include <QIdentityProxyModel>

#include <src/sqltablemodel.h>

class SearchDialogProxy : public QIdentityProxyModel {
  public:
    SearchDialogProxy(SqlTableModel *model, const int &column, QObject *parent = 0);
    ~SearchDialogProxy();
    QVariant data(const QModelIndex &proxyIndex, int role) const override;

  private:
    const int column;
};

#endif // SEARCHDIALOGPROXY_H
