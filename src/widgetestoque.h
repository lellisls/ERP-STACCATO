#ifndef WIDGETESTOQUE_H
#define WIDGETESTOQUE_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetEstoque;
}

class WidgetEstoque : public QWidget {
  Q_OBJECT

public:
  explicit WidgetEstoque(QWidget *parent = 0);
  ~WidgetEstoque();
  bool updateTables();

signals:
  void errorSignal(const QString &error);

private slots:
  void on_lineEditBusca_textChanged(const QString &text);
  void on_radioButtonMaior_toggled(const bool checked);
  void on_table_activated(const QModelIndex &index);
  void on_table_entered(const QModelIndex &);

private:
  // attributes
  SqlTableModel model;
  Ui::WidgetEstoque *ui;
  // methods
  void setupTables();
};

#endif // WIDGETESTOQUE_H
