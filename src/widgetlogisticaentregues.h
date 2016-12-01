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

private:
  // attributes
  SqlTableModel modelProdutos;
  SqlTableModel modelVendas;
  Ui::WidgetLogisticaEntregues *ui;
  // methods
  void setupTables();
  void montaFiltro();
};

#endif // WIDGETLOGISTICAENTREGUES_H
