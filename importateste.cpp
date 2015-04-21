#include <QDebug>
#include <QDate>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QFileDialog>

#include "importaexportproxy.h"
#include "importateste.h"
#include "ui_importateste.h"

ImportaTeste::ImportaTeste(QWidget *parent) : QDialog(parent), ui(new Ui::ImportaTeste) {
  ui->setupUi(this);
  setWindowFlags(Qt::Window);
}

void ImportaTeste::importar(){
  if(!readFile()){
    return;
  }

  setProgressDialog();
  readValidade(); // TODO: implement
  setModelAndTable();

  QSqlQuery("SET AUTOCOMMIT=0").exec();
  QSqlQuery("START TRANSACTION").exec();

  db = QSqlDatabase::addDatabase("QODBC", "Excel Connection");
  db.setDatabaseName("DRIVER={Microsoft Excel Driver (*.xls, *.xlsx, *.xlsm, *.xlsb)};DBQ=" + file);

  if (db.open()) {
    cadastraFornecedores();

    if (fornecedores.size() > 0) {
      mostraApenasEstesFornecedores();
      //      qDebug() << "filter: " << ids;
      model.setFilter(ids);
      model.select();

      marcaTodosProdutosExpirado();
      contaProdutos();

      QSqlQuery queryProd("SELECT * FROM [BASE$]", db);
      int current = 0;
      while (queryProd.next()) {
        values.insert(0, queryProd.value(0).toString());
        if (!values.at(fields.indexOf("fornecedor")).isEmpty()) {
          progressDialog->setValue(current++);

          leituraProduto(queryProd);
          if(!consistenciaDados()){
            continue;
          }

          QSqlQuery produto;
          verificaSeProdutoJaCadastrado(produto);

          if (produto.next()) {
            QString idProduto = produto.value("idProduto").toString();

            atualizaCamposProduto(produto, idProduto);
            guardaNovoPrecoValidade(produto, idProduto);
            marcaProdutoNaoExpirado(produto, idProduto);
          } else {
            cadastraProduto();
            //            QSqlQuery qryInsert = preparaCadastroProduto();

            //            if (!qryInsert.exec()) {
            //              qDebug() << "Erro inserindo produto no BD: " << qryInsert.lastError();
            //              qDebug_InfoProduto(qryInsert);
            //              qDebug() << "--------------------------------";
            //            } else {
            //              //          imported++;
            //            }
//                        cadastraProdutoTemPreco(qryInsert);
          }
        }
      }

      progressDialog->cancel();
    } else {
      qDebug() << "Erro ao cadastrar fornecedores.";
    }
    qDebug() << "Done!";
  } else {
    qDebug() << "db failed :( " << db.lastError();
  }
  ui->tableView->resizeColumnsToContents();
  showMaximized();
}

void ImportaTeste::setProgressDialog() {
  progressDialog = new QProgressDialog(this);
  progressDialog->setCancelButton(0);
  progressDialog->setLabelText("Importando...");
  progressDialog->setWindowTitle("ERP Staccato");
  progressDialog->setWindowModality(Qt::WindowModal);
  progressDialog->setMinimum(0);
  progressDialog->setMaximum(0);
  progressDialog->setCancelButtonText("Cancelar");
}

bool ImportaTeste::readFile() {
  file = QFileDialog::getOpenFileName(this, "Importar tabela genérica", QDir::currentPath(),
                                      tr("Excel (*.xlsx)"));
  qDebug() << "file: " << file;
  if (file.isEmpty()) {
//    qDebug() << "closing: " << close();
    return false;
  }
  return true;
}

void ImportaTeste::readValidade() { validade = 10; }

void ImportaTeste::setModelAndTable() {
  model.setTable("Produto");
  model.setEditStrategy(EditableSqlModel::OnManualSubmit);
  model.setSort(model.fieldIndex("expirado"), Qt::AscendingOrder);
  model.select();

  proxyModel = new ImportaExportProxy(model.fieldIndex("expirado"));
  proxyModel->setSourceModel(&model);
  ui->tableView->setModel(proxyModel);
  for (int i = 1; i < model.columnCount(); i += 2) {
    ui->tableView->setColumnHidden(i, true);
  }
  ui->tableView->setColumnHidden(model.fieldIndex("idProduto"), true);
  ui->tableView->setColumnHidden(model.fieldIndex("idFornecedor"), true);
}

