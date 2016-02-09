#ifndef ORCAMENTOPROXYMODEL_H
#define ORCAMENTOPROXYMODEL_H

#include <QIdentityProxyModel>

#include "sqltablemodel.h"

class OrcamentoProxyModel : public QIdentityProxyModel {
  public:
    explicit OrcamentoProxyModel(SqlTableModel *model, QObject *parent);
    ~OrcamentoProxyModel();
    QVariant data(const QModelIndex &proxyIndex, const int role) const override;

  private:
    const int column;
};

#endif // ORCAMENTOPROXYMODEL_H
