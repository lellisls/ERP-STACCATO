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
    enum Type { Carrinho, GerarCompra, ConfirmarCompra, Faturamento, Coleta, Recebimento, Entrega } type;

    explicit InputDialog(const Type &type, QWidget *parent = 0);
    ~InputDialog();
    QDate getDate();
    QDate getNextDate();
    void setFilter(const QStringList &ids);
    void setFilter(const QString &id);

  private slots:
    void on_comboBoxPgt1_currentTextChanged(const QString &text);
    void on_comboBoxPgt2_currentTextChanged(const QString &text);
    void on_comboBoxPgt3_currentTextChanged(const QString &text);
    void on_dateEditEvento_dateChanged(const QDate &date);
    void on_doubleSpinBoxFrete_editingFinished();
    void on_doubleSpinBoxPgt1_editingFinished();
    void on_doubleSpinBoxPgt2_editingFinished();
    void on_doubleSpinBoxPgt3_editingFinished();
    void on_pushButtonLimparPag_clicked();
    void on_pushButtonSalvar_clicked();
    void on_table_entered(const QModelIndex &);
    void on_tableFluxoCaixa_entered(const QModelIndex &);

  private:
    // attributes
    Ui::InputDialog *ui;
    SqlTableModel model;
    SqlTableModel modelFluxoCaixa;
    // methods
    void setupTables();
    void calculoSpinBox1() const;
    void calculoSpinBox2() const;
    void montarFluxoCaixa();
    void resetarPagamentos();
};

#endif // INPUTDIALOG_H
