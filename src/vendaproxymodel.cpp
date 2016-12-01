#include <QBrush>

#include "vendaproxymodel.h"

VendaProxyModel::VendaProxyModel(SqlTableModel *model, QObject *parent)
    : QIdentityProxyModel(parent), dias(model->fieldIndex("Dias restantes")), status(model->fieldIndex("Status")),
      followup(model->fieldIndex("Observação")), semaforo(model->fieldIndex("semaforo")),
      financeiro(model->fieldIndex("statusFinanceiro")) {
  setSourceModel(model);
}

VendaProxyModel::~VendaProxyModel() {}

QVariant VendaProxyModel::data(const QModelIndex &proxyIndex, const int role) const {
  if (role == Qt::BackgroundRole) {
    if (proxyIndex.column() == dias) {
      const int value = QIdentityProxyModel::data(index(proxyIndex.row(), dias), Qt::DisplayRole).toInt();
      const QString valueStatus =
          QIdentityProxyModel::data(index(proxyIndex.row(), status), Qt::DisplayRole).toString();

      if (value >= 5 or valueStatus == "ENTREGUE") return QBrush(Qt::green);

      if (value >= 3 or valueStatus == "CANCELADO" or valueStatus == "DEVOLVIDO" or valueStatus == "PROCESSADO") {
        return QBrush(Qt::yellow);
      }

      if (value < 3) return QBrush(Qt::red);
    }

    if (proxyIndex.column() == followup) {
      const int value = QIdentityProxyModel::data(index(proxyIndex.row(), semaforo), Qt::DisplayRole).toInt();

      if (value == Quente) return QBrush(QColor(255, 66, 66));
      if (value == Morno) return QBrush(QColor(255, 170, 0));
      if (value == Frio) return QBrush(QColor(70, 113, 255));
    }

    if (proxyIndex.column() == financeiro) {
      const QString value = QIdentityProxyModel::data(index(proxyIndex.row(), financeiro), Qt::DisplayRole).toString();

      if (value == "PENDENTE") return QBrush(Qt::red);
      if (value == "CONFERIDO") return QBrush(Qt::yellow);
      if (value == "LIBERADO") return QBrush(Qt::green);
    }

    const QString value = QIdentityProxyModel::data(index(proxyIndex.row(), status), Qt::DisplayRole).toString();
    if (value == "ENTREGUE") return QBrush(Qt::green);
    if (value == "CANCELADO" or value == "DEVOLVIDO" or value == "PROCESSADO") return QBrush(Qt::yellow);
    if (value == "PERDIDO") return QBrush(Qt::yellow);
  }

  if (role == Qt::ForegroundRole) {
    const QString value = QIdentityProxyModel::data(index(proxyIndex.row(), status), Qt::DisplayRole).toString();
    if (value == "ENTREGUE" or proxyIndex.column() == dias) return QBrush(Qt::black);
  }

  return QIdentityProxyModel::data(proxyIndex, role);
}
