#ifndef BAIXAORCAMENTO_H
#define BAIXAORCAMENTO_H

#include <QDialog>

#include "sqltablemodel.h"

namespace Ui {
class BaixaOrcamento;
}

class BaixaOrcamento : public QDialog {
  Q_OBJECT

public:
  explicit BaixaOrcamento(QString idOrcamento, QWidget *parent = 0);
  ~BaixaOrcamento();

private slots:
  void on_pushButtonCancelar_clicked();
  void on_pushButtonSalvar_clicked();

private:
  Ui::BaixaOrcamento *ui;
  SqlTableModel model;
};

#endif // BAIXAORCAMENTO_H
