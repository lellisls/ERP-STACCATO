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
  void on_dateFollowup_dateChanged(const QDate &date);
  void on_pushButtonCancelar_clicked();
  void on_pushButtonSalvar_clicked();

private:
  // attributes
  const QString id;
  const Tipo tipo;
  SqlTableModel model;
  Ui::FollowUp *ui;
  // methods
  bool verifyFields();
  void setupTables();
};

#endif // FOLLOWUP_H
