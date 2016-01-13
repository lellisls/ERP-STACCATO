#ifndef WIDGETESTOQUE_H
#define WIDGETESTOQUE_H

#include "sqltablemodel.h"

#include <QWidget>

namespace Ui {
  class WidgetEstoque;
}

class WidgetEstoque : public QWidget {
    Q_OBJECT

  public:
    explicit WidgetEstoque(QWidget *parent = 0);
    ~WidgetEstoque();
    QString updateTables();

  private slots:
    void on_table_activated(const QModelIndex &index);
    void on_pushButtonEntradaEstoque_clicked();
    void on_pushButtonTesteFaturamento_clicked();

  private:
    // attributes
    Ui::WidgetEstoque *ui;
    SqlTableModel model;
    // methods
    void setupTables();
};

#endif // WIDGETESTOQUE_H
