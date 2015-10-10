#ifndef ORCAMENTOPROXYMODEL_H
#define ORCAMENTOPROXYMODEL_H

#include <QIdentityProxyModel>
#include <QSqlTableModel>

class OrcamentoProxyModel : public QIdentityProxyModel {
  public:
    explicit OrcamentoProxyModel(QSqlQueryModel *model, int column, QObject *parent);
    ~OrcamentoProxyModel();
    // QAbstractItemModel interface
    QVariant data(const QModelIndex &proxyIndex, const int role) const;

  private:
    const int column;
};

#endif // ORCAMENTOPROXYMODEL_H
