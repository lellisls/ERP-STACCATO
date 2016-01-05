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
    QString gerarExcel(QList<int> lista);
    void on_checkBoxTodosGerar_clicked(const bool &checked);
    void on_pushButtonGerarCompra_clicked();
    void on_tableFornCompras_activated(const QModelIndex &index);

  private:
    // attributes
    Ui::WidgetCompraGerar *ui;
    SqlTableModel modelItemPedidosPend;
    SqlTableModel modelPedForn;
    // methods
    QVariant settings(const QString &key) const;
    void setSettings(const QString &key, const QVariant &value) const;
    void setupTables();
};

#endif // WIDGETCOMPRAGERAR_H
