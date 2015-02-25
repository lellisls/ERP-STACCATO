#ifndef RECEBIMENTOSFORNECEDOR_H
#define RECEBIMENTOSFORNECEDOR_H

#include <QDialog>
#include <QSqlTableModel>

namespace Ui {
  class RecebimentosFornecedor;
}

class RecebimentosFornecedor : public QDialog {
    Q_OBJECT

  public:
    explicit RecebimentosFornecedor(QWidget *parent = 0);
    ~RecebimentosFornecedor();
    void viewRecebimento(QString idPedido);

  private slots:
    void on_pushButtonCancelar_clicked();
    void on_pushButtonSalvar_clicked();

  private:
    //attributes
    Ui::RecebimentosFornecedor *ui;
    QSqlTableModel modelRecebimentos;
    QString idPedido;
};

#endif // RECEBIMENTOSFORNECEDOR_H
