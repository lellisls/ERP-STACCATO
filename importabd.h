#ifndef IMPORTABD_H
#define IMPORTABD_H

#include <QDialog>
#include <QFutureWatcher>
#include <QProgressDialog>

#include "importaexport.h"

namespace Ui {
  class ImportaBD;
}

class ImportaBD : public QDialog {
    Q_OBJECT

  public:
    explicit ImportaBD(QWidget *parent = 0);
    ~ImportaBD();

  public slots:
    void mostraResultado();
    void updateProgressRange(int max);
    void updateProgressValue(int val);
    void updateProgressText(QString str);

  private slots:
    void on_pushButtonExport_clicked();

  private:
    // attributes
    Ui::ImportaBD *ui;
    QFutureWatcher<QString> futureWatcher;
    QProgressDialog *progressDialog;
    ImportaExport importaExport;
};

#endif // IMPORTABD_H
