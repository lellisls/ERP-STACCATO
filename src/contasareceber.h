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
  void on_pushButtonSalvar_clicked();
  void on_table_entered(const QModelIndex &);

private:
  // attributes
  Ui::ContasAReceber *ui;
  SqlTableModel model;
  QString idVenda;
  // methods
  void setupTables();
};

#endif // CONTASARECEBER_H
