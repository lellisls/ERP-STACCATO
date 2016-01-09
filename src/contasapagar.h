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
    void viewConta(const QString &idVenda);

  private slots:
    void on_pushButtonSalvar_clicked();

  private:
    Ui::ContasAPagar *ui;
    QString idVenda;
    SqlTableModel modelContas;
    SqlTableModel modelItensContas;
};

#endif // CONTASAPAGAR_H
