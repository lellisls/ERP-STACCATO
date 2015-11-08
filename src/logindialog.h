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
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_pushButtonConfig_clicked();

  private:
    // attributes
    Ui::LoginDialog *ui;
    // methods
    void verify();
    bool dbConnect();
    QVariant settings(QString key) const;
    void setSettings(QString key, QVariant value) const;
    bool settingsContains(const QString &key) const;
};

#endif // LOGINDIALOG_H
