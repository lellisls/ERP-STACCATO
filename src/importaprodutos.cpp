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
#include "doubledelegate.h"

ImportaProdutos::ImportaProdutos(QWidget *parent) : QDialog(parent), ui(new Ui::ImportaProdutos) {
  ui->setupUi(this);
  setWindowFlags(Qt::Window);

  DoubleDelegate *doubledelegate = new DoubleDelegate(this);
  ui->tableProdutos->setItemDelegate(doubledelegate);
  ui->tableProdutos->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableProdutos->horizontalHeader()->setResizeContentsPrecision(0);

  variantMap.insert("fornecedor", QVariant(QVariant::String));
  variantMap.insert("descricao", QVariant(QVariant::String));
  variantMap.insert("estoque", QVariant(QVariant::Int));
  variantMap.insert("un", QVariant(QVariant::String));
  variantMap.insert("colecao", QVariant(QVariant::String));
  variantMap.insert("m2cx", QVariant(QVariant::Double));
  variantMap.insert("pccx", QVariant(QVariant::Int));
  variantMap.insert("kgcx", QVariant(QVariant::Double));
  variantMap.insert("formComercial", QVariant(QVariant::String));
  variantMap.insert("codComercial", QVariant(QVariant::String));
  variantMap.insert("codBarras", QVariant(QVariant::String));
  variantMap.insert("ncm", QVariant(QVariant::String));
  variantMap.insert("icms", QVariant(QVariant::Double));
  variantMap.insert("situacaoTributaria", QVariant(QVariant::Int));
  variantMap.insert("qtdPallet", QVariant(QVariant::Double));
  variantMap.insert("custo", QVariant(QVariant::Double));
  variantMap.insert("ipi", QVariant(QVariant::Double));
  variantMap.insert("st", QVariant(QVariant::Double));
  variantMap.insert("precoVenda", QVariant(QVariant::Double));
  variantMap.insert("comissao", QVariant(QVariant::Double));
  variantMap.insert("observacoes", QVariant(QVariant::String));
  variantMap.insert("origem", QVariant(QVariant::Int));
  variantMap.insert("descontinuado", QVariant(QVariant::Int));
  variantMap.insert("temLote", QVariant(QVariant::String));
  variantMap.insert("ui", QVariant(QVariant::String));
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

      if (not model.select()) {
        qDebug() << "erro model: " << model.lastError();
        return;
      }

      marcaTodosProdutosDescontinuados();
      contaProdutos();

      int current = 0;
      query.first();

      QSqlRecord record = db.record("BASE$");

      while (query.next()) {
        if (progressDialog->wasCanceled()) {
          canceled = true;
          break;
        }

        if (not query.value(record.indexOf("fornecedor")).isNull()) {
          variantMap.insert("fornecedor", query.value(record.indexOf("fornecedor")));
          progressDialog->setValue(current++);

          leituraProduto(query);

          consistenciaDados();

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
      }

      progressDialog->cancel();
    } else {
      QMessageBox::warning(this, "Aviso!", "Erro ao cadastrar fornecedores.");
      return;
    }

  } else {
    qDebug() << "db failed: " << db.lastError();
    QMessageBox::warning(this, "Aviso!", "Ocorreu um erro ao abrir o arquivo, verifique se o mesmo não está aberto.");
    canceled = true;
  }

  if (not canceled) {
    showMaximized();
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

  if (not model.select()) {
    qDebug() << "erro model: " << model.lastError();
    return;
  }

  //   Proxy usado para pintar células
  ImportaProdutosProxy *proxyModel = new ImportaProdutosProxy(model.fieldIndex("descontinuado"), this);
  proxyModel->setSourceModel(&model);
  ui->tableProdutos->setModel(proxyModel);

  for (int i = 1; i < model.columnCount(); i += 2) {
    ui->tableProdutos->setColumnHidden(i, true);
  }

  ui->tableProdutos->setColumnHidden(model.fieldIndex("idProduto"), true);
  ui->tableProdutos->setColumnHidden(model.fieldIndex("idFornecedor"), true);
  ui->tableProdutos->setColumnHidden(model.fieldIndex("desativado"), true);
  ui->tableProdutos->setColumnHidden(model.fieldIndex("descontinuado"), true);
  ui->tableProdutos->setColumnHidden(model.fieldIndex("estoque"), true);
  ui->tableProdutos->setColumnHidden(model.fieldIndex("markup"), true);
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
  for (int i = 0; i < fornecedores.size(); ++i) {
    if (ids.isEmpty()) {
      ids.append("idFornecedor = " + QString::number(fornecedores.values().at(i)));
    } else {
      ids.append(" OR idFornecedor = " + QString::number(fornecedores.values().at(i)));
    }
  }
}

