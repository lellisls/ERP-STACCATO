#ifndef WIDGETCOMPRAGERAR_H
#define WIDGETCOMPRAGERAR_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
  class WidgetCompraGerar;
}

class WidgetCompraGerar : public QWidget {
    Q_OBJECT

  public:
    explicit WidgetCompraGerar(QWidget *parent = 0);
    ~WidgetCompraGerar();
    QString updateTables();

  private slots:
    void fixPersistente();
    void on_checkBoxMarcarTodos_clicked(const bool &checked);
    void on_pushButtonGerarCompra_clicked();
    void on_tableForn_activated(const QModelIndex &index);

  private:
    // attributes
    Ui::WidgetCompraGerar *ui;
    SqlTableModel modelProdutos;
    SqlTableModel modelForn;
    // methods
    void setupTables();
};

#endif // WIDGETCOMPRAGERAR_H
