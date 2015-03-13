#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDate>

#include "importaportinari.h"
#include "cadastrocliente.h"

ImportaPortinari::ImportaPortinari() {}

ImportaPortinari::~ImportaPortinari() {}

int ImportaPortinari::buscarCadastrarFornecedor(QString column0) {
  int idFornecedor = 0;

  QSqlQuery queryFornecedor;
  if (!queryFornecedor.exec("SELECT * FROM Fornecedor WHERE razaoSocial = '" + column0 + "'")) {
    qDebug() << "Erro buscando fornecedor: " << queryFornecedor.lastError();
  }
  //  qDebug() << "size: " << queryFornecedor.size();

  if (queryFornecedor.next()) {
    idFornecedor = queryFornecedor.value("idFornecedor").toInt();
  } else {
    QSqlQuery cadastrar;
    if (!cadastrar.exec("INSERT INTO Fornecedor (razaoSocial) VALUES ('" + column0 + "')")) {
      qDebug() << "Erro cadastrando fornecedor: " << cadastrar.lastError();
    }
  }
  if (!queryFornecedor.exec("SELECT * FROM Fornecedor WHERE razaoSocial = '" + column0 + "'")) {
    qDebug() << "Erro buscando fornecedor: " << queryFornecedor.lastError();
  }
  //  qDebug() << "size: " << queryFornecedor.size();
  if (queryFornecedor.next()) {
    idFornecedor = queryFornecedor.value("idFornecedor").toInt();
  }

  return idFornecedor;
}

