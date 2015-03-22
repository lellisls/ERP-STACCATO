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

QString ImportaExport::importar(QString file, int validadeInt) {
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
            QString fornecedor = queryForn.value(0).toString();
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

            QString fornecedor = queryProd.value(0).toString();

            if(!fornecedor.isEmpty()){
                QString descricao = queryProd.value(1).toString();
                QString estoque = queryProd.value(2).toString();
                QString unidade = queryProd.value(3).toString();
                QString colecao = queryProd.value(4).toString();
                QString m2cx = queryProd.value(5).toString();
                QString pccx = queryProd.value(6).toString();
                QString kgcx = queryProd.value(7).toString();
                QString formatoCom = queryProd.value(8).toString();
                QString codCom = queryProd.value(9).toString();
                QString codBarras = queryProd.value(10).toString();
                QString ncm = queryProd.value(11).toString();
                QString icms = queryProd.value(12).toString();
                QString situacaoTrib = queryProd.value(13).toString();
                QString qtdPallet = queryProd.value(14).toString();
                QString custo = queryProd.value(15).toString();
                QString ipi = queryProd.value(16).toString();
                QString st = queryProd.value(17).toString();
                QString precoVenda = queryProd.value(18).toString();
                QString comissao = queryProd.value(19).toString();
                QString observacoes = queryProd.value(20).toString();
                QString origem = queryProd.value(21).toString();
                QString descontinuado = queryProd.value(22).toString();
                QString temLote = queryProd.value(23).toString();
                QString ui = queryProd.value(24).toString();
                QString validade = queryProd.value(25).toString();

                // Consistência dos dados
                if(estoque.isEmpty()){
                    estoque = "1";
                }
                if(descontinuado.isEmpty()){
                    descontinuado = "0";
                }
                if(ui.isEmpty()){
                    ui = "1";
                }

                // Verifica se produto já se encontra no BD
                QSqlQuery produto;
                if (!produto.exec("SELECT * FROM Produto WHERE fornecedor = '" + fornecedor + "' AND codComercial = '" +
                                  codCom + "'")) {
                    qDebug() << "Erro buscando produto: " << produto.lastError();
                }

                if (produto.next()) {
                    QString idProduto = produto.value("idProduto").toString();

                    // Se o preço for igual extender a validade
                    if (produto.value("precoVenda").toString() == precoVenda) {
                        if (!produto.exec("UPDATE Produto_has_Preco SET validadeFim = '" +
                                          QDate::currentDate().addDays(validadeInt).toString("yyyy-MM-dd") +
                                          "' WHERE idProduto = " + produto.value("idProduto").toString() + "")) {
                            qDebug() << "Erro atualizando validade do preço: " << produto.lastError();
                        }
                        notChanged++;
                        continue;
                    }

                    // Guarda novo preço do produto e altera sua validade
                    if (!produto.exec(
                                "INSERT INTO Produto_has_Preco (idProduto, preco, validadeInicio, validadeFim) VALUES (" +
                                idProduto + ", '" + precoVenda + "', '" + QDate::currentDate().toString("yyyy-MM-dd") + "',  '" +
                                QDate::currentDate().addDays(validadeInt).toString("yyyy-MM-dd") + "')")) {
                        qDebug() << "Erro inserindo em Preço: " << produto.lastError();
                        qDebug() << "qry: " << produto.lastQuery();
                    }
                    if (!produto.exec("UPDATE Produto SET precoVenda = " + precoVenda + " WHERE idProduto = " + idProduto +
                                      "")) {
                        qDebug() << "Erro atualizando preço do produto: " << produto.lastError();
                        qDebug() << "qry: " << produto.lastQuery();
                    }
                    if (!produto.exec("UPDATE Produto SET validade = '" +
                                      QDate::currentDate().addDays(validadeInt).toString("yyyy-MM-dd") +
                                      "' WHERE idProduto = " + idProduto + "")) {
                        qDebug() << "Erro atualizando validade do produto: " << produto.lastError();
                    }

                    updated++;
                    continue;
                }

                QSqlQuery qryInsert;
                qryInsert.prepare(
                            "INSERT INTO Produto (idFornecedor, fornecedor, descricao, estoque, un, colecao, "
                            "m2cx, pccx, kgcx, formComercial, codComercial, codBarras, ncm, icms, situacaoTributaria, qtdPallet, "
                            "custo, ipi, st, precoVenda, comissao, observacoes, origem, descontinuado, temLote, ui, validade"
                            ") VALUES "
                            "(:idFornecedor, :fornecedor, :descricao, :estoque, :unidade, :colecao, "
                            ":m2cx, :pccx, :kgcx, :formatoCom, :codCom, :codBarras, :ncm, :icms, "
                            ":situacaoTrib, :qtdPallet, :custo, :ipi, :st, :precoVenda, :comissao, "
                            ":observacoes, :origem, :descontinuado, :temLote, :ui, :validade)");
                qryInsert.bindValue(":idFornecedor", map.value(fornecedor));
                qryInsert.bindValue(":fornecedor", fornecedor);
                qryInsert.bindValue(":descricao", descricao);
                qryInsert.bindValue(":estoque", estoque);
                qryInsert.bindValue(":unidade", unidade);
                qryInsert.bindValue(":colecao", colecao);
                qryInsert.bindValue(":m2cx", m2cx);
                qryInsert.bindValue(":pccx", pccx);
                qryInsert.bindValue(":kgcx", kgcx);
                qryInsert.bindValue(":formatoCom", formatoCom);
                qryInsert.bindValue(":codCom", codCom);
                qryInsert.bindValue(":codBarras", codBarras);
                qryInsert.bindValue(":ncm", ncm);
                qryInsert.bindValue(":icms", icms);
                qryInsert.bindValue(":situacaoTrib", situacaoTrib);
                qryInsert.bindValue(":qtdPallet", qtdPallet);
                qryInsert.bindValue(":custo", custo);
                qryInsert.bindValue(":ipi", ipi);
                qryInsert.bindValue(":st", st);
                qryInsert.bindValue(":precoVenda", precoVenda);
                qryInsert.bindValue(":comissao", comissao);
                qryInsert.bindValue(":observacoes", observacoes);
                qryInsert.bindValue(":origem", origem);
                qryInsert.bindValue(":descontinuado", descontinuado);
                qryInsert.bindValue(":temLote", temLote);
                qryInsert.bindValue(":ui", ui);
                qryInsert.bindValue(":validade", validade);

                if (!qryInsert.exec()) {
                    qDebug() << "Erro inserindo produto no BD: " << qryInsert.lastError();
                    qDebug() << "pccx: " << codBarras << "(" << codBarras;
                    qDebug() << "m2cx: " << icms << "(" << icms;
                    qDebug() << "idfornecedor: " << map.value(estoque);
                    qDebug() << "codComercial: " << icms;
                }
                if (qryInsert.lastError().nativeErrorCode() == "") {
                    imported++;
                }

                QString idProduto = qryInsert.lastInsertId().toString();
                //      qDebug() << "idProduto: " << idProduto;

                if(!idProduto.isEmpty()){
                    if (!qryInsert.exec(
                                "INSERT INTO Produto_has_Preco (idProduto, preco, validadeInicio, validadeFim) VALUES (" +
                                idProduto + ", " + precoVenda + ", '" + QDate::currentDate().toString("yyyy-MM-dd") + "', '" +
                                QDate::currentDate().addDays(validadeInt).toString("yyyy-MM-dd") + "')")) {
                        qDebug() << "Erro inserindo em Preço2: " << qryInsert.lastError();
                        qDebug() << "qry: " << qryInsert.lastQuery();
                    }
                }
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
