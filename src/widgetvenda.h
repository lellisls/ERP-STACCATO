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
  void setFinanceiro();

signals:
  void errorSignal(const QString &error);

private slots:
  void on_comboBoxLojas_currentIndexChanged(const int);
  void on_groupBoxStatus_toggled(const bool enabled);
  void on_groupBoxStatusFinanceiro_toggled(const bool enabled);
  void on_pushButtonFollowup_clicked();
  void on_radioButtonProprios_toggled(const bool checked);
  void on_table_activated(const QModelIndex &index);
  void on_table_entered(const QModelIndex &);

private:
  // attributes
  bool financeiro = false;
  SqlTableModel model;
  Ui::WidgetVenda *ui;
  // methods
  void montaFiltro();
  void makeConnections();
  void setPermissions();
  void setupTables();
};

#endif // WIDGETVENDA_H
