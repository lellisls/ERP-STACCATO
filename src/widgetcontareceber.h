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
    QString updateTables();

  private:
    // attributes
    Ui::WidgetContaReceber *ui;
    SqlTableModel model;
    // methods
    void setupTables();
};

#endif // WIDGETCONTARECEBER_H
