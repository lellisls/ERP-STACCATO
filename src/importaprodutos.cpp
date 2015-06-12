#include <QDebug>
#include <QDate>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlRecord>
#include <QSqlField>

#include "importaprodutosproxy.h"
#include "importaprodutos.h"
#include "ui_importaprodutos.h"
#include "dateformatdelegate.h"
#include "validadedialog.h"

ImportaProdutos::ImportaProdutos(QWidget *parent) : QDialog(parent), ui(new Ui::ImportaProdutos) {
  ui->setupUi(this);
  setWindowFlags(Qt::Window);
}

ImportaProdutos::~ImportaProdutos() { delete ui; }

void ImportaProdutos::expiraPrecosAntigos(QSqlQuery produto, QString idProduto) {
  if (not produto.exec("UPDATE Produto_has_Preco SET expirado = 1 WHERE idProduto = " + idProduto)) {
    qDebug() << "Erro expirando preços antigos: " << produto.lastError();
  }
}

void ImportaProdutos::importar() {
  if (not readFile()) {
    return;
  }

  if (not readValidade()) {
    return;
  }

  importarTabela();
}

void ImportaProdutos::importarTabela() {
  setProgressDialog();
  setModelAndTable();

  bool canceled = false;

  QSqlQuery("SET AUTOCOMMIT=0").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (QSqlDatabase::contains("Excel Connection")) {
    db = QSqlDatabase::database("Excel Connection");
  } else {
    db = QSqlDatabase::addDatabase("QODBC", "Excel Connection");
  }

  db.setDatabaseName("DRIVER={Microsoft Excel Driver (*.xls, *.xlsx, *.xlsm, *.xlsb)};DBQ=" + file);

  if (db.open()) {
    if (not verificaTabela()) {
      return;
    }

    QSqlQuery query("SELECT * FROM [BASE$]", db);
    query.first();

    cadastraFornecedores(query);

    if (fornecedores.size() > 0) {
      mostraApenasEstesFornecedores();
      model.setFilter(ids);
      model.select();

      marcaTodosProdutosDescontinuados();
      contaProdutos();

      int current = 0;
      query.first();

      while (query.next()) {
        values.insert(0, query.value(0).toString());

        if (not values.at(fields.indexOf("fornecedor")).isEmpty()) {
          progressDialog->setValue(current++);

          leituraProduto(query);

          if (not consistenciaDados()) {
            continue;
          }

          QSqlQuery produto;
          verificaSeProdutoJaCadastrado(produto);

          if (produto.next()) {
            QString idProduto = produto.value("idProduto").toString();
            atualizaCamposProduto(produto, idProduto);
            guardaNovoPrecoValidade(produto, idProduto);
            marcaProdutoNaoDescontinuado(produto, idProduto);
          } else {
            cadastraProduto();
          }
        }

        if (progressDialog->wasCanceled()) {
          canceled = true;
          break;
        }
      }

      progressDialog->cancel();
    } else {
      QMessageBox::warning(this, "Aviso!", "Erro ao cadastrar fornecedores.");
      return;
    }

  } else {
    qDebug() << "db failed: " << db.lastError();
  }

  if (not canceled) {
    showMaximized();
    ui->tableProdutos->verticalHeader()->setResizeContentsPrecision(0);
    ui->tableProdutos->horizontalHeader()->setResizeContentsPrecision(0);
    ui->tableProdutos->resizeColumnsToContents();
  }
}

void ImportaProdutos::setProgressDialog() {
  progressDialog = new QProgressDialog(this);
  progressDialog->setCancelButton(0);
  progressDialog->setLabelText("Importando...");
  progressDialog->setWindowTitle("ERP Staccato");
  progressDialog->setWindowModality(Qt::WindowModal);
  progressDialog->setMinimum(0);
  progressDialog->setMaximum(0);
  progressDialog->setCancelButtonText("Cancelar");
}

bool ImportaProdutos::readFile() {
  file = QFileDialog::getOpenFileName(this, "Importar tabela genérica", QDir::currentPath(), tr("Excel (*.xlsx)"));

  if (file.isEmpty()) {
    return false;
  }

  return true;
}

bool ImportaProdutos::readValidade() {
  ValidadeDialog *validadeDlg = new ValidadeDialog();

  if (validadeDlg->exec()) {
    validade = validadeDlg->getValidade();
  } else {
    return false;
  }

  return true;
}

