#ifndef WIDGETCOMPRAFATURAR_H
#define WIDGETCOMPRAFATURAR_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
  class WidgetCompraFaturar;
}

class WidgetCompraFaturar : public QWidget {
    Q_OBJECT

  public:
    explicit WidgetCompraFaturar(QWidget *parent = 0);
    ~WidgetCompraFaturar();
    void tableFornCompras_activated(const QString &fornecedor);
    QString updateTables();

  private slots:
    void on_pushButtonMarcarFaturado_clicked();
    void on_table_entered(const QModelIndex &);

  private:
    // attributes
    Ui::WidgetCompraFaturar *ui;
    SqlTableModel model;
    // methods
    void setupTables();
};

#endif // WIDGETCOMPRAFATURAR_H
