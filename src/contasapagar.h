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
    void on_table_entered(const QModelIndex &);

  private:
    // attributes
    Ui::ContasAPagar *ui;
    QString idVenda;
    SqlTableModel model;
    // methods
    void setupTables();
};

#endif // CONTASAPAGAR_H