void ImportaProdutos::setModelAndTable() {
  model.setTable("Produto");
  model.setEditStrategy(EditableSqlModel::OnManualSubmit);
  // TODO: verify if it's possible to sort a dirty model
//  model.setSort(model.fieldIndex("descontinuado"), Qt::AscendingOrder);
  model.select();

  proxyModel = new ImportaProdutosProxy(model.fieldIndex("descontinuado"), this);
  proxyModel->setSourceModel(&model);
  ui->tableProdutos->setModel(proxyModel);

  for (int i = 1; i < model.columnCount(); i += 2) {
    ui->tableProdutos->setColumnHidden(i, true);
  }

  ui->tableProdutos->setColumnHidden(model.fieldIndex("idProduto"), true);
  ui->tableProdutos->setColumnHidden(model.fieldIndex("idFornecedor"), true);
  ui->tableProdutos->setItemDelegateForColumn(model.fieldIndex("validade"), new DateFormatDelegate("dd-MM-yyyy", this));
}

void ImportaProdutos::cadastraFornecedores(QSqlQuery &query) {
  while (query.next()) {
    QString fornecedor = query.value(0).toString();

    if (not fornecedor.isEmpty()) {
      int id = buscarCadastrarFornecedor(fornecedor);
      fornecedores.insert(fornecedor, id);
    }
  }
}

void ImportaProdutos::mostraApenasEstesFornecedores() {
  QList<int> idsMap = fornecedores.values();

  for (int i = 0; i < fornecedores.size(); ++i) {
    if (ids.isEmpty()) {
      ids.append("idFornecedor = " + QString::number(idsMap.at(i)));
    } else {
      ids.append(" OR idFornecedor = " + QString::number(idsMap.at(i)));
    }
  }
}

void ImportaProdutos::marcaTodosProdutosDescontinuados() {
  for (int row = 0; row < model.rowCount(); ++row) {
    model.setData(model.index(row, model.fieldIndex("descontinuado")), 1);
  }
  // TODO: maybe implement progressDialog here
}

void ImportaProdutos::contaProdutos() {
  QSqlQuery queryProdSz("SELECT COUNT(*) FROM [BASE$]", db);
  queryProdSz.first();
  progressDialog->setMaximum(queryProdSz.value(0).toInt());
}

bool ImportaProdutos::consistenciaDados() {
  // TODO: pintar células informando erro
  // TODO: formatoComercial, CódigoIndustrial, ICMS devem ser zerados se vazio

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

  QString un = values.at(fields.indexOf("un")).toLower();
  if (un == "m2") {
    values[fields.indexOf("un")] = "m2";
  } else if (un == "ml") {
    values[fields.indexOf("un")] = "ml";
  } else {
    values[fields.indexOf("un")] = "pç";
  }

  // TODO: NCM pode ter o código EX de 10 digitos
  //  if (values.at(fields.indexOf("ncm")).length() != 8) {
  //  }

  if (values.at(fields.indexOf("codBarras")).isEmpty()) {
    values[fields.indexOf("codBarras")] = "0";
  }

  values[fields.indexOf("custo")] = values[fields.indexOf("custo")].replace(",", ".");

  if (values.at(fields.indexOf("custo")).toDouble() <= 0.0) {
    return false;
  }

  return true;
}

void ImportaProdutos::leituraProduto(QSqlQuery &queryProd) {
  for (int i = 1; i < fields.size(); ++i) {
    values.insert(i, queryProd.value(i).toString());
  }
}

void ImportaProdutos::atualizaCamposProduto(QSqlQuery &produto, QString idProduto) {
  QModelIndex index = model.match(model.index(0, 0), Qt::DisplayRole, idProduto).first();

  model.setData(model.index(index.row(), model.fieldIndex("validade")),
                QDate::currentDate().addDays(validade).toString("yyyy-MM-dd"));

  for (int i = 0; i < fields.size(); ++i) {
    if (not values.at(i).isEmpty() and produto.value(fields.at(i)).toString() != values.at(i)) {
      model.setData(model.index(index.row(), model.fieldIndex(fields.at(i))), values.at(i));
      model.setData(model.index(index.row(), model.fieldIndex(fields.at(i)) + 1), 2);
    } else {
      model.setData(model.index(index.row(), model.fieldIndex(fields.at(i)) + 1), 0);
    }
  }
}

