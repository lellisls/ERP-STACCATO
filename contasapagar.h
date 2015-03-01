#ifndef CONTASAPAGAR_H
#define CONTASAPAGAR_H

#include <QDataWidgetMapper>
#include <QDialog>
#include <QSqlRelationalTableModel>

namespace Ui {
  class ContasAPagar;
}

class ContasAPagar : public QDialog {
    Q_OBJECT

  public:
    explicit ContasAPagar(QWidget *parent = 0);
    ~ContasAPagar();
    void viewConta(QString idVenda);

  private slots:
    void on_checkBoxPago_toggled(bool checked);
    void on_pushButtonCancelar_clicked();
    void on_pushButtonSalvar_clicked();

  private:
    //atributes
    Ui::ContasAPagar *ui;
    QSqlRelationalTableModel modelItensContas, modelContas;
    QString idVenda;
};

#endif // CONTASAPAGAR_H
