#ifndef VALIDADEDIALOG_H
#define VALIDADEDIALOG_H

#include <QDialog>

namespace Ui {
class ValidadeDialog;
}

class ValidadeDialog : public QDialog {
  Q_OBJECT

public:
  explicit ValidadeDialog(QWidget *parent = 0);
  ~ValidadeDialog();
  int getValidade();

private slots:
  void on_pushButtonSalvar_clicked();
  void on_spinBox_valueChanged(const int dias);
  void on_dateEdit_dateChanged(const QDate &date);

private:
  Ui::ValidadeDialog *ui;
};

#endif // VALIDADEDIALOG_H
