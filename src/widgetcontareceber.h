#ifndef WIDGETCONTARECEBER_H
#define WIDGETCONTARECEBER_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetContaReceber;
}

class WidgetContaReceber : public QWidget {
  Q_OBJECT

public:
  explicit WidgetContaReceber(QWidget *parent = 0);
  ~WidgetContaReceber();
  bool updateTables();

signals:
  void errorSignal(QString error);

private slots:
  void on_table_entered(const QModelIndex &);
  void on_table_activated(const QModelIndex &index);
  void on_radioButtonContaReceberRecebido_toggled(bool checked);
  void on_radioButtonContaReceberPendente_toggled(bool checked);
  void on_radioButtonContaReceberLimpar_toggled(bool checked);

private:
  // attributes
  Ui::WidgetContaReceber *ui;
  SqlTableModel model;
  // methods
  void setupTables();
};

#endif // WIDGETCONTARECEBER_H
