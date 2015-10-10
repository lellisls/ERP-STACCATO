#ifndef SENDMAIL_H
#define SENDMAIL_H

#include <QDialog>

namespace Ui {
  class SendMail;
}

class SendMail : public QDialog {
    Q_OBJECT

  public:
    explicit SendMail(QWidget *parent = 0, QString text = QString(), QString arquivo = QString());
    ~SendMail();

  private slots:
    void on_pushButtonBuscar_clicked();
    void on_pushButtonCancelar_clicked();
    void on_pushButtonEnviar_clicked();
    void mailSent(QString status);

  private:
    // attributes
    Ui::SendMail *ui;
    QStringList files;
};

#endif // SENDMAIL_H
