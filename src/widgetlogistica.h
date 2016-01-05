#ifndef WIDGETLOGISTICA_H
#define WIDGETLOGISTICA_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
  class WidgetLogistica;
}

class WidgetLogistica : public QWidget {
    Q_OBJECT

  public:
    explicit WidgetLogistica(QWidget *parent = 0);
    ~WidgetLogistica();
    QString updateTables();

  private slots:
    void on_tableFornLogistica_activated(const QModelIndex &index);
    void on_tabWidgetLogistica_currentChanged(const int &);

  private:
    // attributes
    Ui::WidgetLogistica *ui;
    SqlTableModel modelPedForn;
    // methods
    void setupTables();
};

#endif // WIDGETLOGISTICA_H