void ImportaProdutos::marcaTodosProdutosDescontinuados() {
  for (int row = 0; row < model.rowCount(); ++row) {
    model.setData(model.index(row, model.fieldIndex("descontinuado")), 1);
  }
}

void ImportaProdutos::contaProdutos() {
  QSqlQuery queryProdSz("SELECT COUNT(*) FROM [BASE$]", db);
  queryProdSz.first();
  progressDialog->setMaximum(queryProdSz.value(0).toInt());
}

void ImportaProdutos::consistenciaDados() {
  if (variantMap.values().at(variantMap.keys().indexOf("estoque")).isNull()) {
    variantMap.insert("estoque", 0);
  }

  if (variantMap.values().at(variantMap.keys().indexOf("descontinuado")).isNull()) {
    variantMap.insert("descontinuado", 0);
  }

  if (variantMap.values().at(variantMap.keys().indexOf("ui")).isNull()) {
    variantMap.insert("ui", 0);
  }

  if (variantMap.values().at(variantMap.keys().indexOf("ncm")).isNull()) {
    variantMap.insert("ncm", 0);
  }

  QString un = variantMap.values().at(variantMap.keys().indexOf("un")).toString().toLower();
  if (un == "m2") {
    variantMap.insert("un", "m2");
  } else if (un == "ml") {
    variantMap.insert("un", "ml");
  } else {
    variantMap.insert("un", "pç");
  }

  // TODO: NCM pode ter o código EX de 10 digitos
  //    if (values.at(fields.indexOf("ncm")).length() != 8) {
  //  }

  if (variantMap.values().at(variantMap.keys().indexOf("codBarras")).isNull()) {
    variantMap.insert("codBarras", "0");
  }

  variantMap.insert("ncm", variantMap.values().at(variantMap.keys().indexOf("ncm")).toString().remove(".").remove(","));
  variantMap.insert("codBarras",
                    variantMap.values().at(variantMap.keys().indexOf("codBarras")).toString().remove(".").remove(","));
  variantMap.insert(
        "codComercial",
        variantMap.values().at(variantMap.keys().indexOf("codComercial")).toString().remove(".").remove(","));
  variantMap.insert("pccx", variantMap.values().at(variantMap.keys().indexOf("pccx")).toInt());
  variantMap.insert("precoVenda",
                    QString::number(variantMap.value(variantMap.keys().at(variantMap.keys().indexOf("precoVenda")))
                                    .toString()
                                    .replace(",", ".")
                                    .toDouble()).toDouble());
  variantMap.insert("custo", QString::number(variantMap.value(variantMap.keys().at(variantMap.keys().indexOf("custo")))
                                             .toString()
                                             .replace(",", ".")
                                             .toDouble()).toDouble());
}

void ImportaProdutos::leituraProduto(QSqlQuery &queryProd) {
  QSqlRecord record = db.record("BASE$");

  for (int i = 0; i < variantMap.keys().size(); ++i) {
    //                qDebug() << variantMap.keys().at(i) << ": " <<
    //                queryProd.value(record.indexOf(variantMap.keys().at(i)));
    variantMap.insert(variantMap.keys().at(i), queryProd.value(record.indexOf(variantMap.keys().at(i))));
  }
}

