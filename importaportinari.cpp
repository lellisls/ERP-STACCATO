#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDate>

#include "importaportinari.h"
#include "cadastrocliente.h"

void ImportaPortinari::cancel() {
  canceled = true;
}

ImportaPortinari::ImportaPortinari() {}

ImportaPortinari::~ImportaPortinari() {}

QString ImportaPortinari::importar(QString file, int validade) {
  canceled = false;

  QLocale locale(QLocale::Portuguese);
  QString texto;
  QMap<QString, int> map;

  emit progressTextChanged("Conectando...");
  emit progressRangeChanged(0);

  QSqlQuery("SET AUTOCOMMIT=0").exec();
  QSqlQuery("START TRANSACTION").exec();

  QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", "Excel Connection");
  db.setDatabaseName("DRIVER={Microsoft Excel Driver (*.xls, *.xlsx, *.xlsm, *.xlsb)};DBQ=" + file);

  if (db.open()) {
    emit progressTextChanged("Importando...");

    int imported = 0;
    int notChanged = 0;
    int updated = 0;

    // Cadastra fornecedores
    QSqlQuery queryForn("SELECT * FROM [Base Portinari$]", db);

    while (queryForn.next()) {
      QString fornecedor = queryForn.value(0).toString();
      if (!fornecedor.isEmpty() and (fornecedor != "MARCA")) {
        int id = buscarCadastrarFornecedor(fornecedor);
        qDebug() << "id: " << id << " - " << fornecedor;
        map.insert(fornecedor, id);
      }
    }

    // Lê produtos do excel
    QSqlQuery querySz("SELECT COUNT(*) FROM [Base Portinari$]", db);
    querySz.first();
//    qDebug() << "SIZE: " << querySz.value(0);
    emit progressRangeChanged(querySz.value(0).toInt());

    QSqlQuery queryProd("SELECT * FROM [Base Portinari$]", db);
    int current = 0;
    while (queryProd.next()) {
      emit progressValueChanged(current++);

      if (canceled) {
        emit progressFinished();
        return ("Operação cancelada!");
        QSqlQuery("ROLLBACK").exec();
      }

      QString fornecedor = queryProd.value(0).toString();

      if ((!fornecedor.isEmpty()) and (fornecedor != "MARCA")) {
        //        int idFornecedor = buscarCadastrarFornecedor(fornecedor);
        //        qDebug() << "id: " << idFornecedor;

        QString lancamento = queryProd.value(1).toString();
        QString colecao = queryProd.value(2).toString();
        QString tipo2 = queryProd.value(3).toString();
        QString tipoComp = queryProd.value(4).toString();
        QString formCom = queryProd.value(5).toString();
        QString formFab = queryProd.value(6).toString();
        QString retBold = queryProd.value(7).toString();
        QString descricao = queryProd.value(8).toString();
        QString codigo = queryProd.value(9).toString();
        QString uniInd = queryProd.value(10).toString();

        int uniInd_int = 0;
        //        qDebug() << "co10: " << column10;
        if (uniInd.toInt() != 0) {
          uniInd_int = uniInd.toInt();
        }
        //        qDebug() << "conversion: " << column10int;
        uniInd = uniInd.number(uniInd_int);

        QString status = queryProd.value(11).toString();
        QString pccx = queryProd.value(12).toString();
        QString m2cx = queryProd.value(13).toString();
        QString kgcx = queryProd.value(14).toString();
        QString peiClasse = queryProd.value(15).toString();
        QString coefAtr = queryProd.value(16).toString();
        QString esp = queryProd.value(17).toString();
        QString ipi = queryProd.value(18).toString();
        QString m2Pallet = queryProd.value(19).toString();
        QString unidade = queryProd.value(20).toString();
        //        QString column21 = query.value(21).toString(); // * - string
        QString icms4 = queryProd.value(22).toString();
        QString icms12 = queryProd.value(23).toString();
        QString clasFiscal = queryProd.value(24).toString();
        QString codBarras = queryProd.value(25).toString();
        QString venda = queryProd.value(26).toString();
        QString custo = queryProd.value(27).toString();
        QString margem = queryProd.value(28).toString();

        // Consistência dos dados

        // Verifica se produto já se encontra no BD
        QSqlQuery produto;
        if (!produto.exec("SELECT * FROM Produto WHERE fornecedor = '" + fornecedor +
                          "' AND codComercial = '" + codigo + "'")) {
          qDebug() << "Erro buscando produto: " << produto.lastError();
        }

        if (produto.next()) {
          QString idProduto = produto.value("idProduto").toString();

          // Se o preço for igual extender a validade
          if (produto.value("precoVenda").toString() == venda) {
            if (!produto.exec("UPDATE Produto_has_Preco SET validadeFim = '" +
                              QDate::currentDate().addDays(validade).toString("yyyy-MM-dd") +
                              "' WHERE idProduto = " + produto.value("idProduto").toString() + "")) {
              qDebug() << "Erro atualizando validade do preço: " << produto.lastError();
            }
            notChanged++;
            continue;
          }

          // Guarda novo preço do produto e altera sua validade
          if (!produto.exec(
                "INSERT INTO Produto_has_Preco (idProduto, preco, validadeInicio, validadeFim) VALUES (" +
                idProduto + ", " + venda + ", '" + QDate::currentDate().toString("yyyy-MM-dd") + "', '" +
                QDate::currentDate().addDays(validade).toString("yyyy-MM-dd") + "')")) {
            qDebug() << "Erro inserindo em Preço: " << produto.lastError();
            qDebug() << "qry: " << produto.lastQuery();
          }
          if (!produto.exec("UPDATE Produto SET precoVenda = " + venda + " WHERE idProduto = " + idProduto +
                            "")) {
            qDebug() << "Erro atualizando preço de produto: " << produto.lastError();
            qDebug() << "qry upd: " << produto.lastQuery();
          }

          updated++;
          continue;
        }

        QSqlQuery qry;
        qry.prepare(
              "INSERT INTO mydb.Produto "
              "(idFornecedor, fornecedor, colecao, tipo, formComercial, descricao, codComercial, "
              "UI, pccx, m2cx, ipi, qtdPallet, un, ncm, "
              "codBarras, precoVenda, custo, markup) VALUES (:idFornecedor, :fornecedor, :colecao, :tipo, "
              ":formComercial, :descricao, :codComercial, :UI, :pccx, :m2cx, :ipi, :qtdPallet, :un, "
              ":ncm, :codBarras, :precoVenda, :custo, :markup)");
        qry.bindValue(":idFornecedor", map.value(fornecedor));
        qry.bindValue(":fornecedor", fornecedor);
        qry.bindValue(":colecao", colecao);
        qry.bindValue(":tipo", tipo2);
        qry.bindValue(":formComercial", formCom);
        qry.bindValue(":descricao", descricao);
        qry.bindValue(":codComercial", codigo);
        qry.bindValue(":UI", uniInd);
        qry.bindValue(":pccx", pccx);
        qry.bindValue(":m2cx", m2cx);
        qry.bindValue(":ipi", ipi);
        qry.bindValue(":qtdPallet", m2Pallet);
        qry.bindValue(":un", unidade);
        qry.bindValue(":ncm", clasFiscal);
        qry.bindValue(":codBarras", codBarras);
        qry.bindValue(":precoVenda", venda);
        qry.bindValue(":custo", custo);
        qry.bindValue(":markup", margem);

        if (!qry.exec()) {
          qDebug() << "error? " << qry.lastError();
        }
        if (qry.lastError().nativeErrorCode() == "") {
          imported++;
        }
      }
    }

    QSqlQuery("COMMIT").exec();

    texto = "Produtos importados: " + QString::number(imported) + "\nProdutos atualizados: " +
            QString::number(updated) + "\nNão modificados: " + QString::number(notChanged);
  } else {
    texto = "Importação falhou!";
    qDebug() << "db failed :(";
    qDebug() << db.lastError();
  }
  db.close();
  QSqlDatabase::removeDatabase(db.connectionName());

  return texto;
}

int ImportaPortinari::buscarCadastrarFornecedor(QString fornecedor) {
  int idFornecedor = 0;

  QSqlQuery queryFornecedor;
  if (!queryFornecedor.exec("SELECT * FROM Fornecedor WHERE razaoSocial = '" + fornecedor + "'")) {
//    qDebug() << "Erro buscando fornecedor: " << queryFornecedor.lastError();
  }
  if (queryFornecedor.next()) {
    return queryFornecedor.value("idFornecedor").toInt();
  } else {
    QSqlQuery cadastrar;
    if (!cadastrar.exec("INSERT INTO Fornecedor (razaoSocial) VALUES ('" + fornecedor + "')")) {
      qDebug() << "Erro cadastrando fornecedor: " << cadastrar.lastError();
    } else {
      return cadastrar.lastInsertId().toInt();
    }
  }

  return idFornecedor;
}
