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
    QString updateTables();

  private slots:
    void on_pushButtonConfirmarCompra_clicked();

  private:
    // attributes
    Ui::WidgetCompraConfirmar *ui;
    SqlTableModel model;
    // methods
    void setupTables();
};

#endif // WIDGETCOMPRACONFIRMAR_H