#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QtWidgets>
#include <QtConcurrent/QtConcurrent>

#include "importabd.h"
#include "ui_importabd.h"
#include "cadastrocliente.h"

ImportaBD::ImportaBD(QWidget *parent) : QDialog(parent), ui(new Ui::ImportaBD) {
  ui->setupUi(this);

  progressDialog = new QProgressDialog(this);
  progressDialog->setCancelButton(0);
  progressDialog->setLabelText("Importando...");
  progressDialog->setWindowTitle("ERP Staccato");

  connect(&futureWatcher, &QFutureWatcherBase::finished, progressDialog, &QProgressDialog::cancel);
  connect(&futureWatcher, &QFutureWatcherBase::finished, this, &ImportaBD::mostraResultado);

  show();
}

ImportaBD::~ImportaBD() { delete ui; }

void ImportaBD::mostraResultado() {
  QMessageBox::information(this, "Aviso", futureWatcher.result(), QMessageBox::Ok);
}

void ImportaBD::on_pushButtonPortinari_clicked() {
  QString file = QFileDialog::getOpenFileName(this, "Importar", QDir::currentPath(), tr("Excel (*.xls)"));
  if(file.isEmpty()){
    return;
  }

  QFuture<QString> future = QtConcurrent::run(&this->portinari, &ImportaPortinari::importar, file);
  futureWatcher.setFuture(future);

  progressDialog->setMinimum(0);
  progressDialog->setMaximum(0);
  progressDialog->setWindowModality(Qt::WindowModal);
  progressDialog->exec();
}

void ImportaBD::on_pushButtonApavisa_clicked() {  
  QString file = QFileDialog::getOpenFileName(this, "Importar", QDir::currentPath(), tr("Excel (*.xls)"));
  if(file.isEmpty()){
    return;
  }

  QFuture<QString> future = QtConcurrent::run(&this->apavisa, &ImportaApavisa::importar, file);
  futureWatcher.setFuture(future);

  progressDialog->setMinimum(0);
  progressDialog->setMaximum(0);
  progressDialog->setWindowModality(Qt::WindowModal);
  progressDialog->exec();
}

void ImportaBD::on_pushButtonExport_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, "Importar", QDir::currentPath(), tr("Excel (*.xlsx)"));
    if(file.isEmpty()){
      return;
    }

    QFuture<QString> future = QtConcurrent::run(&this->importaExport, &ImportaExport::importar, file);
    futureWatcher.setFuture(future);

    progressDialog->setMinimum(0);
    progressDialog->setMaximum(0);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->exec();
}
