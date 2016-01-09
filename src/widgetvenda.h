#ifndef WIDGETVENDA_H
#define WIDGETVENDA_H

#include "sqltablemodel.h"

#include <QWidget>

namespace Ui {
  class WidgetVenda;
}

class WidgetVenda : public QWidget {
    Q_OBJECT

  public:
    explicit WidgetVenda(QWidget *parent = 0);
    ~WidgetVenda();
    QString updateTables();

  private slots:
    void montaFiltro();
    void on_groupBoxStatus_toggled(const bool &enabled);
    void on_table_activated(const QModelIndex &index);

  private:
    // attributes
    Ui::WidgetVenda *ui;
    SqlTableModel model;
    // methods
    void setupTables();
};

#endif // WIDGETVENDA_H
