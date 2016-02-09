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
    void viewRegisterById(const QString &codComercial);

  private slots:
    void on_table_activated(const QModelIndex &index);
    void on_pushButtonExibirNfe_clicked();

  private:
    // attributes
    Ui::Estoque *ui;
    SqlTableModel model;
    // methods
    void setupTables();
    void criarConsumo();
};

#endif // ESTOQUE_H
