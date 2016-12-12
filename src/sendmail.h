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
  explicit SendMail(QWidget *parent = 0, const QString &arquivo = QString(), const QString &fornecedor = QString());
  ~SendMail();

private slots:
  void on_pushButtonBuscar_clicked();
  void on_pushButtonEnviar_clicked();
  void mailSent(const QString &status);

private:
  // attributes
  const QString fornecedor;
  QProgressDialog *progress;
  QStringList files;
  Ui::SendMail *ui;
  // methods
  void failureStatus(const QString &status);
  void successStatus();
};

#endif // SENDMAIL_H