void ImportaTeste::cadastraFornecedores() {
  QSqlQuery queryForn("SELECT * FROM [BASE$]", db);

  while (queryForn.next()) {
    QString fornecedor = queryForn.value(0).toString();
    if (!fornecedor.isEmpty()) {
      int id = buscarCadastrarFornecedor(fornecedor);
      //        qDebug() << "id: " << id << " - " << fornecedor;
      fornecedores.insert(fornecedor, id);
    }
  }
}

void ImportaTeste::mostraApenasEstesFornecedores() {
  QList<int> idsMap = fornecedores.values();

  for (int i = 0; i < fornecedores.size(); ++i) {
    if (ids.isEmpty()) {
      ids.append("idFornecedor = " + QString::number(idsMap.at(i)));
    } else {
      ids.append(" OR idFornecedor = " + QString::number(idsMap.at(i)));
    }
  }
}

void ImportaTeste::marcaTodosProdutosExpirado() {
  for (int row = 0; row < model.rowCount(); ++row) {
    if (model.index(row, model.fieldIndex("expirado")).isValid()) {
      model.setData(model.index(row, model.fieldIndex("expirado")), 1);
    }
  }
}

void ImportaTeste::contaProdutos() {
  QSqlQuery queryProdSz("SELECT COUNT(*) FROM [BASE$]", db);
  queryProdSz.first();
  progressDialog->setMaximum(queryProdSz.value(0).toInt());
}

bool ImportaTeste::consistenciaDados() {
  if (values.at(fields.indexOf("estoque")).isEmpty()) {
    values[fields.indexOf("estoque")] = "0";
  }
  if (values.at(fields.indexOf("descontinuado")).isEmpty()) {
    values[fields.indexOf("descontinuado")] = "0";
  }
  if (values.at(fields.indexOf("ui")).isEmpty()) {
    values[fields.indexOf("ui")] = "1";
  }
  if (values.at(fields.indexOf("ncm")).isEmpty()) {
    values[fields.indexOf("ncm")] = "0";
  }

  if (values.at(fields.indexOf("ncm")).length() != 8) {
    // TODO: pintar celular
  }

  values[fields.indexOf("custo")] = values[fields.indexOf("custo")].replace(",", ".");
  if(values.at(fields.indexOf("custo")).toDouble() <= 0.0){
    qDebug() << "custo: " << values.at(fields.indexOf("custo")).toDouble();
    return false;
  }

  return true;
}

void ImportaTeste::leituraProduto(QSqlQuery &queryProd) {
  for (int i = 1; i < fields.size(); ++i) {
    values.insert(i, queryProd.value(i).toString());
  }
}

void ImportaTeste::atualizaCamposProduto(QSqlQuery &produto, QString idProduto) {
  QModelIndex index = model.match(model.index(0, 0), Qt::DisplayRole, idProduto).first();

  for (int i = 0; i < fields.size(); ++i) {
    if (!values.at(i).isEmpty() and produto.value(fields.at(i)).toString() != values.at(i)) {
      model.setData(model.index(index.row(), model.fieldIndex(fields.at(i))), values.at(i));
      model.setData(model.index(index.row(), model.fieldIndex(fields.at(i)) + 1), 2);
    } else {
      model.setData(model.index(index.row(), model.fieldIndex(fields.at(i)) + 1), 0);
    }
  }
}

void ImportaTeste::marcaProdutoNaoExpirado(QSqlQuery &produto, QString idProduto) {
  if (!produto.exec("UPDATE Produto SET expirado = 0 WHERE idProduto = " + idProduto + "")) {
    qDebug() << "Erro marcando produto atualizado como não expirado: " << produto.lastError();
  }
  QModelIndex index = model.match(model.index(0, 0), Qt::DisplayRole, idProduto.toInt()).first();
  model.setData(model.index(index.row(), model.fieldIndex("expirado")), 0);
}

