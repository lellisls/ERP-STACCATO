#ifndef PEDIDOSCOMPRA_H
#define PEDIDOSCOMPRA_H

#include <QDialog>
#include <QSqlRelationalTableModel>

namespace Ui {
  class PedidosCompra;
}

class PedidosCompra : public QDialog {
    Q_OBJECT

  public:
    explicit PedidosCompra(QWidget *parent = 0);
    ~PedidosCompra();
    void viewPedido(const QString idPedido);

  private slots:
    void on_checkBox_toggled(const bool checked);
    void on_pushButtonCancelar_clicked();
    void on_pushButtonNFe_clicked();
    void on_pushButtonSalvar_clicked();
    void on_radioButtonVenda_toggled(const bool checked);

  private:
    // attributes
    Ui::PedidosCompra *ui;
    QSqlRelationalTableModel modelItemPedidos, modelPedidos;
    QString idPedido;
    // methods
    void setupTables();
};

#endif // PEDIDOSCOMPRA_H
