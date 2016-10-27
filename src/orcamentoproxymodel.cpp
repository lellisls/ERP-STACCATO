#include <QBrush>

#include "orcamentoproxymodel.h"

OrcamentoProxyModel::OrcamentoProxyModel(SqlTableModel *model, QObject *parent)
    : QIdentityProxyModel(parent), dias(model->fieldIndex("Dias restantes")), status(model->fieldIndex("status")),
      followup(model->fieldIndex("Observação")), semaforo(model->fieldIndex("semaforo")) {
  setSourceModel(model);
}

OrcamentoProxyModel::~OrcamentoProxyModel() {}

QVariant OrcamentoProxyModel::data(const QModelIndex &proxyIndex, const int role) const {
  if (role == Qt::BackgroundRole) {
    if (proxyIndex.column() == dias) {
      int value = QIdentityProxyModel::data(index(proxyIndex.row(), dias), Qt::DisplayRole).toInt();

      if (value >= 5) return QBrush(Qt::green);
      if (value >= 3) return QBrush(Qt::yellow);
      if (value < 3) return QBrush(Qt::red);
    } else if (proxyIndex.column() == followup) {
      int value = QIdentityProxyModel::data(index(proxyIndex.row(), semaforo), Qt::DisplayRole).toInt();

      if (value == Quente) return QBrush(QColor(255, 66, 66));
      if (value == Morno) return QBrush(QColor(255, 170, 0));
      if (value == Frio) return QBrush(QColor(70, 113, 255));
    } else {
      QString statusValue = QIdentityProxyModel::data(index(proxyIndex.row(), status), Qt::DisplayRole).toString();
      if (statusValue == "FECHADO") return QBrush(Qt::green);
      if (statusValue == "CANCELADO") return QBrush(Qt::yellow);
      if (statusValue == "PERDIDO") return QBrush(Qt::yellow);
    }
  }

  if (role == Qt::ForegroundRole) {
    QString statusValue = QIdentityProxyModel::data(index(proxyIndex.row(), status), Qt::DisplayRole).toString();
    if (statusValue == "FECHADO" or proxyIndex.column() == dias) return QBrush(Qt::black);
  }

  return QIdentityProxyModel::data(proxyIndex, role);
}
