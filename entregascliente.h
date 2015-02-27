#ifndef ENTREGASCLIENTE_H
#define ENTREGASCLIENTE_H

#include <QDialog>
#include <QSqlTableModel>

namespace Ui {
  class EntregasCliente;
}

class EntregasCliente : public QDialog {
    Q_OBJECT

  public:
    explicit EntregasCliente(QWidget *parent = 0);
    ~EntregasCliente();
    void viewEntrega(QString idPedido);

  private slots:
    void on_checkBoxEntregue_clicked();
    void on_pushButtonCancelar_clicked();
    void on_pushButtonSalvar_clicked();

  private:
    //attributes
    Ui::EntregasCliente *ui;
    QSqlTableModel modelEntregas;
    QString idPedido;
};

#endif // ENTREGASCLIENTE_H
