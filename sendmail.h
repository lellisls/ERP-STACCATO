#ifndef SENDMAIL_H
#define SENDMAIL_H

#include <QDialog>

namespace Ui {
  class SendMail;
}

class SendMail : public QDialog {
    Q_OBJECT

  public:
    explicit SendMail(QWidget *parent = 0);
    ~SendMail();

  private slots:
    void on_pushButtonBuscar_clicked();
    void on_pushButtonCancelar_clicked();
    void on_pushButtonEnviar_clicked();

  private:
    //attributes
    Ui::SendMail *ui;
};

#endif // SENDMAIL_H
