#ifndef WIDGETLOGISTICAENTREGUES_H
#define WIDGETLOGISTICAENTREGUES_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetLogisticaEntregues;
}

class WidgetLogisticaEntregues : public QWidget {
  Q_OBJECT

public:
  explicit WidgetLogisticaEntregues(QWidget *parent = 0);
  ~WidgetLogisticaEntregues();
  bool updateTables();

signals:
  void errorSignal(const QString &error);

private slots:
  void on_tableVendas_clicked(const QModelIndex &index);
  void on_pushButtonCancelar_clicked();

private:
  // attributes
  SqlTableModel modelProdutos;
  SqlTableModel modelVendas;
  QString error;
  Ui::WidgetLogisticaEntregues *ui;
  // methods
  void setupTables();
  void montaFiltro();
  bool cancelar(const QModelIndexList &list);
};

#endif // WIDGETLOGISTICAENTREGUES_H
