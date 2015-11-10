#ifndef CONTASAPAGAR_H
#define CONTASAPAGAR_H

#include <QDialog>

#include "sqltablemodel.h"

namespace Ui {
  class ContasAPagar;
}

class ContasAPagar : public QDialog {
    Q_OBJECT

  public:
    explicit ContasAPagar(QWidget *parent = 0);
    ~ContasAPagar();
    void viewConta(const QString idVenda);

  private slots:
    void on_pushButtonCancelar_clicked();
    void on_pushButtonSalvar_clicked();

  private:
    // atributes
    Ui::ContasAPagar *ui;
    SqlTableModel modelItensContas;
    SqlTableModel modelContas;
    QString idVenda;
};

#endif // CONTASAPAGAR_H