void ImportaTeste::guardaNovoPrecoValidade(QSqlQuery &produto, QString idProduto) {
  if (produto.value(fields.at(fields.indexOf("precoVenda"))) != values.at(fields.indexOf("precoVenda"))) {
    if (!produto.exec("INSERT INTO Produto_has_Preco (idProduto, preco, validadeInicio, "
                      "validadeFim) VALUES (" +
                      idProduto + ", '" + values.at(fields.indexOf("precoVenda")) + "', '" +
                      QDate::currentDate().toString("yyyy-MM-dd") + "',  '" +
                      QDate::currentDate().addDays(validade).toString("yyyy-MM-dd") + "')")) {
      qDebug() << "Erro inserindo em Preço: " << produto.lastError();
      qDebug() << "qry: " << produto.lastQuery();
    }
  }
}

void ImportaTeste::verificaSeProdutoJaCadastrado(QSqlQuery &produto) {
  if (!produto.exec("SELECT * FROM Produto WHERE fornecedor = '" + values.at(fields.indexOf("fornecedor")) +
                    "' AND codComercial = '" + values.at(fields.indexOf("codComercial")) + "'")) {
    qDebug() << "Erro buscando produto: " << produto.lastError();
  }
}

void ImportaTeste::cadastraProduto() {
  if (model.insertRow(model.rowCount())) {
    int row = model.rowCount() - 1;
    model.setData(model.index(row, model.fieldIndex("idFornecedor")), fornecedores.value(values.at(fields.indexOf("fornecedor"))));
    for (int i = 0; i < fields.size(); ++i) {
      model.setData(model.index(row, model.fieldIndex(fields.at(i))), values.at(i));
      model.setData(model.index(row, model.fieldIndex(fields.at(i)) + 1), 1);
    }
  }
}

QSqlQuery ImportaTeste::preparaCadastroProduto() {
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
//    qryInsert.bindValue(":idFornecedor", fornecedores.value(fornecedor));
  //  qryInsert.bindValue(":idFornecedorUpd", 1);
  //  qryInsert.bindValue(":fornecedor", fornecedor);
  //  qryInsert.bindValue(":fornecedorUpd", 1);
  //  qryInsert.bindValue(":descricao", descricao);
  //  qryInsert.bindValue(":descricaoUpd", 1);
  //  qryInsert.bindValue(":estoque", estoque);
  //  qryInsert.bindValue(":estoqueUpd", 1);
  //  qryInsert.bindValue(":unidade", un);
  //  qryInsert.bindValue(":unidadeUpd", 1);
  //  qryInsert.bindValue(":colecao", colecao);
  //  qryInsert.bindValue(":colecaoUpd", 1);
  //  qryInsert.bindValue(":m2cx", m2cx);
  //  qryInsert.bindValue(":m2cxUpd", 1);
  //  qryInsert.bindValue(":pccx", pccx);
  //  qryInsert.bindValue(":pccxUpd", 1);
  //  qryInsert.bindValue(":kgcx", kgcx);
  //  qryInsert.bindValue(":kgcxUpd", 1);
  //  qryInsert.bindValue(":formatoCom", formComercial);
  //  qryInsert.bindValue(":formatoComUpd", 1);
  //  qryInsert.bindValue(":codCom", codComercial);
  //  qryInsert.bindValue(":codComUpd", 1);
  //  qryInsert.bindValue(":codBarras", codBarras);
  //  qryInsert.bindValue(":codBarrasUpd", 1);
  //  qryInsert.bindValue(":ncm", ncm);
  //  qryInsert.bindValue(":ncmUpd", 1);
  //  qryInsert.bindValue(":icms", icms);
  //  qryInsert.bindValue(":icmsUpd", 1);
  //  qryInsert.bindValue(":situacaoTrib", situacaoTributaria);
  //  qryInsert.bindValue(":situacaoTribUpd", 1);
  //  qryInsert.bindValue(":qtdPallet", qtdPallet);
  //  qryInsert.bindValue(":qtdPalletUpd", 1);
  //  qryInsert.bindValue(":custo", custo);
  //  qryInsert.bindValue(":custoUpd", 1);
  //  qryInsert.bindValue(":ipi", ipi);
  //  qryInsert.bindValue(":ipiUpd", 1);
  //  qryInsert.bindValue(":st", st);
  //  qryInsert.bindValue(":stUpd", 1);
  //  qryInsert.bindValue(":precoVenda", precoVenda);
  //  qryInsert.bindValue(":precoVendaUpd", 1);
  //  qryInsert.bindValue(":comissao", comissao);
  //  qryInsert.bindValue(":comissaoUpd", 1);
  //  qryInsert.bindValue(":observacoes", observacoes);
  //  qryInsert.bindValue(":observacoesUpd", 1);
  //  qryInsert.bindValue(":origem", origem);
  //  qryInsert.bindValue(":origemUpd", 1);
  //  qryInsert.bindValue(":descontinuado", descontinuado);
  //  qryInsert.bindValue(":descontinuadoUpd", 1);
  //  qryInsert.bindValue(":temLote", temLote);
  //  qryInsert.bindValue(":temLoteUpd", 1);
  //  qryInsert.bindValue(":ui", unidInd);
  //  qryInsert.bindValue(":uiUpd", 1);
  //  qryInsert.bindValue(":expirado", 0);

  return qryInsert;
}

