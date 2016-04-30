#ifndef WIDGETCONTAPAGAR_H
#define WIDGETCONTAPAGAR_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
  class WidgetContaPagar;
}

class WidgetContaPagar : public QWidget {
    Q_OBJECT

  public:
    explicit WidgetContaPagar(QWidget *parent = 0);
    ~WidgetContaPagar();
    bool updateTables(QString &error);

  private slots:
    void on_table_entered(const QModelIndex &);

  private:
    // attributes
    Ui::WidgetContaPagar *ui;
    SqlTableModel model;
    // methods
    void setupTables();
};

#endif // WIDGETCONTAPAGAR_H