QString ImportaPortinari::importar(QString file) {
  QString texto;

  QSqlQuery("BEGIN TRANSACTION").exec();

  QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", "Excel Connection");
  db.setDatabaseName("DRIVER={Microsoft Excel Driver (*.xls, *.xlsx, *.xlsm, *.xlsb)};DBQ=" + file);

  if (db.open()) {
    int imported = 0;
    int notChanged = 0;
    int updated = 0;

    QSqlQuery query("SELECT * FROM [" + QString("Base Portinari$]"), db); // Select range, place A1:B5 after $

    while (query.next()) {
      QString column0 = query.value(0).toString(); // fornecedor - string

      if ((!column0.isEmpty()) and (column0 != "MARCA")) {
        int idFornecedor = buscarCadastrarFornecedor(column0);
        //        qDebug() << "id: " << idFornecedor;

        QString column1 = query.value(1).toString();   // lancamento - string
        QString column2 = query.value(2).toString();   // colecao - string
        QString column3 = query.value(3).toString();   // tipo2 - string
        QString column4 = query.value(4).toString();   // tipo compl - string
        QString column5 = query.value(5).toString();   // fmt com - string
        QString column6 = query.value(6).toString();   // fmt fabr - string
        QString column7 = query.value(7).toString();   // ret/bold - string
        QString column8 = query.value(8).toString();   // produto - string
        QString column9 = query.value(9).toString();   // codigo - int
        QString column10 = query.value(10).toString(); // ui - int

        int column10int = 0;
        //        qDebug() << "co10: " << column10;
        if (column10.toInt() != 0) {
          column10int = column10.toInt();
        }
        //        qDebug() << "conversion: " << column10int;
        column10 = column10.number(column10int);

        QString column11 = query.value(11).toString(); // status - string
        QString column12 = query.value(12).toString(); // pccx - int
        QString column13 = query.value(13).toString(); // m2cx - int
        QString column14 = query.value(14).toString(); // kgcx - int
        QString column15 = query.value(15).toString(); // pei_classe uso - string
        QString column16 = query.value(16).toString(); // coef atrito - string
        QString column17 = query.value(17).toString(); // esp - int
        QString column18 = query.value(18).toString(); // ipi - int
        QString column19 = query.value(19).toString(); // m2pallet - int
        QString column20 = query.value(20).toString(); // un - string
        QString column21 = query.value(21).toString(); // * - string
        QString column22 = query.value(22).toString(); // icms4 - int
        QString column23 = query.value(23).toString(); // icms12 - int
        QString column24 = query.value(24).toString(); // clasFiscal - int
        QString column25 = query.value(25).toString(); // codBarras - int
        QString column26 = query.value(26).toString(); // venda - int
        QString column27 = query.value(27).toString(); // custo - int
        QString column28 = query.value(28).toString(); // margem - int

        QSqlQuery produto;
        if (!produto.exec("SELECT * FROM Produto WHERE codComercial = '" + column9 + "'")) {
          qDebug() << "Erro buscando produto: " << produto.lastError();
        }

        if (produto.next()) {
          int idProduto = produto.value("idProduto").toInt();

          // if price is equal just continue (maybe extend validity)
          if (produto.value("precoVenda").toString() == column26) {
            notChanged++;
            continue;
          }
          // if price is different insert into produto_has_preco
          if (!produto.exec("INSERT INTO Produto_has_Preco (idProduto, preco, validade) VALUES (" +
                            QString::number(idProduto) + ", " + column26 + ", '" +
                            QDate::currentDate().toString("yyyy-MM-dd") + "')")) {
            qDebug() << "Erro inserindo em Preço: " << produto.lastError();
            qDebug() << "qry: " << produto.lastQuery();
          }
          if (!produto.exec("UPDATE Produto SET precoVenda = " + column26 + " WHERE idProduto = " +
                            QString::number(idProduto) + "")) {
            qDebug() << "idProduto: " << idProduto;
            qDebug() << "Erro atualizando preço de produto: " << produto.lastError();
            qDebug() << "qry upd: " << produto.lastQuery();
          }

          updated++;
          continue;
        }

        //        qDebug() << column0 << " " << column1 << " " << column2 << " " << column3 << " " << column4
        //        << " "
        //                 << column5 << " " << column6 << " " << column7;
        //        qDebug() << "---------------";

        QSqlQuery qry;
        qry.prepare(
            "INSERT INTO mydb.Produto "
            "(idFornecedor, fornecedor, colecao, tipo, formComercial, descricao, codComercial, "
            "UI, pccx, m2cx, ipi, qtdPallet, un, ncm, "
            "codBarras, precoVenda, custo, markup) VALUES (:idFornecedor, :fornecedor, :colecao, :tipo, "
            ":formComercial, :descricao, :codComercial, :UI, :pccx, :m2cx, :ipi, :qtdPallet, :un, "
            ":ncm, :codBarras, :precoVenda, :custo, :markup)");
        qry.bindValue(":idFornecedor", idFornecedor);
        qry.bindValue(":fornecedor", column0);
        qry.bindValue(":colecao", column2);
        qry.bindValue(":tipo", column3);
        qry.bindValue(":formComercial", column5);
        qry.bindValue(":descricao", column8);
        qry.bindValue(":codComercial", column9);
        qry.bindValue(":UI", column10);
        qry.bindValue(":pccx", column12);
        qry.bindValue(":m2cx", column13);
        qry.bindValue(":ipi", column18);
        qry.bindValue(":qtdPallet", column19);
        qry.bindValue(":un", column20);
        qry.bindValue(":ncm", column24);
        qry.bindValue(":codBarras", column25);
        qry.bindValue(":precoVenda", column26);
        qry.bindValue(":custo", column27);
        qry.bindValue(":markup", column28);

        if (!qry.exec()) {
          //        qDebug() << "qry: " << qry.lastQuery();
          qDebug() << "error? " << qry.lastError();
        }
        //        qDebug() << "error? " << qry.lastError();
        //        qDebug() << "1: " << qry.lastError().databaseText();
        //        qDebug() << "2: " << qry.lastError().driverText();
        //        qDebug() << "3: " << qry.lastError().text();
        //        qDebug() << "4: " << qry.lastError().nativeErrorCode();
        //        qDebug() << "5: " << qry.lastError().number();
        //        qDebug() << "qry: " << qry.lastQuery();
        //        if (qry.lastError().nativeErrorCode() == "1062") {
        //          duplicate++;
        //        }
        if (qry.lastError().nativeErrorCode() == "") {
          imported++;
        }
      }
    }

    QSqlQuery("COMMIT").exec();

    texto = "Produtos importados: " + QString::number(imported) + "\nProdutos atualizados: " +
            QString::number(updated) + "\nNão modificados: " + QString::number(notChanged);
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
