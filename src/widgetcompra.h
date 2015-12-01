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
    QString updateTables();

  private slots:
    void montaFiltro();
    void on_groupBoxStatusPendentes_toggled(const bool &enabled);
    void on_pushButtonConfirmarCompra_clicked();
    void on_pushButtonGerarCompra_clicked();
    void on_pushButtonMarcarFaturado_clicked();
    void on_pushButtonTesteEmail_clicked();
    void on_pushButtonTodosFornCompras_clicked();
    void on_tableFornCompras_activated(const QModelIndex &index);
    void on_tableProdutosPend_activated(const QModelIndex &index);
    void on_tabWidgetCompra_currentChanged(const int &);

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
