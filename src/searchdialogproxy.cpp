#include <QBrush>
#include <QDate>

#include "searchdialogproxy.h"

SearchDialogProxy::SearchDialogProxy(SqlTableModel *model, QObject *parent)
    : QIdentityProxyModel(parent), estoque_promocao(model->fieldIndex("estoque_promocao")), descontinuado(model->fieldIndex("descontinuado")), validade(model->fieldIndex("validadeProdutos")) {
  setSourceModel(model);
}

QVariant SearchDialogProxy::data(const QModelIndex &proxyIndex, int role) const {
  if (role == Qt::BackgroundRole) {
    const int descontinuado = QIdentityProxyModel::data(index(proxyIndex.row(), this->descontinuado), Qt::DisplayRole).toInt();

    if (descontinuado == 1) return QBrush(Qt::red); // descontinuado

    const int estoque_promocao = QIdentityProxyModel::data(index(proxyIndex.row(), this->estoque_promocao), Qt::DisplayRole).toInt();

    if (estoque_promocao == 1) return QBrush(Qt::yellow); // estoque
    if (estoque_promocao == 2) return QBrush(Qt::green);  // promocao

    if (proxyIndex.column() == this->validade) {
      const QDate validade = QIdentityProxyModel::data(index(proxyIndex.row(), this->validade), Qt::DisplayRole).toDate();

      if (validade < QDate::currentDate()) return QBrush(Qt::red);
    }
  }

  if (role == Qt::ForegroundRole) {
    const int estoque_promocao = QIdentityProxyModel::data(index(proxyIndex.row(), this->estoque_promocao), Qt::DisplayRole).toInt();

    if (estoque_promocao == 1 or estoque_promocao == 2) return QBrush(Qt::black); // estoque/promocao
  }

  return QIdentityProxyModel::data(proxyIndex, role);
}
