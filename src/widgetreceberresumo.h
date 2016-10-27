#ifndef WIDGETRECEBERRESUMO_H
#define WIDGETRECEBERRESUMO_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetReceberResumo;
}

class WidgetReceberResumo : public QWidget {
  Q_OBJECT

public:
  explicit WidgetReceberResumo(QWidget *parent = 0);
  ~WidgetReceberResumo();
  bool updateTables();

private slots:
  void on_tableVencidos_entered(const QModelIndex &);
  void on_tableVencer_entered(const QModelIndex &);

private:
  // attributes
  Ui::WidgetReceberResumo *ui;
  SqlTableModel modelVencidos;
  SqlTableModel modelVencer;
  // methods
  void setupTables();
};

#endif // WIDGETRECEBERRESUMO_H
