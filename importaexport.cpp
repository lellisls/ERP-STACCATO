#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDate>
#include <QProgressBar>
#include <QTimer>

#include "importaexport.h"

void ImportaExport::cancel() { canceled = true; }

ImportaExport::ImportaExport(QObject *parent) : QObject(parent) {}

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
      QString fornecedor = queryForn.value(0).toString();
      if (not fornecedor.isEmpty()) {
        int id = buscarCadastrarFornecedor(fornecedor);
        //        qDebug() << "id: " << id << " - " << fornecedor;
        map.insert(fornecedor, id);
      }
    }

    // Marca todos produtos destes fornecedores como expirado
    QString query = "UPDATE Produto SET expirado = 1";
    QString ids;
    QList<int> idsMap = map.values();

    for (int i = 0; i < idsMap.size(); ++i) {
      qDebug() << "map[" << i << "]: " << idsMap.at(i);
    }

    for (int i = 0; i < map.size(); ++i) {
      if (ids.isEmpty()) {
        ids.append(" WHERE idFornecedor = " + QString::number(idsMap.at(i)));
      } else {
        ids.append(" OR idFornecedor = " + QString::number(idsMap.at(i)));
      }
    }
    query += ids;
    qDebug() << "query: " << query;

    QSqlQuery queryExp;
    if (not queryExp.exec(query)) {
      qDebug() << "Erro em queryExp: " << queryExp.lastError();
    }

    // Lê produtos do excel
    QSqlQuery queryProdSz("SELECT COUNT(*) FROM [BASE$]", db);
    queryProdSz.first();
    //    qDebug() << "SIZE: " << queryProdSz.value(0);
    emit progressRangeChanged(queryProdSz.value(0).toInt());

    QSqlQuery queryProd("SELECT * FROM [BASE$]", db);
    int current = 0;
    while (queryProd.next()) {
      QString fornecedor = queryProd.value(0).toString();
      if (not fornecedor.isEmpty()) {
        emit progressValueChanged(current++);

        if (canceled) {
          emit progressFinished();
          return ("Operação cancelada!");
          QSqlQuery("ROLLBACK").exec();
        }

        QString descricao = queryProd.value(1).toString();
        QString estoque = queryProd.value(2).toString();
        QString un = queryProd.value(3).toString();
        QString colecao = queryProd.value(4).toString();
        QString m2cx = queryProd.value(5).toString();
        QString pccx = queryProd.value(6).toString();
        QString kgcx = queryProd.value(7).toString();
        QString formComercial = queryProd.value(8).toString();
        QString codComercial = queryProd.value(9).toString();
        QString codBarras = queryProd.value(10).toString();
        QString ncm = queryProd.value(11).toString();
        QString icms = queryProd.value(12).toString();
        QString situacaoTributaria = queryProd.value(13).toString();
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
        //      QString validade = queryProd.value(25).toString();

        // Consistência dos dados
        if (estoque.isEmpty()) {
          estoque = "0";
        }
        if (descontinuado.isEmpty()) {
          descontinuado = "0";
        }
        if (ui.isEmpty()) {
          ui = "1";
        }

        // Verifica se produto já se encontra no BD
        QSqlQuery produto, produtoUpd;
        if (not produto.exec("SELECT * FROM Produto WHERE fornecedor = '" + fornecedor +
                             "' AND codComercial = '" + codComercial + "'")) {
          qDebug() << "Erro buscando produto: " << produto.lastError();
        }

        if (produto.next()) {
          QString idProduto = produto.value("idProduto").toString();

          //          // Se o preço for igual extender a validade
          //          if (produto.value("precoVenda").toString() == precoVenda) {
          //            if (not produto.exec("UPDATE Produto_has_Preco SET validadeFim = '" +
          //                              QDate::currentDate().addDays(validade).toString("yyyy-MM-dd") +
          //                              "' WHERE idProduto = " + produto.value("idProduto").toString() +
          //                              "")) {
          //              qDebug() << "Erro atualizando validade do preço: " << produto.lastError();
          //            }
          //            notChanged++;
          //            continue;
          //          }

          if (not fornecedor.isEmpty() and produto.value("fornecedor").toString() != fornecedor) {
            if (produto.value("fornecedor").toString() != fornecedor) {
              qDebug() << "fornecedor diferente";
              //            if (not produto.value("fornecedor").toString().isEmpty()) {
              if (not produtoUpd.exec("UPDATE Produto SET fornecedor = " + fornecedor +
                                      " WHERE idProduto = " + idProduto + "")) {
                qDebug() << "Error on fornecedor: " << produtoUpd.lastError();
              }
              if (not produtoUpd.exec("UPDATE Produto SET fornecedorUpd = 1 WHERE idProduto = " + idProduto +
                                      "")) {
                qDebug() << "Error on fornecedorUpd1: " << produtoUpd.lastError();
              }
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET fornecedorUpd = 0 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on fornecedorUpd0: " << produtoUpd.lastError();
            }
          }

          if (not descricao.isEmpty() and produto.value("descricao").toString() != descricao) {
            qDebug() << "descricao diferente: " << descricao << " | "
                     << produto.value("descricao").toString();
            if (not produtoUpd.exec("UPDATE Produto SET descricao = '" + descricao + "' WHERE idProduto = " +
                                    idProduto + "")) {
              qDebug() << "Error on descricao: " << produtoUpd.lastError();
            }
            if (not produtoUpd.exec("UPDATE Produto SET descricaoUpd = 1 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on descricaoUpd1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET descricaoUpd = 0 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on descricaoUpd0: " << produtoUpd.lastError();
            }
          }

          if (not estoque.isEmpty() and produto.value("estoque") != estoque) {
            if (not produtoUpd.exec("UPDATE Produto SET estoque = " + estoque + " WHERE idProduto = " +
                                    idProduto + "")) {
              qDebug() << "Error on estoque: " << produtoUpd.lastError();
            }
            if (not produtoUpd.exec("UPDATE Produto SET estoqueUpd = 1 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on estoqueUpd1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET estoqueUpd = 0 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on estoqueUpd0: " << produtoUpd.lastError();
            }
          }

          if (not un.isEmpty() and produto.value("un").toString() != un) {
            if (not produtoUpd.exec("UPDATE Produto SET un = '" + un + "' WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on unidade: " << produtoUpd.lastError();
            }
            if (not produtoUpd.exec("UPDATE Produto SET unUpd = 1 WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on unidadeUp1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET unUpd = 0 WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on unidadeUpd0: " << produtoUpd.lastError();
            }
          }

          if (not colecao.isEmpty() and produto.value("colecao").toString() != colecao) {
            if (not produtoUpd.exec("UPDATE Produto SET colecao = '" + colecao + "' WHERE idProduto = " +
                                    idProduto + "")) {
              qDebug() << "Error on colecao: " << produtoUpd.lastError();
            }
            if (not produtoUpd.exec("UPDATE Produto SET colecaoUpd = 1 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on colecaoUp1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET colecaoUpd = 0 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on colecaoUpd0: " << produtoUpd.lastError();
            }
          }

          if (not m2cx.isEmpty() and produto.value("m2cx").toString() != m2cx) {
            if (not produtoUpd.exec("UPDATE Produto SET m2cx = " + m2cx + " WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on m2cx: " << produtoUpd.lastError();
            }
            if (not produtoUpd.exec("UPDATE Produto SET m2cxUpd = 1 WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on m2cxUpd1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET m2cxUpd = 0 WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on m2cxUpd0: " << produtoUpd.lastError();
            }
          }

          if (not pccx.isEmpty() and produto.value("pccx").toString() != pccx) {
            if (not produtoUpd.exec("UPDATE Produto SET pccx = " + pccx + " WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on pccx: " << produtoUpd.lastError();
            }
            if (not produtoUpd.exec("UPDATE Produto SET pccxUpd = 1 WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on pccxUpd1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET pccxUpd = 0 WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on pccxUpd0: " << produtoUpd.lastError();
            }
          }

          if (not kgcx.isEmpty() and produto.value("kgcx").toString() != kgcx) {
            if (not produtoUpd.exec("UPDATE Produto SET kgcx = " + kgcx + " WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on kgcx: " << produtoUpd.lastError();
            }
            if (not produtoUpd.exec("UPDATE Produto SET kgcxUpd = 1 WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on kgcxUpd1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET kgcxUpd = 0 WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on kgcxUpd0: " << produtoUpd.lastError();
            }
          }

          if (not formComercial.isEmpty() and produto.value("formComercial").toString() != formComercial) {
            if (not produtoUpd.exec("UPDATE Produto SET formComercial = '" + formComercial +
                                    "' WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on formComercial: " << produtoUpd.lastError();
            }
            if (not produtoUpd.exec("UPDATE Produto SET formComercialUpd = 1 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on formComercialUpd1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET formComercialUpd = 0 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on formComercialUpd0: " << produtoUpd.lastError();
            }
          }

          if (not codComercial.isEmpty() and produto.value("codComercial").toString() != codComercial) {
            if (not produtoUpd.exec("UPDATE Produto SET codComercial = " + codComercial +
                                    " WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on codCom: " << produtoUpd.lastError();
            }
            if (not produtoUpd.exec("UPDATE Produto SET codComercialUpd = 1 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on codComUpd1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET codComercialUpd = 0 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on codComUpd0: " << produtoUpd.lastError();
            }
          }

          if (not codBarras.isEmpty() and produto.value("codBarras").toString() != codBarras) {
            if (not produtoUpd.exec("UPDATE Produto SET codBarras = " + codBarras + " WHERE idProduto = " +
                                    idProduto + "")) {
              qDebug() << "Error on codBarras: " << produtoUpd.lastError();
            }
            if (not produtoUpd.exec("UPDATE Produto SET codBarrasUpd = 1 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on codBarrasUpd1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET codBarrasUpd = 0 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on codBarrasUpd0: " << produtoUpd.lastError();
            }
          }

          if (not ncm.isEmpty() and produto.value("ncm").toString() != ncm) {
            if (not produtoUpd.exec("UPDATE Produto SET ncm = " + ncm + " WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on ncm: " << produtoUpd.lastError();
            }
            if (not produtoUpd.exec("UPDATE Produto SET ncmUpd = 1 WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on ncmUpd1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET ncmUpd = 0 WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on ncmUpd0: " << produtoUpd.lastError();
            }
          }

          if (not icms.isEmpty() and produto.value("icms").toString() != icms) {
            if (not produtoUpd.exec("UPDATE Produto SET icms = " + icms + " WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on icms: " << produtoUpd.lastError();
            }
            if (not produtoUpd.exec("UPDATE Produto SET icmsUpd = 1 WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on icmsUpd1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET icmsUpd = 0 WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on icmsUpd0: " << produtoUpd.lastError();
            }
          }

          if (not situacaoTributaria.isEmpty() and
              produto.value("situacaoTributaria").toString() != situacaoTributaria) {
            if (not produtoUpd.exec("UPDATE Produto SET situacaoTributaria = " + situacaoTributaria +
                                    " WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on situacaoTributaria: " << produtoUpd.lastError();
            }
            if (not produtoUpd.exec("UPDATE Produto SET situacaoTributariaUpd = 1 WHERE idProduto = " +
                                    idProduto + "")) {
              qDebug() << "Error on situacaoTributariaUpd1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET situacaoTributariaUpd = 0 WHERE idProduto = " +
                                    idProduto + "")) {
              qDebug() << "Error on situacaoTributariaUpd0: " << produtoUpd.lastError();
            }
          }

          if (not qtdPallet.isEmpty() and produto.value("qtdPallet").toString() != qtdPallet) {
            if (not produtoUpd.exec("UPDATE Produto SET qtdPallet = " + qtdPallet + " WHERE idProduto = " +
                                    idProduto + "")) {
              qDebug() << "Error on qtdPallet: " << produtoUpd.lastError();
            }
            if (not produtoUpd.exec("UPDATE Produto SET qtdPalletUpd = 1 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on qtdPalletUpd1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET qtdPalletUpd = 0 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on qtdPalletUpd0: " << produtoUpd.lastError();
            }
          }

          if (not custo.isEmpty() and produto.value("custo").toString() != custo) {
            if (not produtoUpd.exec("UPDATE Produto SET custo = " + custo + " WHERE idProduto = " +
                                    idProduto + "")) {
              qDebug() << "Error on custo: " << produtoUpd.lastError();
            }
            if (not produtoUpd.exec("UPDATE Produto SET custoUpd = 1 WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on custoUpd1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET custoUpd = 0 WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on custoUpd0: " << produtoUpd.lastError();
            }
          }

          if (not ipi.isEmpty() and produto.value("ipi").toString() != ipi) {
            if (not produtoUpd.exec("UPDATE Produto SET ipi = " + ipi + " WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on ipi: " << produtoUpd.lastError();
              qDebug() << "qry: " << produtoUpd.lastQuery();
            }
            if (not produtoUpd.exec("UPDATE Produto SET ipiUpd = 1 WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on ipiUpd1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET ipiUpd = 0 WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on ipiUpd0: " << produtoUpd.lastError();
            }
          }

          if (not st.isEmpty() and produto.value("st").toString() != st) {
            if (not produtoUpd.exec("UPDATE Produto SET st = " + st + " WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on st: " << produtoUpd.lastError();
            }
            if (not produtoUpd.exec("UPDATE Produto SET stUpd = 1 WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on stUpd1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET stUpd = 0 WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on stUpd0: " << produtoUpd.lastError();
            }
          }

          if (not precoVenda.isEmpty() and produto.value("precoVenda").toString() != precoVenda) {
            if (not produtoUpd.exec("UPDATE Produto SET precoVenda = " + precoVenda + " WHERE idProduto = " +
                                    idProduto + "")) {
              qDebug() << "Error on precoVenda: " << produtoUpd.lastError();
            }
            if (not produtoUpd.exec("UPDATE Produto SET precoVendaUpd = 1 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on precoVendaUpd1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET precoVendaUpd = 0 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on precoVendaUpd0: " << produtoUpd.lastError();
            }
          }

          if (not comissao.isEmpty() and produto.value("comissao").toString() != comissao) {
            if (not produtoUpd.exec("UPDATE Produto SET comissao = " + comissao + " WHERE idProduto = " +
                                    idProduto + "")) {
              qDebug() << "Error on comissao: " << produtoUpd.lastError();
            }
            if (not produtoUpd.exec("UPDATE Produto SET comissaoUpd = 1 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on comissaoUpd1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET comissaoUpd = 0 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on comissaoUpd0: " << produtoUpd.lastError();
            }
          }

          if (not observacoes.isEmpty() and produto.value("observacoes").toString() != observacoes) {
            if (not produtoUpd.exec("UPDATE Produto SET observacoes = " + observacoes +
                                    " WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on observacoes: " << produtoUpd.lastError();
            }
            if (not produtoUpd.exec("UPDATE Produto SET observacoesUpd = 1 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on observacoesUpd1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET observacoesUpd = 0 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on observacoesUpd0: " << produtoUpd.lastError();
            }
          }

          if (not origem.isEmpty() and produto.value("origem").toString() != origem) {
            if (not produtoUpd.exec("UPDATE Produto SET origem = " + origem + " WHERE idProduto = " +
                                    idProduto + "")) {
              qDebug() << "Error on origem: " << produtoUpd.lastError();
            }
            if (not produtoUpd.exec("UPDATE Produto SET origemUpd = 1 WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on origemUpd1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET origemUpd = 0 WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on origemUpd0: " << produtoUpd.lastError();
            }
          }

          if (not descontinuado.isEmpty() and produto.value("descontinuado").toString() != descontinuado) {
            if (not produtoUpd.exec("UPDATE Produto SET descontinuado = " + descontinuado +
                                    " WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on descontinuado: " << produtoUpd.lastError();
            }
            if (not produtoUpd.exec("UPDATE Produto SET descontinuadoUpd = 1 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on descontinuadoUpd1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET descontinuadoUpd = 0 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on descontinuadoUpd0: " << produtoUpd.lastError();
            }
          }

          if (not temLote.isEmpty() and produto.value("temLote").toString() != temLote) {
            if (not produtoUpd.exec("UPDATE Produto SET temLote = " + temLote + " WHERE idProduto = " +
                                    idProduto + "")) {
              qDebug() << "Error on temLote: " << produtoUpd.lastError();
            }
            if (not produtoUpd.exec("UPDATE Produto SET temLoteUpd = 1 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on temLoteUpd1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET temLoteUpd = 0 WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on temLoteUpd0: " << produtoUpd.lastError();
            }
          }

          if (not ui.isEmpty() and produto.value("ui").toString() != ui) {
            if (not produtoUpd.exec("UPDATE Produto SET ui = " + ui + " WHERE idProduto = " + idProduto +
                                    "")) {
              qDebug() << "Error on ui: " << produtoUpd.lastError();
            }

            if (not produtoUpd.exec("UPDATE Produto SET uiUpd = 1 WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on uiUpd1: " << produtoUpd.lastError();
            }
          } else {
            if (not produtoUpd.exec("UPDATE Produto SET uiUpd = 0 WHERE idProduto = " + idProduto + "")) {
              qDebug() << "Error on uiUpd0: " << produtoUpd.lastError();
            }
          }

          // Guarda novo preço do produto e altera sua validade
          if (precoVenda != produto.value("precoVenda").toString()) {
            if (not produto.exec(
                  "INSERT INTO Produto_has_Preco (idProduto, preco, validadeInicio, validadeFim) VALUES (" +
                  idProduto + ", '" + precoVenda + "', '" + QDate::currentDate().toString("yyyy-MM-dd") +
                  "',  '" + QDate::currentDate().addDays(validade).toString("yyyy-MM-dd") + "')")) {
              qDebug() << "Erro inserindo em Preço: " << produto.lastError();
              qDebug() << "qry: " << produto.lastQuery();
            }
          }

          // Marca como não expirado
          if (not produto.exec("UPDATE Produto SET expirado = 0 WHERE idProduto = " + idProduto + "")) {
            qDebug() << "Erro marcando produto atualizado como não expirado: " << produto.lastError();
          }

          //          if (not produto.exec("UPDATE Produto SET precoVenda = " + precoVenda + " WHERE idProduto
          //          =
          //          " +
          //                            idProduto + "")) {
          //            qDebug() << "Erro atualizando preço do produto: " << produto.lastError();
          //            qDebug() << "qry: " << produto.lastQuery();
          //          }
          //          if (not produto.exec("UPDATE Produto SET validade = '" +
          //                            QDate::currentDate().addDays(validade).toString("yyyy-MM-dd") +
          //                            "' WHERE idProduto = " + idProduto + "")) {
          //            qDebug() << "Erro atualizando validade do produto: " << produto.lastError();
          //          }

          updated++;
          continue;
        }

        QSqlQuery qryInsert;
        qryInsert.prepare(
              "INSERT INTO Produto (idFornecedor, idFornecedorUpd, fornecedor, fornecedorUpd, "
              "descricao, descricaoUpd, estoque, estoqueUpd, un, unUpd, colecao, "
              "colecaoUpd, m2cx, m2cxUpd, pccx, pccxUpd, kgcx, kgcxUpd, formComercial, "
              "formComercialUpd, codComercial, codComercialUpd, codBarras, codBarrasUpd, ncm, "
              "ncmUpd, icms, icmsUpd, situacaoTributaria, situacaoTributariaUpd, "
              "qtdPallet, qtdPalletUpd, custo, custoUpd, ipi, ipiUpd, st, stUpd,"
              " precoVenda, precoVendaUpd, comissao, comissaoUpd, observacoes, "
              "observacoesUpd, origem, origemUpd, descontinuado, "
              "descontinuadoUpd, temLote, temLoteUpd, ui, uiUpd, expirado"
              ") VALUES "
              "(:idFornecedor, :idFornecedorUpd, :fornecedor, :fornecedorUpd, :descricao, :descricaoUpd, "
              ":estoque, :estoqueUpd, :unidade, :unidadeUpd, :colecao, "
              ":colecaoUpd, :m2cx, :m2cxUpd, :pccx, :pccxUpd, :kgcx, :kgcxUpd, :formatoCom, :formatoComUpd, "
              ":codCom, :codComUpd, :codBarras, :codBarrasUpd, :ncm, :ncmUpd, :icms, :icmsUpd, "
              ":situacaoTrib, :situacaoTribUpd, :qtdPallet, :qtdPalletUpd, :custo, :custoUpd, :ipi, "
              ":ipiUpd, :st, :stUpd, :precoVenda, :precoVendaUpd, :comissao, :comissaoUpd, "
              ":observacoes, :observacoesUpd, :origem, :origemUpd, :descontinuado, :descontinuadoUpd, "
              ":temLote, :temLoteUpd, :ui, :uiUpd, :expirado)");
        qryInsert.bindValue(":idFornecedor", map.value(fornecedor));
        qryInsert.bindValue(":idFornecedorUpd", 1);
        qryInsert.bindValue(":fornecedor", fornecedor);
        qryInsert.bindValue(":fornecedorUpd", 1);
        qryInsert.bindValue(":descricao", descricao);
        qryInsert.bindValue(":descricaoUpd", 1);
        qryInsert.bindValue(":estoque", estoque);
        qryInsert.bindValue(":estoqueUpd", 1);
        qryInsert.bindValue(":unidade", un);
        qryInsert.bindValue(":unidadeUpd", 1);
        qryInsert.bindValue(":colecao", colecao);
        qryInsert.bindValue(":colecaoUpd", 1);
        qryInsert.bindValue(":m2cx", m2cx);
        qryInsert.bindValue(":m2cxUpd", 1);
        qryInsert.bindValue(":pccx", pccx);
        qryInsert.bindValue(":pccxUpd", 1);
        qryInsert.bindValue(":kgcx", kgcx);
        qryInsert.bindValue(":kgcxUpd", 1);
        qryInsert.bindValue(":formatoCom", formComercial);
        qryInsert.bindValue(":formatoComUpd", 1);
        qryInsert.bindValue(":codCom", codComercial);
        qryInsert.bindValue(":codComUpd", 1);
        qryInsert.bindValue(":codBarras", codBarras);
        qryInsert.bindValue(":codBarrasUpd", 1);
        qryInsert.bindValue(":ncm", ncm);
        qryInsert.bindValue(":ncmUpd", 1);
        qryInsert.bindValue(":icms", icms);
        qryInsert.bindValue(":icmsUpd", 1);
        qryInsert.bindValue(":situacaoTrib", situacaoTributaria);
        qryInsert.bindValue(":situacaoTribUpd", 1);
        qryInsert.bindValue(":qtdPallet", qtdPallet);
        qryInsert.bindValue(":qtdPalletUpd", 1);
        qryInsert.bindValue(":custo", custo);
        qryInsert.bindValue(":custoUpd", 1);
        qryInsert.bindValue(":ipi", ipi);
        qryInsert.bindValue(":ipiUpd", 1);
        qryInsert.bindValue(":st", st);
        qryInsert.bindValue(":stUpd", 1);
        qryInsert.bindValue(":precoVenda", precoVenda);
        qryInsert.bindValue(":precoVendaUpd", 1);
        qryInsert.bindValue(":comissao", comissao);
        qryInsert.bindValue(":comissaoUpd", 1);
        qryInsert.bindValue(":observacoes", observacoes);
        qryInsert.bindValue(":observacoesUpd", 1);
        qryInsert.bindValue(":origem", origem);
        qryInsert.bindValue(":origemUpd", 1);
        qryInsert.bindValue(":descontinuado", descontinuado);
        qryInsert.bindValue(":descontinuadoUpd", 1);
        qryInsert.bindValue(":temLote", temLote);
        qryInsert.bindValue(":temLoteUpd", 1);
        qryInsert.bindValue(":ui", ui);
        qryInsert.bindValue(":uiUpd", 1);
        qryInsert.bindValue(":expirado", 0);

        if (not qryInsert.exec()) {
          qDebug() << "Erro inserindo produto no BD: " << qryInsert.lastError();
          qDebug() << "qry: " << qryInsert.lastQuery();

          qDebug() << "--------------------------------";
          qDebug() << "idFornecedor: " << map.value(fornecedor);
          qDebug() << "fornecedor: " << fornecedor;
          qDebug() << "descricao: " << descricao;
          qDebug() << "estoque: " << estoque;
          qDebug() << "unidade: " << un;
          qDebug() << "colecao: " << colecao;
          qDebug() << "m2cx: " << m2cx;
          qDebug() << "pccx: " << pccx;
          qDebug() << "kgcx: " << kgcx;
          qDebug() << "formatoCom: " << formComercial;
          qDebug() << "codCom: " << codComercial;
          qDebug() << "codBarras: " << codBarras;
          qDebug() << "ncm: " << ncm;
          qDebug() << "icms: " << icms;
          qDebug() << "situacaoTrib: " << situacaoTributaria;
          qDebug() << "qtdPallet: " << qtdPallet;
          qDebug() << "custo: " << custo;
          qDebug() << "ipi: " << ipi;
          qDebug() << "st: " << st;
          qDebug() << "precoVenda: " << precoVenda;
          qDebug() << "comissao: " << comissao;
          qDebug() << "observacoes: " << observacoes;
          qDebug() << "origem: " << origem;
          qDebug() << "descontinuado: " << descontinuado;
          qDebug() << "temLote: " << temLote;
          qDebug() << "ui: " << ui;
          qDebug() << "validade: " << validade;
          qDebug() << "--------------------------------";
        } else {
          imported++;

          QString idProduto = qryInsert.lastInsertId().toString();
          //          qDebug() << "idProduto: " << idProduto;

          if (not qryInsert.exec(
                "INSERT INTO Produto_has_Preco (idProduto, preco, validadeInicio, validadeFim) VALUES (" +
                idProduto + ", " + precoVenda + ", '" + QDate::currentDate().toString("yyyy-MM-dd") +
                "', '" + QDate::currentDate().addDays(validade).toString("yyyy-MM-dd") + "')")) {
            qDebug() << "Erro inserindo em Preço: " << qryInsert.lastError();
          }
        }
      }
    }

    QSqlQuery("COMMIT").exec();

    texto = "Produtos importados: " + QString::number(imported) + "\nProdutos atualizados: " +
            QString::number(updated) + "\nNão modificados: " + QString::number(notChanged) +
            "\nNão importados: " + QString::number(notImported);
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
  if (not queryFornecedor.exec("SELECT * FROM Fornecedor WHERE razaoSocial = '" + fornecedor + "'")) {
    qDebug() << "Erro buscando fornecedor: " << queryFornecedor.lastError();
  }
  if (queryFornecedor.next()) {
    return queryFornecedor.value("idFornecedor").toInt();
  } else {
    QSqlQuery cadastrar;
    if (not cadastrar.exec("INSERT INTO Fornecedor (razaoSocial) VALUES ('" + fornecedor + "')")) {
      qDebug() << "Erro cadastrando fornecedor: " << cadastrar.lastError();
    } else {
      return cadastrar.lastInsertId().toInt();
    }
  }

  return idFornecedor;
}

// importar no model da tabela produto e usar delegates para marcar cores nos produtos atualizados, nao
// importados na tabela nova etc
