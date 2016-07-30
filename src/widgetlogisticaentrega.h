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

private slots:
  void on_lineEditBuscaEntregas_textChanged(const QString &text);
  void on_radioButtonEntregaEnviado_clicked();
  void on_radioButtonEntregaLimpar_clicked();
  void on_radioButtonEntregaPendente_clicked();
  void on_table_activated(const QModelIndex &index);
  void on_table_entered(const QModelIndex &);
  void on_table_clicked(const QModelIndex &index);

signals:
  void errorSignal(QString error);

private:
  // attributes
  Ui::WidgetLogisticaEntrega *ui;
  SqlTableModel model;
  SqlTableModel model2;
  // methods
  void setupTables();
};

#endif // WIDGETLOGISTICAENTREGA_H
