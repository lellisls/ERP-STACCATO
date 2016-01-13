#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

namespace Ui {
  class LoginDialog;
}

class LoginDialog : public QDialog {
    Q_OBJECT

  public:
    explicit LoginDialog(QWidget *parent = 0);
    ~LoginDialog();

  private slots:
    void on_pushButtonConfig_clicked();
    void on_pushButtonLogin_clicked();

  private:
    // attributes
    Ui::LoginDialog *ui;
    // methods
    bool dbConnect();
};

#endif // LOGINDIALOG_H
