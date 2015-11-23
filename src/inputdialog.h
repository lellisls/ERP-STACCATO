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
    void on_pushButtonSalvar_clicked();
    void on_pushButtonCancelar_clicked();
    void on_dateEditEvento_dateChanged(const QDate &date);

  private:
    // attributes
    Ui::InputDialog *ui;
    SqlTableModel model;
    // methods
    void setupTables();
};

#endif // INPUTDIALOG_H
