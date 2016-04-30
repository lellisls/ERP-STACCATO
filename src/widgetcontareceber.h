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
    bool updateTables(QString &error);

  private slots:
    void on_table_entered(const QModelIndex &);

  private:
    // attributes
    Ui::WidgetContaReceber *ui;
    SqlTableModel model;
    // methods
    void setupTables();
};

#endif // WIDGETCONTARECEBER_H
