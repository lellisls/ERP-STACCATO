#ifndef WIDGETCOMPRAFATURAR_H
#define WIDGETCOMPRAFATURAR_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetCompraFaturar;
}

class WidgetCompraFaturar : public QWidget {
  Q_OBJECT

public:
  explicit WidgetCompraFaturar(QWidget *parent = 0);
  ~WidgetCompraFaturar();
  bool updateTables();

signals:
  void errorSignal(QString error);

private slots:
  void on_checkBoxRepresentacao_toggled(bool checked);
  void on_pushButtonMarcarFaturado_clicked();
  void on_table_entered(const QModelIndex &);

private:
  // attributes
  Ui::WidgetCompraFaturar *ui;
  SqlTableModel model;
  // methods
  void setupTables();
  bool faturarCompra();
};

#endif // WIDGETCOMPRAFATURAR_H
