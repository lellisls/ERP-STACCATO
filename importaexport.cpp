#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDate>

#include "importaexport.h"

ImportaExport::ImportaExport() {}

ImportaExport::~ImportaExport() {}

QString ImportaExport::importar(QString file) {
  QString texto;

  QSqlQuery("BEGIN TRANSACTION").exec();

  QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", "Excel Connection");
  db.setDatabaseName("DRIVER={Microsoft Excel Driver (*.xls, *.xlsx, *.xlsm, *.xlsb)};DBQ=" + file);

  if (db.open()) {
    int imported = 0;
    int notChanged = 0;
    int updated = 0;

    QSqlQuery query("SELECT * FROM [" + QString("DE_PARA$]"), db);

    while (query.next()) {
      QString id = query.value(0).toString();
      if (!id.isEmpty()) {
        QString nome = query.value(1).toString();
        qDebug() << "id: " << id << " - " << nome;
        int idFornecedor = buscarCadastrarFornecedor(id, nome);
      }
    }
  } else {
    texto = "Importação falhou!";
    qDebug() << "db failed :(";
    qDebug() << db.lastError();
  }
  db.close();
  QSqlDatabase::removeDatabase(db.connectionName());

  return texto;
}

int ImportaExport::buscarCadastrarFornecedor(QString id, QString fornecedor) {
  int idFornecedor = 0;

  QSqlQuery queryFornecedor;
  if (!queryFornecedor.exec("SELECT * FROM Fornecedor WHERE razaoSocial = '" + fornecedor + "'")) {
    qDebug() << "Erro buscando fornecedor: " << queryFornecedor.lastError();
  }
  if (queryFornecedor.next()) {
    idFornecedor = queryFornecedor.value("idFornecedor").toInt();
  } else {
    QSqlQuery cadastrar;
    if (!cadastrar.exec("INSERT INTO Fornecedor (idFornecedor, razaoSocial) VALUES (" + id + ", '" +
                        fornecedor + "')")) {
      qDebug() << "Erro cadastrando fornecedor: " << cadastrar.lastError();
    }
  }
  if (!queryFornecedor.exec("SELECT * FROM Fornecedor WHERE razaoSocial = '" + fornecedor + "'")) {
    qDebug() << "Erro buscando fornecedor: " << queryFornecedor.lastError();
  }
  if (queryFornecedor.next()) {
    idFornecedor = queryFornecedor.value("idFornecedor").toInt();
  }

  return idFornecedor;
}
