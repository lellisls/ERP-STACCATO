#ifndef CONTASARECEBER_H
#define CONTASARECEBER_H

#include <QDialog>

#include "sqltablemodel.h"

namespace Ui {
  class ContasAReceber;
}

class ContasAReceber : public QDialog {
    Q_OBJECT

  public:
    explicit ContasAReceber(QWidget *parent = 0);
    ~ContasAReceber();
    void viewConta(const QString &idVenda);

  private slots:
    void on_pushButtonCancelar_clicked();
    void on_pushButtonSalvar_clicked();

  private:
    Ui::ContasAReceber *ui;
    SqlTableModel modelContas;
    QString idVenda;
};

#endif // CONTASARECEBER_H
