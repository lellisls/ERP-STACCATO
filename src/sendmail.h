#ifndef SENDMAIL_H
#define SENDMAIL_H

#include <QDialog>
#include <QProgressDialog>

namespace Ui {
  class SendMail;
}

class SendMail : public QDialog {
    Q_OBJECT

  public:
    explicit SendMail(QWidget *parent = 0, const QString &text = QString(), const QString &arquivo = QString());
    ~SendMail();

  private slots:
    void on_pushButtonBuscar_clicked();
    void on_pushButtonEnviar_clicked();
    void mailSent(const QString &status);

  private:
    // attributes
    Ui::SendMail *ui;
    QProgressDialog *progress;
    QStringList files;
    // methods
    void successStatus();
    void failureStatus(const QString &status);
};

#endif // SENDMAIL_H
