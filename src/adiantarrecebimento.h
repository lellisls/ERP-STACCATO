#ifndef ADIANTARRECEBIMENTO_H
#define ADIANTARRECEBIMENTO_H

#include <QDialog>

#include "sqltablemodel.h"

namespace Ui {
class AdiantarRecebimento;
}

class AdiantarRecebimento : public QDialog {
  Q_OBJECT

public:
  explicit AdiantarRecebimento(QWidget *parent = 0);
  ~AdiantarRecebimento();

private slots:
  void on_table_entered(const QModelIndex &);

private:
  // attributes
  SqlTableModel model;
  Ui::AdiantarRecebimento *ui;
  // methods
  void calcularTotais();
  void setupTables();
};

#endif // ADIANTARRECEBIMENTO_H
