#ifndef PAGAMENTOSDIA_H
#define PAGAMENTOSDIA_H

#include <QDialog>

#include "sqltablemodel.h"

namespace Ui {
class PagamentosDia;
}

class PagamentosDia : public QDialog {
  Q_OBJECT

public:
  explicit PagamentosDia(QWidget *parent = 0);
  ~PagamentosDia();
  bool setFilter(const QDate &date, const QString &idConta);

private:
  SqlTableModel model;
  Ui::PagamentosDia *ui;
  void setupTables();
};

#endif // PAGAMENTOSDIA_H
