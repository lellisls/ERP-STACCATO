#ifndef WIDGETVENDA_H
#define WIDGETVENDA_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetVenda;
}

class WidgetVenda : public QWidget {
  Q_OBJECT

public:
  explicit WidgetVenda(QWidget *parent = 0);
  ~WidgetVenda();
  bool updateTables(QString &error);

private slots:
  void montaFiltro();
  void on_groupBoxStatus_toggled(const bool &enabled);
  void on_table_activated(const QModelIndex &index);
  void on_table_entered(const QModelIndex &);
  void on_radioButtonProprios_toggled(bool checked);

private:
  // attributes
  Ui::WidgetVenda *ui;
  SqlTableModel model;
  // methods
  void setupTables();
};

#endif // WIDGETVENDA_H
