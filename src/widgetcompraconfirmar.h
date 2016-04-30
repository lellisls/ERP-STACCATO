#ifndef WIDGETCOMPRACONFIRMAR_H
#define WIDGETCOMPRACONFIRMAR_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
  class WidgetCompraConfirmar;
}

class WidgetCompraConfirmar : public QWidget {
    Q_OBJECT

  public:
    explicit WidgetCompraConfirmar(QWidget *parent = 0);
    ~WidgetCompraConfirmar();
    bool updateTables(QString &error);

  private slots:
    void on_pushButtonConfirmarCompra_clicked();
    void on_table_entered(const QModelIndex &);

  private:
    // attributes
    Ui::WidgetCompraConfirmar *ui;
    SqlTableModel model;
    // methods
    void setupTables();
};

#endif // WIDGETCOMPRACONFIRMAR_H