void ImportaTeste::qDebug_InfoProduto(QSqlQuery &qryInsert) {
  qDebug() << "qry: " << qryInsert.lastQuery();

  qDebug() << "--------------------------------";
  //  qDebug() << "idFornecedor: " << fornecedores.value(fornecedor);
  //  qDebug() << "fornecedor: " << fornecedor;
  //  qDebug() << "descricao: " << descricao;
  //  qDebug() << "estoque: " << estoque;
  //  qDebug() << "unidade: " << un;
  //  qDebug() << "colecao: " << colecao;
  //  qDebug() << "m2cx: " << m2cx;
  //  qDebug() << "pccx: " << pccx;
  //  qDebug() << "kgcx: " << kgcx;
  //  qDebug() << "formatoCom: " << formComercial;
  //  qDebug() << "codCom: " << codComercial;
  //  qDebug() << "codBarras: " << codBarras;
  //  qDebug() << "ncm: " << ncm;
  //  qDebug() << "icms: " << icms;
  //  qDebug() << "situacaoTrib: " << situacaoTributaria;
  //  qDebug() << "qtdPallet: " << qtdPallet;
  //  qDebug() << "custo: " << custo;
  //  qDebug() << "ipi: " << ipi;
  //  qDebug() << "st: " << st;
  //  qDebug() << "precoVenda: " << precoVenda;
  //  qDebug() << "comissao: " << comissao;
  //  qDebug() << "observacoes: " << observacoes;
  //  qDebug() << "origem: " << origem;
  //  qDebug() << "descontinuado: " << descontinuado;
  //  qDebug() << "temLote: " << temLote;
  //  qDebug() << "ui: " << unidInd;
  //  qDebug() << "validade: " << validade;
}

void ImportaTeste::cadastraProdutoTemPreco(QSqlQuery &qryInsert) {
  QString idProduto = qryInsert.lastInsertId().toString();
  //          qDebug() << "idProduto: " << idProduto;

  if (!qryInsert.exec(
        "INSERT INTO Produto_has_Preco (idProduto, preco, validadeInicio, validadeFim) VALUES (" +
        idProduto + ", " + values.at(fields.indexOf("precoVenda")) + ", '" +
        QDate::currentDate().toString("yyyy-MM-dd") + "', '" +
        QDate::currentDate().addDays(validade).toString("yyyy-MM-dd") + "')")) {
    qDebug() << "Erro inserindo em Preço: " << qryInsert.lastError();
  }
}

ImportaTeste::~ImportaTeste() { delete ui; }

int ImportaTeste::buscarCadastrarFornecedor(QString fornecedor) {
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

void ImportaTeste::on_pushButtonCancelar_clicked() { QSqlQuery("ROLLBACK").exec(); }

void ImportaTeste::on_pushButtonSalvar_clicked() {
  qDebug() << "idFornecedor: " << model.fieldIndex("idFornecedor");
  if (model.submitAll()) {
    qDebug() << "submeteu model, comittando sql";
    QSqlQuery("COMMIT").exec();
    close();
  } else {
    qDebug() << "Erro submetendo model: " << model.lastError();
    QSqlQuery("ROLLBACK").exec();
  }
}
