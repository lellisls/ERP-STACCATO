#ifndef WIDGETCOMPRA_H
#define WIDGETCOMPRA_H

#include "sqltablemodel.h"

#include <QWidget>

namespace Ui {
  class WidgetCompra;
}

class WidgetCompra : public QWidget {
    Q_OBJECT

  public:
    explicit WidgetCompra(QWidget *parent = 0);
    ~WidgetCompra();
    bool updateTables();

  private slots:
    void on_tableFornCompras_activated(const QModelIndex &index);
    void on_pushButtonConfirmarCompra_clicked();
    void on_radioButtonProdPendTodos_clicked();
    void on_radioButtonProdPendPend_clicked();
    void on_radioButtonProdPendEmCompra_clicked();
    void on_pushButtonMarcarFaturado_clicked();
    void on_tableProdutosPend_activated(const QModelIndex &index);
    void on_pushButtonGerarCompra_clicked();
    void on_pushButtonTodosFornCompras_clicked();

    void on_tabWidgetCompra_currentChanged(int);

    void on_pushButtonTesteEmail_clicked();

  private:
    Ui::WidgetCompra *ui;
    SqlTableModel modelProdPend;
    SqlTableModel modelPedForn;
    SqlTableModel modelItemPedidosPend;
    SqlTableModel modelItemPedidosComp;
    SqlTableModel modelFat;
    void setupTables();
    QVariant settings(const QString &key) const;
    void setSettings(const QString &key, const QVariant &value) const;
};

#endif // WIDGETCOMPRA_H
