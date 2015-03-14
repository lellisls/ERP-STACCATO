#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDate>

#include "importaexport.h"

ImportaExport::ImportaExport() {}

ImportaExport::~ImportaExport() {}

QString ImportaExport::importar(QString file) {
  QLocale locale(QLocale::Portuguese);
  QString texto;
  QMap<int, QString> map;

  QSqlQuery("BEGIN TRANSACTION").exec();

  QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", "Excel Connection");
  db.setDatabaseName("DRIVER={Microsoft Excel Driver (*.xls, *.xlsx, *.xlsm, *.xlsb)};DBQ=" + file);

  if (db.open()) {
    int imported = 0;
    int notChanged = 0;
    int updated = 0;

    // Cadastra fornecedores
    QSqlQuery queryForn("SELECT * FROM [" + QString("DE_PARA$]"), db);

    while (queryForn.next()) {
      QString id = queryForn.value(0).toString();
      if (!id.isEmpty()) {
        QString nome = queryForn.value(1).toString();
//        qDebug() << "id: " << id << " - " << nome;
        buscarCadastrarFornecedor(id, nome);
//        qDebug() << "id to int: " << id.toInt();
        map.insert(id.toInt(), nome);
      }
    }

    // Lê produtos do excel
    QSqlQuery queryProd("SElECT * FROM [" + QString("BASE$]"), db);
    while (queryProd.next()) {
      QString descricao = queryProd.value(0).toString();
      QString idFabricante = queryProd.value(1).toString();
      QString colecao = queryProd.value(2).toString();
      QString tipo = queryProd.value(3).toString();
      QString formatoCom = queryProd.value(4).toString();
      QString retBold = queryProd.value(5).toString();
      QString custo = queryProd.value(6).toString();
      QString unidade = queryProd.value(7).toString();
      QString metrosCaixa = queryProd.value(8).toString();
      QString pecasCaixa = queryProd.value(9).toString();
      QString ncm = queryProd.value(11).toString();
      QString codCom = queryProd.value(14).toString();
      QString margem = queryProd.value(15).toString();
      QString venda = queryProd.value(16).toString();

      // Consistência dos dados
      if (descricao.isEmpty()) {
        continue;
      }
      if (metrosCaixa == "N/A") {
        metrosCaixa = "1";
      }
      if (pecasCaixa == "N/A") {
        pecasCaixa = "1";
      }
      if (codCom.isEmpty()) {
        continue;
      }

      // Verifica se produto já se encontra no BD
      QSqlQuery produto;
      if (!produto.exec("SELECT * FROM Produto WHERE idFornecedor = " + idFabricante +
                        " AND codComercial = '" + codCom + "'")) {
        qDebug() << "Erro buscando produto: " << produto.lastError();
      }

      if (produto.next()) {
        QString idProduto = produto.value("idProduto").toString();

        // if price is equal just continue (maybe extend validity)
        if (produto.value("precoVenda").toString() == venda) {
          notChanged++;
          continue;
        }
        // Guarda novo preço do produto e altera sua validade
        if (!produto.exec(
                "INSERT INTO Produto_has_Preco (idProduto, preco, validadeInicio, validadeFim) VALUES (" +
                idProduto + ", '" + venda + "', '" + QDate::currentDate().toString("yyyy-MM-dd") + "',  '" +
                QDate::currentDate().addMonths(1).toString("yyyy-MM-dd") + "')")) {
          qDebug() << "Erro inserindo em Preço: " << produto.lastError();
          qDebug() << "qry: " << produto.lastQuery();
        }
        if (!produto.exec("UPDATE Produto SET precoVenda = " + venda + " WHERE idProduto = " + idProduto +
                          "")) {
          qDebug() << "Erro atualizando preço do produto: " << produto.lastError();
          qDebug() << "qry: " << produto.lastQuery();
        }
        if (!produto.exec("UPDATE Produto SET validade = '" +
                          QDate::currentDate().addMonths(1).toString("yyyy-MM-dd") + "' WHERE idProduto = " +
                          idProduto + "")) {
          qDebug() << "Erro atualizando validade do produto: " << produto.lastError();
        }

        updated++;
        continue;
      }

      QSqlQuery qryInsert;
      qryInsert.prepare("INSERT INTO mydb.Produto (idFornecedor, fornecedor, colecao, tipo, formComercial, "
                  "descricao, codComercial, pccx, m2cx, un, ncm, precoVenda, custo, markup) VALUES "
                  "(:idFornecedor, :fornecedor, :colecao, :tipo, :formComercial, :descricao, :codComercial, "
                  ":pccx, :m2cx, :un, :ncm, :precoVenda, :custo, :markup)");
      qryInsert.bindValue(":idFornecedor", idFabricante);
      //      qry.bindValue(":fornecedor", buscarFornecedor(idFabricante));
      qryInsert.bindValue(":fornecedor", map.value(idFabricante.toInt()));
      qryInsert.bindValue(":colecao", colecao);
      qryInsert.bindValue(":tipo", tipo);
      qryInsert.bindValue(":formComercial", formatoCom);
      qryInsert.bindValue(":descricao", descricao);
      qryInsert.bindValue(":codComercial", codCom);
      qryInsert.bindValue(":pccx", locale.toDouble(pecasCaixa));
      qryInsert.bindValue(":m2cx", locale.toDouble(metrosCaixa));
      qryInsert.bindValue(":un", unidade);
      qryInsert.bindValue(":ncm", ncm);
      qryInsert.bindValue(":precoVenda", venda);
      qryInsert.bindValue(":custo", custo);
      qryInsert.bindValue(":markup", margem);

      if (!qryInsert.exec()) {
        qDebug() << "Erro inserindo produto no BD: " << qryInsert.lastError();
        qDebug() << "pccx: " << pecasCaixa << "(" << locale.toDouble(pecasCaixa);
        qDebug() << "m2cx: " << metrosCaixa << "(" << locale.toDouble(metrosCaixa);
        qDebug() << "fornecedor: " << map.value(idFabricante.toInt());
        qDebug() << "codComercial: " << codCom;
      }
      if (qryInsert.lastError().nativeErrorCode() == "") {
        imported++;
      }

      QSqlQuery produto2;
      if (!produto2.exec("SELECT * FROM Produto WHERE idFornecedor = " + idFabricante +
                         " AND codComercial = '" + codCom + "'")) {
        qDebug() << "Erro buscando produto: " << produto2.lastError();
      }

      if (produto2.next()) {
        QString idProduto = produto2.value("idProduto").toString();

        if (!produto2.exec(
                "INSERT INTO Produto_has_Preco (idProduto, preco, validadeInicio, validadeFim) VALUES (" +
                idProduto + ", '" + venda + "', '" + QDate::currentDate().toString("yyyy-MM-dd") + "',  '" +
                QDate::currentDate().addMonths(1).toString("yyyy-MM-dd") + "')")) {
          qDebug() << "Erro inserindo em Preço: " << produto2.lastError();
          qDebug() << "qry: " << produto2.lastQuery();
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

// importar no model da tabela produto e usar delegates para marcar cores nos produtos atualizados, nao
// importados na tabela nova etc
