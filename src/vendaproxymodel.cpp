#include <QBrush>

#include "vendaproxymodel.h"

VendaProxyModel::VendaProxyModel(SqlTableModel *model, QObject *parent)
    : QIdentityProxyModel(parent), dias(model->fieldIndex("Dias restantes")), status(model->fieldIndex("Status")), followup(model->fieldIndex("Observação")), semaforo(model->fieldIndex("semaforo")),
      financeiro(model->fieldIndex("statusFinanceiro")) {
  setSourceModel(model);
}

QVariant VendaProxyModel::data(const QModelIndex &proxyIndex, const int role) const {
  if (role == Qt::BackgroundRole) {
    if (proxyIndex.column() == this->dias) {
      const int dias = QIdentityProxyModel::data(index(proxyIndex.row(), this->dias), Qt::DisplayRole).toInt();
      const QString status = QIdentityProxyModel::data(index(proxyIndex.row(), this->status), Qt::DisplayRole).toString();

      if (dias >= 5 or status == "ENTREGUE") return QBrush(Qt::green);

      if (dias >= 3 or status == "CANCELADO" or status == "DEVOLVIDO" or status == "PROCESSADO") {
        return QBrush(Qt::yellow);
      }

      if (dias < 3) return QBrush(Qt::red);
    }

    if (proxyIndex.column() == this->followup) {
      const int semaforo = QIdentityProxyModel::data(index(proxyIndex.row(), this->semaforo), Qt::DisplayRole).toInt();

      if (semaforo == Quente) return QBrush(QColor(255, 66, 66));
      if (semaforo == Morno) return QBrush(QColor(255, 170, 0));
      if (semaforo == Frio) return QBrush(QColor(70, 113, 255));
    }

    if (proxyIndex.column() == this->financeiro) {
      const QString financeiro = QIdentityProxyModel::data(index(proxyIndex.row(), this->financeiro), Qt::DisplayRole).toString();

      if (financeiro == "PENDENTE") return QBrush(Qt::red);
      if (financeiro == "CONFERIDO") return QBrush(Qt::yellow);
      if (financeiro == "LIBERADO") return QBrush(Qt::green);
    }

    const QString status = QIdentityProxyModel::data(index(proxyIndex.row(), this->status), Qt::DisplayRole).toString();
    if (status == "ENTREGUE") return QBrush(Qt::green);
    if (status == "CANCELADO" or status == "DEVOLVIDO" or status == "PROCESSADO") return QBrush(Qt::yellow);
    if (status == "PERDIDO") return QBrush(Qt::yellow);
  }

  if (role == Qt::ForegroundRole) {
    const QString status = QIdentityProxyModel::data(index(proxyIndex.row(), this->status), Qt::DisplayRole).toString();

    if (status == "ENTREGUE" or status == "DEVOLVIDO" or status == "PROCESSADO" or status == "CANCELADO" or proxyIndex.column() == this->dias) {
      return QBrush(Qt::black);
    }
  }

  return QIdentityProxyModel::data(proxyIndex, role);
}
