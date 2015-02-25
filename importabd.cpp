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
  int idFornecedor = 0;

  QSqlQuery queryFornecedor("SELECT * FROM Cadastro WHERE nome = 'Portinari'");
  if(!queryFornecedor.exec()){
    qDebug() << "Erro buscando fornecedor: " << queryFornecedor.lastError();
  }
  qDebug() << "size: " << queryFornecedor.size();
  if(queryFornecedor.next()){
  idFornecedor = queryFornecedor.value("idCadastro").toInt();
  } else{
    CadastroCliente *cad = new CadastroCliente();
    cad->setTipo("PJ");
    if(cad->exec() != QDialog::Accepted){
      qDebug() << "Sem cadastro para realizar importação!";
      return;
    }
  }
  if(!queryFornecedor.exec("SELECT * FROM Cadastro WHERE nome = 'Portinari'")){
    qDebug() << "Erro buscando fornecedor: " << queryFornecedor.lastError();
  }
  qDebug() << "size: " << queryFornecedor.size();
  if(queryFornecedor.next()){
  idFornecedor = queryFornecedor.value("idCadastro").toInt();
  }
  qDebug() << "id: " << idFornecedor;

  QString file = QFileDialog::getOpenFileName(this, "Importar", QDir::currentPath(), tr("Excel (*.xls)"));
  if(file.isEmpty()){
    return;
  }

  QFuture<QString> future = QtConcurrent::run(&this->portinari, &ImportaPortinari::importar, file, idFornecedor);
  futureWatcher.setFuture(future);

  progressDialog->setMinimum(0);
  progressDialog->setMaximum(0);
  progressDialog->setWindowModality(Qt::WindowModal);
  progressDialog->exec();
}

void ImportaBD::on_pushButtonApavisa_clicked() {
  int idFornecedor = 0;
  
  QSqlQuery queryFornecedor("SELECT * FROM Cadastro WHERE nome = 'Apavisa'");
  if(!queryFornecedor.exec()){
    qDebug() << "Erro buscando fornecedor: " << queryFornecedor.lastError();
  }
  qDebug() << "size: " << queryFornecedor.size();
  if(queryFornecedor.next()){
  idFornecedor = queryFornecedor.value("idCadastro").toInt();
  } else{
    CadastroCliente *cad = new CadastroCliente();
    cad->setTipo("PJ");
    if(cad->exec() != QDialog::Accepted){
      qDebug() << "Sem cadastro para realizar importação!";
      return;
    }
  }
  if(!queryFornecedor.exec("SELECT * FROM Cadastro WHERE nome = 'Apavisa'")){
    qDebug() << "Erro buscando fornecedor: " << queryFornecedor.lastError();
  }
  qDebug() << "size: " << queryFornecedor.size();
  if(queryFornecedor.next()){
  idFornecedor = queryFornecedor.value("idCadastro").toInt();
  }
  qDebug() << "id: " << idFornecedor;
  
  QString file = QFileDialog::getOpenFileName(this, "Importar", QDir::currentPath(), tr("Excel (*.xls)"));
  if(file.isEmpty()){
    return;
  }

  QFuture<QString> future = QtConcurrent::run(&this->apavisa, &ImportaApavisa::importar, file, idFornecedor);
  futureWatcher.setFuture(future);

  progressDialog->setMinimum(0);
  progressDialog->setMaximum(0);
  progressDialog->setWindowModality(Qt::WindowModal);
  progressDialog->exec();
}
