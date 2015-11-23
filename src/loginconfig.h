#ifndef LOGINCONFIG_H
#define LOGINCONFIG_H

#include <QDialog>

namespace Ui {
  class LoginConfig;
}

class LoginConfig : public QDialog {
    Q_OBJECT

  public:
    explicit LoginConfig(QWidget *parent = 0);
    ~LoginConfig();

  private slots:
    void on_pushButtonSalvar_clicked();
    void on_pushButtonCancelar_clicked();

  private:
    Ui::LoginConfig *ui;
    QVariant settings(const QString &key) const;
    void setSettings(const QString &key, const QVariant &value) const;};

#endif // LOGINCONFIG_H
