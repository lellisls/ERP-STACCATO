#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDate>

#include "importaapavisa.h"
#include "cadastrocliente.h"

void ImportaApavisa::cancel() {
  canceled = true;
}

ImportaApavisa::ImportaApavisa() {}

ImportaApavisa::~ImportaApavisa() {}

QString ImportaApavisa::importar(QString file, int validade) {
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
    int imported = 0;
    int notChanged = 0;
    int updated = 0;

    // Cadastra fornecedores
    QSqlQuery queryForn("SELECT * FROM [Base Apavisa$]", db);

    while (queryForn.next()) {
      QString fornecedor = queryForn.value(21).toString();
      if (!fornecedor.isEmpty() and (fornecedor != "Marca")) {
        int id = buscarCadastrarFornecedor(fornecedor);
//        qDebug() << "id: " << id << " - " << fornecedor;
        map.insert(fornecedor, id);
      }
    }

    // Lê produtos do excel
    QSqlQuery querySz("SELECT COUNT(*) FROM [BASE Apavisa$]", db);
    querySz.first();
    qDebug() << "SIZE: " << querySz.value(0);
    emit progressRangeChanged(querySz.value(0).toInt());

    QSqlQuery queryProd("SELECT * FROM [BASE Apavisa$]", db);
    int current = 0;
    while(queryProd.next()) {
      emit progressValueChanged(current++);

      if(canceled) {
        emit progressFinished();
        return ("Operação cancelada!");
        QSqlQuery("ROLLBACK").exec();
      }

      QString fornecedor = queryProd.value(21).toString();

      if(!fornecedor.isEmpty() and (fornecedor != "Marca")) {

        QString codigo = queryProd.value(22).toString();
        QString colecao = queryProd.value(23).toString();
        QString formCom = queryProd.value(26).toString();
        QString formFab = queryProd.value(28).toString();
        QString descricao = queryProd.value(30).toString();
        QString pcCx = queryProd.value(32).toString();
        QString m2Cx = queryProd.value(33).toString();
        QString cxPallet = queryProd.value(34).toString();
        QString m2Pallet = queryProd.value(35).toString();
        QString kgCx = queryProd.value(36).toString();
        QString m2Pc = queryProd.value(37).toString();
        QString kgPallet = queryProd.value(38).toString();
        QString gTarifa = queryProd.value(39).toString();
        QString precoM2 = queryProd.value(40).toString();
        QString precoPc = queryProd.value(41).toString();
        QString ncm = queryProd.value(42).toString();
        QString custo = queryProd.value(43).toString();
        QString unidade = queryProd.value(44).toString();
        QString venda = queryProd.value(45).toString();

        // Consistência dos dados

        QString column20 = "0";
        if (fornecedor == "Apavisa") {
          column20 = "5";
        }
        if (fornecedor == "Aparici") {
          column20 = "42";
        }

        // Verifica se produto já se encontra no BD
        QSqlQuery produto;
        if(!produto.exec("SELECT * FROM Produto WHERE fornecedor = '"+fornecedor+"' AND codComercial = '"+codigo+"'")) {
          qDebug() << "Erro buscando produto: " << produto.lastError();
        }

        if(produto.next()) {
          QString idProduto = produto.value("idProduto").toString();

          // Se o preço for igual extender a validade
          if(produto.value("precoVenda").toString() == venda) {
            if(!produto.exec("UPDATE Produto_has_Preco SET validadeFim = '"+QDate::currentDate().addDays(validade).toString("yyyy-MM-dd")+"' WHERE idProduto = " + produto.value("idProduto").toString() + "")) {
              qDebug() << "Erro atualizando validade do preço: " << produto.lastError();
            }
            notChanged++;
            continue;
          }

          // Guarda novo preço do produto e altera sua validade
          if(!produto.exec(
                "INSERT INTO Produto_has_Preco (idProduto, preco, validadeInicio, validadeFim) VALUES ("+idProduto+", "+venda+", '"+QDate::currentDate().toString("yyyy-MM-dd")+"', '"+QDate::currentDate().addDays(validade).toString("yyyy-MM-dd")+"')")) {
            qDebug() << "Erro inserindo em Preço: " << produto.lastError();
            qDebug() << "qry: " << produto.lastQuery();
          }
          if(!produto.exec("UPDATE Produto SET precoVenda = "+venda+" WHERE idProduto = "+idProduto+"")) {
            qDebug() << "Erro atualizando preço de produto: " << produto.lastError();
            qDebug() << "qry upd: " << produto.lastQuery();
          }

          updated++;
          continue;
        }

        QSqlQuery qry;
        qry.prepare(
          "INSERT INTO mydb.Produto "
          "(idFornecedor, fornecedor, colecao, formComercial, descricao, codComercial, "
          "pccx, m2cx, qtdPallet, un, ncm, "
          "precoVenda, custo, markup, ui) VALUES (:idFornecedor, :fornecedor, :colecao, :formComercial, "
          ":descricao, :codComercial, :pccx, :m2cx, :qtdPallet, :un, :ncm, :precoVenda, :custo, "
          ":markup, :ui)");
        qry.bindValue(":idFornecedor", map.value(fornecedor));
        qry.bindValue(":fornecedor", fornecedor);
        qry.bindValue(":colecao", colecao);
        qry.bindValue(":formComercial", formCom);
        qry.bindValue(":descricao", descricao);
        qry.bindValue(":codComercial", codigo);
        qry.bindValue(":pccx", pcCx);
        qry.bindValue(":m2cx", m2Cx);
        qry.bindValue(":qtdPallet", m2Pallet);
        qry.bindValue(":un", unidade);
        qry.bindValue(":ncm", ncm);
        qry.bindValue(":precoVenda", venda);
        qry.bindValue(":custo", custo);
        qry.bindValue(":markup", 95);
        qry.bindValue(":ui", column20);

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

int ImportaApavisa::buscarCadastrarFornecedor(QString fornecedor) {
  int idFornecedor = 0;

  QSqlQuery queryFornecedor;
  if (!queryFornecedor.exec("SELECT * FROM Fornecedor WHERE razaoSocial = '" + fornecedor + "'")) {
    qDebug() << "Erro buscando fornecedor: " << queryFornecedor.lastError();
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
