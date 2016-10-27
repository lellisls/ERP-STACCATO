#ifndef FOLLOWUP_H
#define FOLLOWUP_H

#include <QDialog>

#include "sqltablemodel.h"

namespace Ui {
class FollowUp;
}

class FollowUp : public QDialog {
  Q_OBJECT

public:
  enum Tipo { Orcamento, Venda };
  explicit FollowUp(const QString &id, const Tipo tipo, QWidget *parent = 0);
  ~FollowUp();

private slots:
  void on_pushButtonCancelar_clicked();
  void on_pushButtonSalvar_clicked();

private:
  // attributes
  int row;
  QString id;
  SqlTableModel model;
  Tipo tipo;
  Ui::FollowUp *ui;
  // methods
  bool savingProcedures();
  bool verifyFields();
  void setupTables();
};

#endif // FOLLOWUP_H
