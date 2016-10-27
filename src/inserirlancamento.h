#ifndef INSERIRLANCAMENTO_H
#define INSERIRLANCAMENTO_H

#include <QDialog>

#include <src/sqltablemodel.h>

namespace Ui {
class InserirLancamento;
}

class InserirLancamento : public QDialog {
  Q_OBJECT

public:
  enum Tipo { Pagar, Receber };
  explicit InserirLancamento(Tipo tipo, QWidget *parent = 0);
  ~InserirLancamento();

private slots:
  void on_pushButtonCriarLancamento_clicked();
  void on_pushButtonSalvar_clicked();

private:
  Ui::InserirLancamento *ui;
  Tipo tipo;
  SqlTableModel model;
  //
  void setupTables();
  bool verifyFields();
};

#endif // INSERIRLANCAMENTO_H
