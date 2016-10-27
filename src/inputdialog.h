#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QDialog>

#include "sqltablemodel.h"

namespace Ui {
class InputDialog;
}

class InputDialog : public QDialog {
  Q_OBJECT

public:
  enum Type {
    Carrinho,
    GerarCompra,
    ConfirmarCompra,
    Faturamento,
    AgendarColeta,
    Coleta,
    AgendarRecebimento,
    Recebimento,
    AgendarEntrega,
    Entrega,
    Financeiro
  } type;

  explicit InputDialog(const Type &type, QWidget *parent = 0);
  ~InputDialog();
  QDate getDate();
  QDate getNextDate();
  bool setFilter(const QString &id);
  bool setFilter(const QStringList &id);

private slots:
  void calcularTotal();
  void on_checkBoxMarcarTodos_toggled(bool checked);
  void on_comboBoxPgt1_currentTextChanged(const QString &text);
  void on_comboBoxPgt2_currentTextChanged(const QString &text);
  void on_comboBoxPgt3_currentTextChanged(const QString &text);
  void on_dateEditEvento_dateChanged(const QDate &date);
  void on_doubleSpinBoxAdicionais_valueChanged(double);
  void on_doubleSpinBoxFrete_valueChanged(double);
  void on_doubleSpinBoxPgt1_valueChanged(double);
  void on_doubleSpinBoxPgt2_valueChanged(double);
  void on_doubleSpinBoxPgt3_valueChanged(double);
  void on_pushButtonCorrigirFluxo_clicked();
  void on_pushButtonLimparPag_clicked();
  void on_pushButtonSalvar_clicked();
  void on_table_entered(const QModelIndex &);
  void on_tableFluxoCaixa_entered(const QModelIndex &);

private:
  // attributes
  SqlTableModel model;
  SqlTableModel modelFluxoCaixa;
  Ui::InputDialog *ui;
  // methods
  bool cadastrar();
  bool verifyFields();
  void calculoSpinBox1() const;
  void calculoSpinBox2() const;
  void montarFluxoCaixa();
  void resetarPagamentos();
  void setupTables();
  void updateTableData(const QModelIndex &topLeft);
};

#endif // INPUTDIALOG_H
