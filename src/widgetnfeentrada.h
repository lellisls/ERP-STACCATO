#ifndef WIDGETNFEENTRADA_H
#define WIDGETNFEENTRADA_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
  class WidgetNfeEntrada;
}

class WidgetNfeEntrada : public QWidget {
    Q_OBJECT

  public:
    explicit WidgetNfeEntrada(QWidget *parent = 0);
    ~WidgetNfeEntrada();
    QString updateTables();

  private slots:
    void on_table_activated(const QModelIndex &index);

  private:
    Ui::WidgetNfeEntrada *ui;
    SqlTableModel model;
    void setupTables();
};

#endif // WIDGETNFEENTRADA_H
