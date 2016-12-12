#ifndef WIDGETLOGISTICACAMINHAO_H
#define WIDGETLOGISTICACAMINHAO_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetLogisticaCaminhao;
}

class WidgetLogisticaCaminhao : public QWidget {
  Q_OBJECT

public:
  explicit WidgetLogisticaCaminhao(QWidget *parent = 0);
  ~WidgetLogisticaCaminhao();
  bool updateTables();

signals:
  void errorSignal(const QString &error);

private slots:
  void on_table_clicked(const QModelIndex &index);
  void on_table_entered(const QModelIndex &);

private:
  // attributes
  SqlTableModel modelCaminhao;
  SqlTableModel modelCarga;
  Ui::WidgetLogisticaCaminhao *ui;
  // methods
  void setupTables();
};

#endif // WIDGETLOGISTICACAMINHAO_H