void ImportaProdutos::atualizaCamposProduto(QSqlQuery &produto, QString idProduto) {
  QModelIndex index = model.match(model.index(0, 0), Qt::DisplayRole, idProduto).first();

  model.setData(model.index(index.row(), model.fieldIndex("validade")),
                QDate::currentDate().addDays(validade).toString("yyyy-MM-dd"));

  for (int i = 0; i < variantMap.keys().size(); ++i) {
    if (not variantMap.values().at(i).isNull() and
        produto.value(variantMap.keys().at(i)) != variantMap.values().at(i)) {
      model.setData(model.index(index.row(), model.fieldIndex(variantMap.keys().at(i))), variantMap.values().at(i));
      model.setData(model.index(index.row(), model.fieldIndex(variantMap.keys().at(i) + "Upd")), 2);
    } else {
      model.setData(model.index(index.row(), model.fieldIndex(variantMap.keys().at(i) + "Upd")), 0);
    }

    pintarCamposForaDoPadrao(index.row());
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
  if (produto.value("precoVenda") != variantMap.values().at(variantMap.keys().indexOf("precoVenda"))) {
    expiraPrecosAntigos(produto, idProduto);

    if (not produto.exec("INSERT INTO Produto_has_Preco (idProduto, preco, validadeInicio, validadeFim) VALUES (" +
                         idProduto + ", '" +
                         variantMap.values().at(variantMap.keys().indexOf("precoVenda")).toString() + "', '" +
                         QDate::currentDate().toString("yyyy-MM-dd") + "', '" +
                         QDate::currentDate().addDays(validade).toString("yyyy-MM-dd") + "')")) {
      qDebug() << "Erro inserindo em Preço: " << produto.lastError();
      qDebug() << "qry: " << produto.lastQuery();
    }
  }
}

void ImportaProdutos::verificaSeProdutoJaCadastrado(QSqlQuery &produto) {
  if (not produto.exec("SELECT * FROM Produto WHERE fornecedor = '" +
                       variantMap.values().at(variantMap.keys().indexOf("fornecedor")).toString() +
                       "' AND codComercial = '" +
                       variantMap.values().at(variantMap.keys().indexOf("codComercial")).toString() + "' AND ui = '" +
                       variantMap.values().at(variantMap.keys().indexOf("ui")).toString() + "'")) {
    qDebug() << "Erro buscando produto: " << produto.lastError();
  }
}

void ImportaProdutos::pintarCamposForaDoPadrao(int row) {
  if (variantMap.value("ncm").toString() == "0" or variantMap.value("ncm").toString().isEmpty()) {
    model.setData(model.index(row, model.fieldIndex("ncmUpd")), 2);
  }

  if (variantMap.value("codBarras").toString() == "0" or variantMap.value("codBarras").toString().isEmpty()) {
    model.setData(model.index(row, model.fieldIndex("codBarrasUpd")), 2);
  }

  qDebug() << "custo: " << variantMap.value("custo");
  if (variantMap.value("custo") <= 0.0) {
    qDebug() << "custo zero!";
    model.setData(model.index(row, model.fieldIndex("custoUpd")), 3);
  }
}

void ImportaProdutos::cadastraProduto() {
  if (model.insertRow(model.rowCount())) {
    int row = model.rowCount() - 1;

    model.setData(model.index(row, model.fieldIndex("idFornecedor")),
                  fornecedores.value(variantMap.values().at(variantMap.keys().indexOf("fornecedor")).toString()));
    model.setData(model.index(row, model.fieldIndex("atualizarTabelaPreco")), true);
    model.setData(model.index(row, model.fieldIndex("validade")),
                  QDate::currentDate().addDays(validade).toString("yyyy-MM-dd"));

    for (int i = 0; i < variantMap.keys().size(); ++i) {
      model.setData(model.index(row, model.fieldIndex(variantMap.keys().at(i))), variantMap.values().at(i));
      model.setData(model.index(row, model.fieldIndex(variantMap.keys().at(i) + "Upd")), 1);
    }

    pintarCamposForaDoPadrao(row);
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

void ImportaProdutos::on_pushButtonCancelar_clicked() {
  QSqlQuery("ROLLBACK").exec();
  close();
}

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
    qDebug() << "query: " << model.query().lastQuery();
    QMessageBox::warning(this, "Aviso!", "Ocorreu um erro: " + model.lastError().text());
    QSqlQuery("ROLLBACK").exec();
  }
}

bool ImportaProdutos::verificaTabela() {
  QSqlRecord record = db.record("BASE$");

  for (int i = 0; i < variantMap.size(); ++i) {
    if (not record.contains(variantMap.keys().at(i))) {
      QMessageBox::warning(this, "Aviso!", "Tabela não possui coluna " + variantMap.keys().at(i));
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
