#ifndef BACKGROUNDPROXYMODEL_H
#define BACKGROUNDPROXYMODEL_H

#include <QIdentityProxyModel>

class BackgroundProxyModel : public QIdentityProxyModel {
  public:
    explicit BackgroundProxyModel(int column);
    ~BackgroundProxyModel();

    // QAbstractItemModel interface
  public:
    QVariant data(const QModelIndex &proxyIndex, const int role) const;

  private:
    const int column;
};

#endif // BACKGROUNDPROXYMODEL_H
