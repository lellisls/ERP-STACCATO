#ifndef ESTOQUE_H
#define ESTOQUE_H

#include <QDialog>

#include "sqltablemodel.h"

namespace Ui {
  class Estoque;
}

class Estoque : public QDialog {
    Q_OBJECT

  public:
    explicit Estoque(QWidget *parent = 0);
    ~Estoque();
    void viewRegisterById(const QString codComercial);

  private slots:
    void on_tableEstoque_activated(const QModelIndex &index);
    void on_pushButtonExibirNfe_clicked();

  private:
    Ui::Estoque *ui;
    SqlTableModel modelEstoque;
};

#endif // ESTOQUE_H