void ImportaProdutos::marcaProdutoNaoDescontinuado(QSqlQuery &produto, QString idProduto) {
  if (not produto.exec("UPDATE Produto SET descontinuado = 0 WHERE idProduto = " + idProduto + "")) {
    qDebug() << "Erro marcando produto atualizado como não descontinuado: " << produto.lastError();
  }

  QModelIndex index = model.match(model.index(0, 0), Qt::DisplayRole, idProduto).first();
  model.setData(model.index(index.row(), model.fieldIndex("descontinuado")), 0);
}

void ImportaProdutos::guardaNovoPrecoValidade(QSqlQuery &produto, QString idProduto) {
  if (produto.value("precoVenda") != values.at(fields.indexOf("precoVenda"))) {
    expiraPrecosAntigos(produto, idProduto);

    if (not produto.exec("INSERT INTO Produto_has_Preco (idProduto, preco, validadeInicio, "
                         "validadeFim) VALUES (" +
                         idProduto + ", '" + values.at(fields.indexOf("precoVenda")) + "', '" +
                         QDate::currentDate().toString("yyyy-MM-dd") + "',  '" +
                         QDate::currentDate().addDays(validade).toString("yyyy-MM-dd") + "')")) {
      qDebug() << "Erro inserindo em Preço: " << produto.lastError();
      qDebug() << "qry: " << produto.lastQuery();
    }
  }
}

void ImportaProdutos::verificaSeProdutoJaCadastrado(QSqlQuery &produto) {
  if (not produto.exec("SELECT * FROM Produto WHERE fornecedor = '" + values.at(fields.indexOf("fornecedor")) +
                       "' AND codComercial = '" + values.at(fields.indexOf("codComercial")) + "'")) {
    qDebug() << "Erro buscando produto: " << produto.lastError();
  }
}

void ImportaProdutos::cadastraProduto() {
  if (model.insertRow(model.rowCount())) {
    int row = model.rowCount() - 1;

    model.setData(model.index(row, model.fieldIndex("idFornecedor")),
                  fornecedores.value(values.at(fields.indexOf("fornecedor"))));
    model.setData(model.index(row, model.fieldIndex("atualizarTabelaPreco")), true);
    model.setData(model.index(row, model.fieldIndex("validade")),
                  QDate::currentDate().addDays(validade).toString("yyyy-MM-dd"));

    for (int i = 0; i < fields.size(); ++i) {
      model.setData(model.index(row, model.fieldIndex(fields.at(i))), values.at(i));
      model.setData(model.index(row, model.fieldIndex(fields.at(i)) + 1), 1);
    }
  }
}

int ImportaProdutos::buscarCadastrarFornecedor(QString fornecedor) {
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

void ImportaProdutos::on_pushButtonCancelar_clicked() { QSqlQuery("ROLLBACK").exec(); }

void ImportaProdutos::on_pushButtonSalvar_clicked() {
  if (model.submitAll()) {
    QSqlQuery("COMMIT").exec();

    QSqlQuery qryPrecos;

    if (qryPrecos.exec("INSERT INTO Produto_has_Preco (idProduto, preco, validadeInicio, validadeFim) SELECT "
                       "idProduto, precoVenda, '" +
                       QDate::currentDate().toString("yyyy-MM-dd") + "' AS validadeInicio, '" +
                       QDate::currentDate().addDays(validade).toString("yyyy-MM-dd") +
                       "' AS validadeFim FROM Produto "
                       "WHERE atualizarTabelaPreco = 1")) {
      qryPrecos.exec("UPDATE Produto SET atualizarTabelaPreco = 0");
      qryPrecos.exec("COMMIT");
    } else {
      qDebug() << "Erro inserindo preços: " << qryPrecos.lastError() << endl;
      qDebug() << "query: " << qryPrecos.lastQuery();
    }

    close();

  } else {
    qDebug() << "Erro submetendo model: " << model.lastError();
    QMessageBox::warning(this, "Aviso!", "Ocorreu um erro.");
    QSqlQuery("ROLLBACK").exec();
  }
}

bool ImportaProdutos::verificaTabela() {
  QSqlRecord record = db.record("BASE$");

  for (int i = 0; i < fields.size(); ++i) {
    if (not record.contains(fields.at(i))) {
      QMessageBox::warning(this, "Aviso!", "Tabela não possui coluna " + fields.at(i));
      return false;
    }
  }

  return true;
}

#ifdef TEST

void ImportaProdutos::TestImportacao() {
  file = "C:/temp/build-Loja-Desktop_Qt_5_4_0_MinGW_32bit-Test/Bellinzoni.xlsx";
  validade = 10;

  importarTabela();
  on_pushButtonSalvar_clicked();
}

#endif
