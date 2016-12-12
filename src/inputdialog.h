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
  enum Type { Carrinho, AgendarColeta, Coleta, AgendarRecebimento, AgendarEntrega };

  explicit InputDialog(const Type &type, QWidget *parent = 0);
  ~InputDialog();
  QDateTime getDate() const;
  QDateTime getNextDate() const;

private slots:
  void on_dateEditEvento_dateChanged(const QDate &date);
  void on_pushButtonSalvar_clicked();

private:
  // attributes
  const Type type;
  Ui::InputDialog *ui;
  // methods
};

#endif // INPUTDIALOG_H
