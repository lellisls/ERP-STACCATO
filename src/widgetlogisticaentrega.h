#ifndef WIDGETLOGISTICAENTREGA_H
#define WIDGETLOGISTICAENTREGA_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetLogisticaEntrega;
}

class WidgetLogisticaEntrega : public QWidget {
  Q_OBJECT

public:
  explicit WidgetLogisticaEntrega(QWidget *parent = 0);
  ~WidgetLogisticaEntrega();
  bool updateTables();

signals:
  void errorSignal(QString error);

private slots:
  void on_pushButtonAgendar_clicked();
  void on_tableProdutos_entered(const QModelIndex &);
  void on_tableVendas_clicked(const QModelIndex &index);
  void on_tableVendas_entered(const QModelIndex &);

private:
  // attributes
  SqlTableModel modelProdutos;
  SqlTableModel modelVendas;
  Ui::WidgetLogisticaEntrega *ui;
  // methods
  bool processRows(QModelIndexList list);
  void montaFiltro();
  void setupTables();
};

#endif // WIDGETLOGISTICAENTREGA_H
