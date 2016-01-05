#ifndef WIDGETCOMPRA_H
#define WIDGETCOMPRA_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
  class WidgetCompra;
}

class WidgetCompra : public QWidget {
    Q_OBJECT

  public:
    explicit WidgetCompra(QWidget *parent = 0);
    ~WidgetCompra();
    QString updateTables();

  private slots:
    void montaFiltro();
    void on_groupBoxStatusPendentes_toggled(const bool &enabled);
    void on_tableProdutosPend_activated(const QModelIndex &index);
    void on_tabWidget_currentChanged(const int &);

  private:
    // attributes
    Ui::WidgetCompra *ui;
    SqlTableModel modelProdPend;
    // methods
    void setupTables();
};

#endif // WIDGETCOMPRA_H
