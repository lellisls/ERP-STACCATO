#ifndef IMPORTABD_H
#define IMPORTABD_H

#include <QDialog>
#include <QFutureWatcher>
#include <QProgressDialog>

#include "importaportinari.h"
#include "importaapavisa.h"

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

  private slots:
    void on_pushButtonApavisa_clicked();
    void on_pushButtonPortinari_clicked();

  private:
    //attributes
    Ui::ImportaBD *ui;
    QFutureWatcher<QString> futureWatcher;
    QProgressDialog *progressDialog;
    ImportaPortinari portinari;
    ImportaApavisa apavisa;

};

#endif // IMPORTABD_H
