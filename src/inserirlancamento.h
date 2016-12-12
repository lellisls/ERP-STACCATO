#ifndef INSERIRLANCAMENTO_H
#define INSERIRLANCAMENTO_H

#include <QDialog>

#include "sqltablemodel.h"

namespace Ui {
class InserirLancamento;
}

class InserirLancamento : public QDialog {
  Q_OBJECT

public:
  enum Tipo { Pagar, Receber };
  explicit InserirLancamento(const Tipo tipo, QWidget *parent = 0);
  ~InserirLancamento();

private slots:
  void on_pushButtonCriarLancamento_clicked();
  void on_pushButtonSalvar_clicked();

private:
  // attributes
  const Tipo tipo;
  SqlTableModel model;
  Ui::InserirLancamento *ui;
  // methods
  void setupTables();
  bool verifyFields();
};

#endif // INSERIRLANCAMENTO_H
