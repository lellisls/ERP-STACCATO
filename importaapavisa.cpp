#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include "importaapavisa.h"
#include "cadastrocliente.h"

ImportaApavisa::ImportaApavisa() {}

ImportaApavisa::~ImportaApavisa() {}

int ImportaApavisa::buscarCadastrarFornecedor(QString column0) {
  int idFornecedor = 0;

  QSqlQuery queryFornecedor;
  if (!queryFornecedor.exec("SELECT * FROM Fornecedor WHERE nome = '" + column0 + "'")) {
    qDebug() << "Erro buscando fornecedor: " << queryFornecedor.lastError();
  }
  qDebug() << "size: " << queryFornecedor.size();
  if (queryFornecedor.next()) {
    idFornecedor = queryFornecedor.value("idFornecedor").toInt();
  } else {
    QSqlQuery cadastrar;
    if (!cadastrar.exec("INSERT INTO Fornecedor (nome) VALUES (" + column0 + "')")) {
      qDebug() << "Erro cadastrando fornecedor: " << cadastrar.lastError();
    }
  }
  if (!queryFornecedor.exec("SELECT * FROM Fornecedor WHERE nome = '" + column0 + "'")) {
    qDebug() << "Erro buscando fornecedor: " << queryFornecedor.lastError();
  }
  qDebug() << "size: " << queryFornecedor.size();
  if (queryFornecedor.next()) {
    idFornecedor = queryFornecedor.value("idFornecedor").toInt();
  }

  return idFornecedor;
}

QString ImportaApavisa::importar(QString file) {
  QString texto;

  QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", "Excel Connection");
  db.setDatabaseName("DRIVER={Microsoft Excel Driver (*.xls)};DBQ=" + file);

  if (db.open()) {
    int imported = 0;
    int duplicate = 0;

    QSqlQuery query("SELECT * FROM [Base Apavisa$]", db); // Select range, place A1:B5 after $

    while (query.next()) {
      QString column0 = query.value(21).toString(); // fornecedor - string
      if ((!column0.isEmpty()) and (column0 != "Marca")) {
        int idFornecedor = buscarCadastrarFornecedor(column0);
        qDebug() << "id: " << idFornecedor;

        QString column1 = query.value(22).toString();  // codigo - int
        QString column2 = query.value(23).toString();  // colecao - string
        QString column3 = query.value(26).toString();  // formato com. - string
        QString column4 = query.value(28).toString();  // formato fabr - string
        QString column5 = query.value(30).toString();  // descricao - string
        QString column6 = query.value(32).toString();  // pccx - int
        QString column7 = query.value(33).toString();  // m2cx - double
        QString column8 = query.value(34).toString();  // cx_pallet - int
        QString column9 = query.value(35).toString();  // m2_pallet - double
        QString column10 = query.value(36).toString(); // kg_cx - double
        QString column11 = query.value(37).toString(); // m2_pc - double
        QString column12 = query.value(38).toString(); // kg_pallet - int
        QString column13 = query.value(39).toString(); // g-tarifa - string
        QString column14 = query.value(40).toString(); // preco_m2 - double
        QString column15 = query.value(41).toString(); // preco_pc - double
        QString column16 = query.value(42).toString(); // ncm - int
        QString column17 = query.value(43).toString(); // custo liquido - double
        QString column18 = query.value(44).toString(); // unidade - string
        QString column19 = query.value(45).toString(); // preco - double

        //        qDebug() << column0 << " " << column1 << " " << column2 << " " << column3 << " " << column4
        //        << " "
        //                 << column5 << " " << column6 << " " << column7;
        //        qDebug() << "---------------";
        //        qDebug() << "marca: " << column0;

        QString column20 = "0";
        if (column0 == "Apavisa") {
          column20 = "5";
        }
        if (column0 == "Aparici") {
          column20 = "42";
        }
        //        qDebug() << "codigo: " << column20;

        QSqlQuery qry;
        qry.prepare(
              "INSERT INTO mydb.Produto "
              "(idFornecedor, fornecedor, colecao, formComercial, descricao, codComercial, "
              "pccx, m2cx, qtdPallet, un, ncm, "
              "precoVenda, custo, markup, ui) VALUES (:idFornecedor, :fornecedor, :colecao, :formComercial, "
              ":descricao, :codComercial, :pccx, :m2cx, :qtdPallet, :un, :ncm, :precoVenda, :custo, "
              ":markup, :ui)");
        qry.bindValue(":idFornecedor", idFornecedor);
        qry.bindValue(":fornecedor", column0);
        qry.bindValue(":colecao", column2);
        qry.bindValue(":formComercial", column3);
        qry.bindValue(":descricao", column5);
        qry.bindValue(":codComercial", column1);
        qry.bindValue(":pccx", column6);
        qry.bindValue(":m2cx", column7);
        qry.bindValue(":qtdPallet", column9);
        qry.bindValue(":un", column18);
        qry.bindValue(":ncm", column16);
        qry.bindValue(":precoVenda", column19);
        qry.bindValue(":custo", column17);
        qry.bindValue(":markup", 95);
        qry.bindValue(":ui", column20);

        if (!qry.exec()) {
          qDebug() << "error? " << qry.lastError();
          //          qDebug() << "qry: " << qry.lastQuery();
        }
        //                qDebug() << "qry: " << qry.lastQuery();
        //        qDebug() << "error? " << qry.lastError();
        if (qry.lastError().nativeErrorCode() == "1062") {
          duplicate++;
        }
        if (qry.lastError().nativeErrorCode() == "") {
          imported++;
        }
        qry.finish();
      }
    }

    query.finish();

    texto = "Produtos importados: " + QString::number(imported) + "\nProdutos duplicados: " +
            QString::number(duplicate);
    //    QMessageBox::information(this, "Aviso", texto, QMessageBox::Ok);

  } else {
    //    QMessageBox::information(this, "Aviso", "Importação falhou!", QMessageBox::Ok);
    texto = "Importação falhou!";
    qDebug() << "db failed :(";
    qDebug() << db.lastError();
  }
  db.close();
  QSqlDatabase::removeDatabase(db.connectionName());

  return texto;
}
