#ifndef BACKGROUNDPROXYMODEL_H
#define BACKGROUNDPROXYMODEL_H

#include <QIdentityProxyModel>

class BackgroundProxyModel : public QIdentityProxyModel {
  public:
    BackgroundProxyModel(int column);
    ~BackgroundProxyModel();

    // QAbstractItemModel interface
  public:
    QVariant data(const QModelIndex &proxyIndex, int role) const;

  private:
    int column;
};

#endif // BACKGROUNDPROXYMODEL_H
