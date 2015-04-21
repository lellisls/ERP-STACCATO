#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QtWidgets>
#include <QtConcurrent/QtConcurrent>

#include "importabd.h"
#include "ui_importabd.h"
#include "cadastrocliente.h"
#include "importaexportproxy.h"

ImportaBD::ImportaBD(QWidget *parent) : QDialog(parent), ui(new Ui::ImportaBD) {
  ui->setupUi(this);
  setWindowFlags(Qt::Window);

  showMaximized();

  progressDialog = new QProgressDialog(this);
  progressDialog->setCancelButton(0);
  progressDialog->setLabelText("Importando...");
  progressDialog->setWindowTitle("ERP Staccato");
  progressDialog->setWindowModality(Qt::WindowModal);
  progressDialog->setMinimum(0);
  progressDialog->setMaximum(0);
  progressDialog->setCancelButtonText("Cancelar");
  connect(&futureWatcher, &QFutureWatcherBase::finished, progressDialog, &QProgressDialog::cancel);
  connect(&futureWatcher, &QFutureWatcherBase::finished, this, &ImportaBD::mostraResultado);
  connect(progressDialog, &QProgressDialog::canceled, &importaExport, &ImportaExport::cancel);
  connect(&importaExport, &ImportaExport::progressRangeChanged, this, &ImportaBD::updateProgressRange);
  connect(&importaExport, &ImportaExport::progressValueChanged, this, &ImportaBD::updateProgressValue);
  connect(&importaExport, &ImportaExport::progressTextChanged, this, &ImportaBD::updateProgressText);

  QString file = QFileDialog::getOpenFileName(this, "Importar tabela genérica", QDir::currentPath(),
                                              tr("Excel (*.xlsx)"));
  if (file.isEmpty()) {
    return;
  }
  int validade = QInputDialog::getInt(this, "Validade", "Insira a validade em dias: ");

  QFuture<QString> future = QtConcurrent::run(&importaExport, &ImportaExport::importar, file, validade);
  futureWatcher.setFuture(future);
  progressDialog->exec();

  model.setTable("Produto");
  model.setSort(model.fieldIndex("expirado"), Qt::AscendingOrder);
  //  model.setHeaderData();
  model.select();

  ImportaExportProxy *proxyModel = new ImportaExportProxy(model.fieldIndex("expirado"));
  proxyModel->setSourceModel(&model);
  ui->tableView->setModel(proxyModel);
  for (int i = 1; i < model.columnCount(); i += 2) {
    ui->tableView->setColumnHidden(i, true);
  }
  ui->tableView->setColumnHidden(model.fieldIndex("idProduto"), true);
  ui->tableView->setColumnHidden(model.fieldIndex("idFornecedor"), true);
}

ImportaBD::~ImportaBD() { delete ui; }

void ImportaBD::mostraResultado() {
  QMessageBox::information(this, "Aviso", futureWatcher.result(), QMessageBox::Ok);
}

void ImportaBD::updateProgressRange(int max) { progressDialog->setMaximum(max); }

void ImportaBD::updateProgressValue(int val) {
  progressDialog->setValue(val);
}

void ImportaBD::updateProgressText(QString str) { progressDialog->setLabelText(str); }
