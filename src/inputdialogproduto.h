#ifndef INPUTDIALOGPRODUTO_H
#define INPUTDIALOGPRODUTO_H

#include <QDialog>

#include "sqltablemodel.h"

namespace Ui {
class InputDialogProduto;
}

class InputDialogProduto : public QDialog {
  Q_OBJECT

public:
  enum Type { GerarCompra, Faturamento };

  explicit InputDialogProduto(const Type &type, QWidget *parent = 0);
  ~InputDialogProduto();
  QDateTime getDate() const;
  QDateTime getNextDate() const;
  bool setFilter(const QStringList &ids);

private slots:
  void on_comboBoxST_currentTextChanged(const QString &);
  void on_dateEditEvento_dateChanged(const QDate &date);
  void on_doubleSpinBoxAliquota_valueChanged(double);
  void on_doubleSpinBoxST_valueChanged(double value);
  void on_pushButtonSalvar_clicked();
  void on_table_entered(const QModelIndex &);

private:
  // attributes
  bool isBlockedAliquota = false;
  const Type type;
  SqlTableModel model;
  Ui::InputDialogProduto *ui;
  // methods
  bool cadastrar();
  void calcularTotal();
  void processST();
  void setupTables();
  void updateTableData(const QModelIndex &topLeft);
};

#endif // INPUTDIALOGPRODUTO_H
