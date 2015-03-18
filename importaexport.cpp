#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDate>
#include <QProgressBar>
#include <QTimer>

#include "importaexport.h"

void ImportaExport::cancel() {
  canceled = true;
}

ImportaExport::ImportaExport() {}

ImportaExport::~ImportaExport() {}

QString ImportaExport::importar(QString file, int validade) {
  canceled = false;
  QTime time;
  time.start();
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
    int notImported = 0;

    // Cadastra fornecedores
    QSqlQuery queryForn("SELECT * FROM [BASE$]", db);

    while (queryForn.next()) {
      QString fornecedor = queryForn.value(1).toString();
      if (!fornecedor.isEmpty()) {
        int id = buscarCadastrarFornecedor(fornecedor);
        //        qDebug() << "id: " << id << " - " << fornecedor;
        map.insert(fornecedor, id);
      }
    }

    // Lê produtos do excel
    QSqlQuery queryProdSz("SELECT COUNT(*) FROM [BASE$]", db);
    queryProdSz.first();
    //    qDebug() << "SIZE: " << queryProdSz.value(0);
    emit progressRangeChanged(queryProdSz.value(0).toInt());

    QSqlQuery queryProd("SELECT * FROM [BASE$]", db);
    int current = 0;
    while (queryProd.next()) {
      emit progressValueChanged(current++);

      if (canceled) {
        emit progressFinished();
        return ("Operação cancelada!");
        QSqlQuery("ROLLBACK").exec();
      }
      QString descricao = queryProd.value(0).toString();
      QString fabricante = queryProd.value(1).toString();
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
        notImported++;
        continue;
      }
      if (metrosCaixa == "N/A") {
        metrosCaixa = "1";
      }
      if (pecasCaixa == "N/A") {
        pecasCaixa = "1";
      }
      if (codCom.isEmpty()) {
        notImported++;
        continue;
      }
      if(venda.toDouble() == 0.0){
        notImported++;
        continue;
      }

      // Verifica se produto já se encontra no BD
      QSqlQuery produto;
      if (!produto.exec("SELECT * FROM Produto WHERE fornecedor = '" + fabricante + "' AND codComercial = '" +
                        codCom + "'")) {
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
              idProduto + ", '" + venda + "', '" + QDate::currentDate().toString("yyyy-MM-dd") + "',  '" +
              QDate::currentDate().addDays(validade).toString("yyyy-MM-dd") + "')")) {
          qDebug() << "Erro inserindo em Preço: " << produto.lastError();
          qDebug() << "qry: " << produto.lastQuery();
        }
        if (!produto.exec("UPDATE Produto SET precoVenda = " + venda + " WHERE idProduto = " + idProduto +
                          "")) {
          qDebug() << "Erro atualizando preço do produto: " << produto.lastError();
          qDebug() << "qry: " << produto.lastQuery();
        }
        if (!produto.exec("UPDATE Produto SET validade = '" +
                          QDate::currentDate().addDays(validade).toString("yyyy-MM-dd") +
                          "' WHERE idProduto = " + idProduto + "")) {
          qDebug() << "Erro atualizando validade do produto: " << produto.lastError();
        }

        updated++;
        continue;
      }

      QSqlQuery qryInsert;
      qryInsert.prepare(
            "INSERT INTO Produto (idFornecedor, fornecedor, colecao, tipo, formComercial, "
            "descricao, codComercial, pccx, m2cx, un, ncm, precoVenda, custo, markup) VALUES "
            "(:idFornecedor, :fornecedor, :colecao, :tipo, :formComercial, :descricao, :codComercial, "
            ":pccx, :m2cx, :un, :ncm, :precoVenda, :custo, :markup)");
      qryInsert.bindValue(":idFornecedor", map.value(fabricante));
      qryInsert.bindValue(":fornecedor", fabricante);
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
        qDebug() << "idfornecedor: " << map.value(fabricante);
        qDebug() << "codComercial: " << codCom;
      }
      if (qryInsert.lastError().nativeErrorCode() == "") {
        imported++;
      }

      QString idProduto = qryInsert.lastInsertId().toString();
      //      qDebug() << "idProduto: " << idProduto;

      if (!qryInsert.exec(
            "INSERT INTO Produto_has_Preco (idProduto, preco, validadeInicio, validadeFim) VALUES (" +
            idProduto + ", " + venda + ", '" + QDate::currentDate().toString("yyyy-MM-dd") + "', '" +
            QDate::currentDate().addDays(validade).toString("yyyy-MM-dd") + "')")) {
        qDebug() << "Erro inserindo em Preço: " << qryInsert.lastError();
      }
    }

    QSqlQuery("COMMIT").exec();

    texto = "Produtos importados: " + QString::number(imported) + "\nProdutos atualizados: " +
            QString::number(updated) + "\nNão modificados: " + QString::number(notChanged) + "\nNão importados: " + QString::number(notImported);
  } else {
    texto = "Importação falhou!";
    qDebug() << "db failed :(";
    qDebug() << db.lastError();
  }
  db.close();
  QSqlDatabase::removeDatabase(db.connectionName());
  emit progressFinished();
  qDebug() << "tempo: " << time.elapsed();
  return texto;
}

int ImportaExport::buscarCadastrarFornecedor(QString fornecedor) {
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

// importar no model da tabela produto e usar delegates para marcar cores nos produtos atualizados, nao
// importados na tabela nova etc
