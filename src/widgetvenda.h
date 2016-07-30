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
  bool updateTables();

signals:
  void errorSignal(QString error);

private slots:
  void montaFiltro();
  void on_comboBoxLojas_currentIndexChanged(int);
  void on_groupBoxStatus_toggled(const bool &enabled);
  void on_radioButtonProprios_toggled(bool checked);
  void on_table_activated(const QModelIndex &index);
  void on_table_entered(const QModelIndex &);

private:
  // attributes
  Ui::WidgetVenda *ui;
  SqlTableModel model;
  // methods
  void setupTables();
  void setPermissions();
  void makeConnections();
};

#endif // WIDGETVENDA_H
