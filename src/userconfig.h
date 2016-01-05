#ifndef USERCONFIG_H
#define USERCONFIG_H

#include <QDialog>

namespace Ui {
  class UserConfig;
}

class UserConfig : public QDialog {
    Q_OBJECT

  public:
    explicit UserConfig(QWidget *parent = 0);
    ~UserConfig();

  private slots:
    void on_pushButtonUserFolder_clicked();
    void on_pushButtonSalvar_clicked();
    void on_pushButtonCancelar_clicked();
    void on_pushButtonACBrEntrada_clicked();
    void on_pushButtonACBrSaida_clicked();
    void on_pushButtonACBrXML_clicked();

    void on_pushButtonUserFolder_2_clicked();

  private:
    Ui::UserConfig *ui;
    QVariant settings(const QString &key) const;
    void setSettings(const QString &key, const QVariant &value) const;
};

#endif // USERCONFIG_H
