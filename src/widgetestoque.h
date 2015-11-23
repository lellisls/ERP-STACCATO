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
    bool updateTables();

  private slots:
    void on_tableEstoque_activated(const QModelIndex &index);
    void on_pushButtonEntradaEstoque_clicked();
    void on_pushButtonTesteFaturamento_clicked();

  private:
    Ui::WidgetEstoque *ui;
    SqlTableModel *modelEstoque;
    void setupTables();
};

#endif // WIDGETESTOQUE_H
