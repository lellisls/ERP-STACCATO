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
#include "porcentagemdelegate.h"

ImportaProdutos::ImportaProdutos(QWidget *parent) : QDialog(parent), ui(new Ui::ImportaProdutos) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setVariantMap();
  setProgressDialog();
  setModelAndTable();

  ui->tableProdutos->horizontalHeader()->setMinimumSectionSize(50);
}

ImportaProdutos::~ImportaProdutos() { delete ui; }

void ImportaProdutos::expiraPrecosAntigos(QSqlQuery produto, QString idProduto) {
  produto.prepare("UPDATE Produto_has_Preco SET expirado = TRUE WHERE idProduto = :idProduto");
  produto.bindValue(":idProduto", idProduto);

  if (not produto.exec()) {
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
      db.close();
      return;
    }

    QSqlQuery query("SELECT * FROM [BASE$]", db);

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

      QSqlRecord record = db.record("BASE$");
      int current = 0;

      query.first();

      do {
        if (progressDialog->wasCanceled()) {
          canceled = true;
          break;
        }

        if (not query.value(record.indexOf("fornecedor")).toString().isEmpty()) {
          variantMap.insert("fornecedor", query.value(record.indexOf("fornecedor")));
          progressDialog->setValue(current++);

          leituraProduto(query, record);

          consistenciaDados();

          QSqlQuery produto;
          verificaSeProdutoJaCadastrado(produto);

          if (produto.next()) {
            QString idProduto = produto.value("idProduto").toString();
            // these are the slow functions
            atualizaCamposProduto(produto, idProduto);
            guardaNovoPrecoValidade(produto, idProduto);
            marcaProdutoNaoDescontinuado(produto, idProduto);
          } else {
            cadastraProduto();
          }
        }
      } while (query.next());

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

  db.close();

  if (not canceled) {
    showMaximized();
    ui->tableProdutos->resizeColumnsToContents();

    QString resultado = "Produtos importados: " + QString::number(itensImported) + "\nProdutos atualizados: " +
                        QString::number(itensUpdated) + "\nNão modificados: " + QString::number(itensNotChanged) +
                        "\nDescontinuados: " + QString::number(itensExpired) + "\nCom erro: " +
                        QString::number(itensError);
    QMessageBox::information(this, "Resultado", resultado);
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

  setWindowTitle(file);

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

  model.setHeaderData(model.fieldIndex("fornecedor"), Qt::Horizontal, "Fornecedor");
  model.setHeaderData(model.fieldIndex("descricao"), Qt::Horizontal, "Descrição");
  model.setHeaderData(model.fieldIndex("un"), Qt::Horizontal, "Un.");
  model.setHeaderData(model.fieldIndex("colecao"), Qt::Horizontal, "Coleção");
  model.setHeaderData(model.fieldIndex("tipo"), Qt::Horizontal, "Tipo");
  model.setHeaderData(model.fieldIndex("m2cx"), Qt::Horizontal, "M./Cx.");
  model.setHeaderData(model.fieldIndex("pccx"), Qt::Horizontal, "Pç./Cx.");
  model.setHeaderData(model.fieldIndex("kgcx"), Qt::Horizontal, "Kg./Cx.");
  model.setHeaderData(model.fieldIndex("formComercial"), Qt::Horizontal, "Form. Com.");
  model.setHeaderData(model.fieldIndex("codComercial"), Qt::Horizontal, "Cód. Com.");
  model.setHeaderData(model.fieldIndex("codBarras"), Qt::Horizontal, "Cód. Barras");
  model.setHeaderData(model.fieldIndex("ncm"), Qt::Horizontal, "NCM");
  model.setHeaderData(model.fieldIndex("ncmEx"), Qt::Horizontal, "NCM EX");
  model.setHeaderData(model.fieldIndex("icms"), Qt::Horizontal, "ICMS");
  model.setHeaderData(model.fieldIndex("cst"), Qt::Horizontal, "CST");
  model.setHeaderData(model.fieldIndex("qtdPallet"), Qt::Horizontal, "Qt. Pallet");
  model.setHeaderData(model.fieldIndex("custo"), Qt::Horizontal, "Custo");
  model.setHeaderData(model.fieldIndex("ipi"), Qt::Horizontal, "IPI");
  model.setHeaderData(model.fieldIndex("st"), Qt::Horizontal, "ST");
  model.setHeaderData(model.fieldIndex("precoVenda"), Qt::Horizontal, "Preço Venda");
  model.setHeaderData(model.fieldIndex("comissao"), Qt::Horizontal, "Comissão");
  model.setHeaderData(model.fieldIndex("observacoes"), Qt::Horizontal, "Obs.");
  model.setHeaderData(model.fieldIndex("origem"), Qt::Horizontal, "Origem");
  model.setHeaderData(model.fieldIndex("ui"), Qt::Horizontal, "UI");
  model.setHeaderData(model.fieldIndex("validade"), Qt::Horizontal, "Validade");
  model.setHeaderData(model.fieldIndex("markup"), Qt::Horizontal, "Markup");

  ui->tableProdutos->setModel(proxyModel);

  ui->tableProdutos->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableProdutos->horizontalHeader()->setResizeContentsPrecision(0);

  for (int i = 1; i <= model.fieldIndex("descontinuadoUpd"); i += 2) {
    ui->tableProdutos->setColumnHidden(i, true);
  }

  ui->tableProdutos->setColumnHidden(model.fieldIndex("idProduto"), true);
  ui->tableProdutos->setColumnHidden(model.fieldIndex("idFornecedor"), true);
  ui->tableProdutos->setColumnHidden(model.fieldIndex("desativado"), true);
  ui->tableProdutos->setColumnHidden(model.fieldIndex("descontinuado"), true);
  ui->tableProdutos->setColumnHidden(model.fieldIndex("estoque"), true);
  ui->tableProdutos->setColumnHidden(model.fieldIndex("cfop"), true);
  ui->tableProdutos->setColumnHidden(model.fieldIndex("atualizarTabelaPreco"), true);
  ui->tableProdutos->setColumnHidden(model.fieldIndex("temLote"), true);
  ui->tableProdutos->setColumnHidden(model.fieldIndex("tipo"), true);
  ui->tableProdutos->setColumnHidden(model.fieldIndex("comissao"), true);
  ui->tableProdutos->setColumnHidden(model.fieldIndex("observacoes"), true);
  ui->tableProdutos->setColumnHidden(model.fieldIndex("origem"), true);
  ui->tableProdutos->setColumnHidden(model.fieldIndex("representacao"), true);

  ui->tableProdutos->setItemDelegateForColumn(model.fieldIndex("validade"), new DateFormatDelegate("dd-MM-yyyy", this));

  DoubleDelegate *doubledelegate = new DoubleDelegate(this);
  ui->tableProdutos->setItemDelegateForColumn(model.fieldIndex("m2cx"), doubledelegate);
  ui->tableProdutos->setItemDelegateForColumn(model.fieldIndex("kgcx"), doubledelegate);
  ui->tableProdutos->setItemDelegateForColumn(model.fieldIndex("qtdPallet"), doubledelegate);
  ui->tableProdutos->setItemDelegateForColumn(model.fieldIndex("custo"), doubledelegate);
  ui->tableProdutos->setItemDelegateForColumn(model.fieldIndex("precoVenda"), doubledelegate);

  PorcentagemDelegate *porcDelegate = new PorcentagemDelegate(this);
  ui->tableProdutos->setItemDelegateForColumn(model.fieldIndex("icms"), porcDelegate);
  ui->tableProdutos->setItemDelegateForColumn(model.fieldIndex("ipi"), porcDelegate);
  ui->tableProdutos->setItemDelegateForColumn(model.fieldIndex("markup"), porcDelegate);
  ui->tableProdutos->setItemDelegateForColumn(model.fieldIndex("st"), porcDelegate);
}

void ImportaProdutos::setVariantMap() {
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
  variantMap.insert("cst", QVariant(QVariant::String));
  variantMap.insert("qtdPallet", QVariant(QVariant::Double));
  variantMap.insert("custo", QVariant(QVariant::Double));
  variantMap.insert("ipi", QVariant(QVariant::Double));
  variantMap.insert("st", QVariant(QVariant::Double));
  variantMap.insert("precoVenda", QVariant(QVariant::Double));
  variantMap.insert("comissao", QVariant(QVariant::Double));
  variantMap.insert("observacoes", QVariant(QVariant::String));
  variantMap.insert("origem", QVariant(QVariant::Int));
  variantMap.insert("descontinuado", QVariant(QVariant::Int));
  variantMap.insert("representacao", QVariant(QVariant::String));
  variantMap.insert("ui", QVariant(QVariant::String));
}

void ImportaProdutos::cadastraFornecedores(QSqlQuery &query) {
  query.first();

  do {
    QString fornecedor = query.value(0).toString();

    if (not fornecedor.isEmpty()) {
      int id = buscarCadastrarFornecedor(fornecedor);
      fornecedores.insert(fornecedor, id);
    }
  } while (query.next());
}

void ImportaProdutos::mostraApenasEstesFornecedores() {
  for (int i = 0, size = fornecedores.size(); i < size; ++i) {
    if (ids.isEmpty()) {
      ids.append("idFornecedor = " + QString::number(fornecedores.values().at(i)));
    } else {
      ids.append(" OR idFornecedor = " + QString::number(fornecedores.values().at(i)));
    }
  }
}

void ImportaProdutos::marcaTodosProdutosDescontinuados() {
  for (int row = 0, rowCount = model.rowCount(); row < rowCount; ++row) {
    model.setData(model.index(row, model.fieldIndex("descontinuado")), 1);
  }

  itensExpired = model.rowCount();
}

void ImportaProdutos::contaProdutos() {
  QSqlQuery queryProdSize("SELECT COUNT(*) FROM [BASE$]", db);
  queryProdSize.first();
  progressDialog->setMaximum(queryProdSize.value(0).toInt());
}

void ImportaProdutos::consistenciaDados() {
  for (int i = 0, size = variantMap.keys().size(); i < size; ++i) {
    if (variantMap.value(variantMap.keys().at(i)).toString().contains("*")) {
      variantMap.insert(variantMap.keys().at(i), variantMap.value(variantMap.keys().at(i)).toString().remove("*"));
    }
  }

  //  if (variantMap.value("colecao").isNull()) {
  //    variantMap.insert("colecao", 0);
  //  }

  if (variantMap.value("estoque").isNull()) {
    variantMap.insert("estoque", 0);
  }

  //  if (variantMap.value("m2cx").toDouble() <= 0.0) {
  //    variantMap.insert("m2cx", 1.0);
  //  }

  //  if (variantMap.value("pccx").toInt() <= 0) {
  //    variantMap.insert("pccx", 1);
  //  }

  //  if (variantMap.value("kgcx").toDouble() <= 0.0) {
  //    variantMap.insert("kgcx", 0.0);
  //  }

  //  if (variantMap.value("formComercial").isNull()) {
  //    variantMap.insert("formComercial", "0");
  //  }

  //  if (variantMap.value("codBarras").isNull()) {
  //    variantMap.insert("codBarras", "0");
  //  }

  //  if (variantMap.value("ncm").toString().isEmpty()) {
  //    variantMap.insert("ncm", 0);
  //  }

  //  if (variantMap.value("icms").isNull()) {
  //    variantMap.insert("icms", 0);
  //  }

  if (variantMap.value("cst").isNull()) {
    variantMap.insert("cst", "000");
  }

  //  if (variantMap.value("qtdPallet").isNull()) {
  //    variantMap.insert("qtdPallet", 0);
  //  }

  //  if (variantMap.value("ipi").isNull()) {
  //    variantMap.insert("ipi", 0);
  //  }

  //  if (variantMap.value("st").isNull()) {
  //    variantMap.insert("st", 0);
  //  }

  if (variantMap.value("representacao").isNull()) {
    variantMap.insert("representacao", 0);
  }

  if (variantMap.value("descontinuado").isNull()) {
    variantMap.insert("descontinuado", 0);
  }

  if (variantMap.value("ui").isNull()) {
    variantMap.insert("ui", 0);
  }

  QString un = variantMap.value("un").toString().toLower();

  if (un == "m2") {
    variantMap.insert("un", "m2");
  } else if (un == "ml") {
    variantMap.insert("un", "ml");
  } else {
    variantMap.insert("un", "pç");
  }

  variantMap.insert("ncm", variantMap.value("ncm").toString().remove(".").remove(","));
  variantMap.insert("codBarras", variantMap.value("codBarras").toString().remove(".").remove(","));
  variantMap.insert("codComercial", variantMap.value("codComercial").toString().remove(".").remove(","));
  variantMap.insert("pccx", variantMap.value("pccx").toInt());
  variantMap.insert("precoVenda",
                    QString::number(variantMap.value("precoVenda").toString().replace(",", ".").toDouble()).toDouble());
  variantMap.insert("custo",
                    QString::number(variantMap.value("custo").toString().replace(",", ".").toDouble()).toDouble());
}

void ImportaProdutos::leituraProduto(QSqlQuery &query, QSqlRecord &record) {
  for (int i = 0, size = variantMap.keys().size(); i < size; ++i) {
    //    qDebug() << variantMap.keys().at(i) << ": " << query.value(record.indexOf(variantMap.keys().at(i)));
    variantMap.insert(variantMap.keys().at(i), query.value(record.indexOf(variantMap.keys().at(i))));
  }
}

void ImportaProdutos::atualizaCamposProduto(QSqlQuery &produto, QString idProduto) {
  bool changed = false;

  int row = model.match(model.index(0, 0), Qt::DisplayRole, idProduto).first().row();

  QString validadeStr = QDate::currentDate().addDays(validade).toString("yyyy-MM-dd");

  if (produto.value("validade") != validadeStr) {
    model.setData(model.index(row, model.fieldIndex("validade")), validadeStr);
    model.setData(model.index(row, model.fieldIndex("validadeUpd")), Yellow);
    changed = true;
  } else {
    model.setData(model.index(row, model.fieldIndex("validadeUpd")), White);
  }

  double markup = (variantMap.value("precoVenda").toDouble() / variantMap.value("custo").toDouble()) - 1.0;

  if (produto.value("markup") != markup) {
    model.setData(model.index(row, model.fieldIndex("markup")), markup);
    model.setData(model.index(row, model.fieldIndex("markupUpd")), Yellow);
    changed = true;
  } else {
    model.setData(model.index(row, model.fieldIndex("markupUpd")), White);
  }

  if (variantMap.value("ncm").toString().length() == 10) {
    QString ncmEx = variantMap.value("ncm").toString().right(2);
    variantMap.insert("ncm", variantMap.value("ncm").toString().left(8));

    if (produto.value("ncmEx") != ncmEx) {
      model.setData(model.index(row, model.fieldIndex("ncmEx")), ncmEx);
      model.setData(model.index(row, model.fieldIndex("ncmExUpd")), Yellow);
      changed = true;
    } else {
      model.setData(model.index(row, model.fieldIndex("ncmExUpd")), White);
    }
  } else {
    model.setData(model.index(row, model.fieldIndex("ncmExUpd")), White);
  }

  for (int i = 0, size = variantMap.keys().size(); i < size; ++i) {
    if (not variantMap.values().at(i).isNull() and
        produto.value(variantMap.keys().at(i)) != variantMap.values().at(i)) {
      model.setData(model.index(row, model.fieldIndex(variantMap.keys().at(i))), variantMap.values().at(i));
      model.setData(model.index(row, model.fieldIndex(variantMap.keys().at(i) + "Upd")), Yellow);
      changed = true;
    } else {
      model.setData(model.index(row, model.fieldIndex(variantMap.keys().at(i) + "Upd")), White);
    }
  }

  pintarCamposForaDoPadrao(row);

  if (changed) {
    itensUpdated++;
  } else {
    itensNotChanged++;
  }
}

void ImportaProdutos::marcaProdutoNaoDescontinuado(QSqlQuery &produto, QString idProduto) {
  produto.prepare("UPDATE Produto SET descontinuado = 0 WHERE idProduto = :idProduto");
  produto.bindValue(":idProduto", idProduto);

  if (not produto.exec()) {
    qDebug() << "Erro marcando produto atualizado como não descontinuado: " << produto.lastError();
  }

  int row = model.match(model.index(0, 0), Qt::DisplayRole, idProduto).first().row();
  model.setData(model.index(row, model.fieldIndex("descontinuado")), 0);

  itensExpired--;
}

void ImportaProdutos::guardaNovoPrecoValidade(QSqlQuery &produto, QString idProduto) {
  if (produto.value("precoVenda") != variantMap.value("precoVenda")) {
    expiraPrecosAntigos(produto, idProduto);

    produto.prepare("INSERT INTO Produto_has_Preco (idProduto, preco, validadeInicio, validadeFim) VALUES (:idProduto, "
                    ":preco, :validadeInicio, :validadeFim)");
    produto.bindValue(":idProduto", idProduto);
    produto.bindValue(":preco", variantMap.value("precoVenda"));
    produto.bindValue(":validadeInicio", QDate::currentDate().toString("yyyy-MM-dd"));
    produto.bindValue(":validadeFim", QDate::currentDate().addDays(validade).toString("yyyy-MM-dd"));

    if (not produto.exec()) {
      qDebug() << "Erro inserindo em Preço: " << produto.lastError();
    }
  }
}

void ImportaProdutos::verificaSeProdutoJaCadastrado(QSqlQuery &produto) {
  //  produto.prepare("SELECT * FROM Produto WHERE fornecedor = :fornecedor AND descricao = :descricao AND colecao = "
  //                  ":colecao AND codComercial = "
  //                  ":codComercial AND formComercial = :formComercial AND ui = :ui");
  //  produto.bindValue(":fornecedor", variantMap.value("fornecedor"));
  //  produto.bindValue(":descricao", variantMap.value("descricao"));
  //  produto.bindValue(":colecao", variantMap.value("colecao"));
  //  produto.bindValue(":codComercial", variantMap.value("codComercial"));
  //  produto.bindValue(":formComercial", variantMap.value("formComercial"));
  //  produto.bindValue(":ui", variantMap.value("ui"));

  produto.prepare("SELECT * FROM Produto WHERE fornecedor = :fornecedor AND codComercial = "
                  ":codComercial AND ui = :ui");
  produto.bindValue(":fornecedor", variantMap.value("fornecedor"));
  produto.bindValue(":descricao", variantMap.value("descricao"));
  produto.bindValue(":colecao", variantMap.value("colecao"));
  produto.bindValue(":codComercial", variantMap.value("codComercial"));
  produto.bindValue(":formComercial", variantMap.value("formComercial"));
  produto.bindValue(":ui", variantMap.value("ui"));

  if (not produto.exec()) {
    qDebug() << "Erro buscando produto: " << produto.lastError();
  }
}

void ImportaProdutos::pintarCamposForaDoPadrao(int row) {
  // Fora do padrão
  if (variantMap.value("ncm").toString() == "0" or variantMap.value("ncm").toString().isEmpty() or
      (variantMap.value("ncm").toString().length() != 8 and variantMap.value("ncm").toString().length() != 10)) {
    model.setData(model.index(row, model.fieldIndex("ncmUpd")), Gray);
  }

  if (variantMap.value("codBarras").toString() == "0" or variantMap.value("codBarras").toString().isEmpty()) {
    model.setData(model.index(row, model.fieldIndex("codBarrasUpd")), Gray);
  }

  if (variantMap.value("codComercial").toString() == "0" or variantMap.value("codComercial").toString().isEmpty()) {
    model.setData(model.index(row, model.fieldIndex("codComercialUpd")), Gray);
  }

  // Errados
  if (variantMap.value("custo") <= 0.0) {
    model.setData(model.index(row, model.fieldIndex("custoUpd")), Red);
    hasError = true;
    itensError++;
  }

  if (variantMap.value("precoVenda") <= 0.0) {
    model.setData(model.index(row, model.fieldIndex("precoVendaUpd")), Red);
    hasError = true;
    itensError++;
  }
}

void ImportaProdutos::cadastraProduto() {
  int row = model.rowCount();
  model.insertRow(row);

  model.setData(model.index(row, model.fieldIndex("idFornecedor")),
                fornecedores.value(variantMap.value("fornecedor").toString()));

  model.setData(model.index(row, model.fieldIndex("atualizarTabelaPreco")), true);
  model.setData(model.index(row, model.fieldIndex("validade")),
                QDate::currentDate().addDays(validade).toString("yyyy-MM-dd"));
  model.setData(model.index(row, model.fieldIndex("validadeUpd")), Green);

  double markup = (variantMap.value("precoVenda").toDouble() / variantMap.value("custo").toDouble());
  model.setData(model.index(row, model.fieldIndex("markup")), markup);
  model.setData(model.index(row, model.fieldIndex("markupUpd")), Green);

  if (variantMap.value("ncm").toString().length() == 10) {
    model.setData(model.index(row, model.fieldIndex("ncmEx")), variantMap.value("ncm").toString().right(2));
    model.setData(model.index(row, model.fieldIndex("ncmExUpd")), Green);
    variantMap.insert("ncm", variantMap.value("ncm").toString().left(8));
  } else {
    model.setData(model.index(row, model.fieldIndex("ncmEx")), 0);
    model.setData(model.index(row, model.fieldIndex("ncmExUpd")), Green);
  }

  for (int i = 0, size = variantMap.keys().size(); i < size; ++i) {
    model.setData(model.index(row, model.fieldIndex(variantMap.keys().at(i))), variantMap.values().at(i));
    model.setData(model.index(row, model.fieldIndex(variantMap.keys().at(i) + "Upd")), Green);
  }

  pintarCamposForaDoPadrao(row);

  itensImported++;
}

int ImportaProdutos::buscarCadastrarFornecedor(QString fornecedor) {
  int idFornecedor = 0;

  QSqlQuery queryFornecedor;
  queryFornecedor.prepare("SELECT * FROM Fornecedor WHERE razaoSocial = :razaoSocial");
  queryFornecedor.bindValue(":razaoSocial", fornecedor);

  if (not queryFornecedor.exec()) {
    qDebug() << "Erro buscando fornecedor: " << queryFornecedor.lastError();
  }

  if (queryFornecedor.next()) {
    return queryFornecedor.value("idFornecedor").toInt();
  } else {
    queryFornecedor.prepare("INSERT INTO Fornecedor (razaoSocial) VALUES (:razaoSocial)");
    queryFornecedor.bindValue(":razaoSocial", fornecedor);

    if (not queryFornecedor.exec()) {
      qDebug() << "Erro cadastrando fornecedor: " << queryFornecedor.lastError();
    } else {
      return queryFornecedor.lastInsertId().toInt();
    }
  }

  return idFornecedor;
}

void ImportaProdutos::on_pushButtonCancelar_clicked() {
  QSqlQuery("ROLLBACK").exec();
  close();
}

void ImportaProdutos::salvar() {
  if (model.submitAll()) {
    QSqlQuery("COMMIT").exec();

    QSqlQuery queryPrecos;
    queryPrecos.prepare("INSERT INTO Produto_has_Preco (idProduto, preco, validadeInicio, validadeFim) SELECT "
                        "idProduto, precoVenda, :validadeInicio AS validadeInicio, :validadeFim AS validadeFim FROM "
                        "Produto WHERE atualizarTabelaPreco = TRUE");
    queryPrecos.bindValue(":validadeInicio", QDate::currentDate().toString("yyyy-MM-dd"));
    queryPrecos.bindValue(":validadeFim", QDate::currentDate().addDays(validade).toString("yyyy-MM-dd"));

    if (queryPrecos.exec()) {
      queryPrecos.exec("UPDATE Produto SET atualizarTabelaPreco = FALSE");
      queryPrecos.exec("COMMIT");
    } else {
      qDebug() << "Erro inserindo preços: " << queryPrecos.lastError();
      qDebug() << "query: " << queryPrecos.lastQuery();
    }

    close();

  } else {
    qDebug() << "Erro submetendo model: " << model.lastError();
    qDebug() << "query: " << model.query().lastQuery();
    QMessageBox::warning(this, "Aviso!", "Ocorreu um erro: " + model.lastError().text());
    QSqlQuery("ROLLBACK").exec();
  }
}

void ImportaProdutos::on_pushButtonSalvar_clicked() {
  if (hasError) {
    QMessageBox msgBox(QMessageBox::Warning, "Atenção!",
                       "Tabela possui campos com valores errados. Ainda deseja salvar?",
                       QMessageBox::Yes | QMessageBox::No, this);
    msgBox.setButtonText(QMessageBox::Yes, "Sim");
    msgBox.setButtonText(QMessageBox::No, "Não");

    if (msgBox.exec() == QMessageBox::Yes) {
      salvar();
    }
  } else {
    salvar();
  }
}

bool ImportaProdutos::verificaTabela() {
  QSqlRecord record = db.record("BASE$");

  for (int i = 0; i < variantMap.size(); ++i) {
    if (not record.contains(variantMap.keys().at(i))) {
      QMessageBox::warning(this, "Aviso!", "Tabela não possui coluna \"" + variantMap.keys().at(i) + "\"");
      return false;
    }
  }

  return true;
}

void ImportaProdutos::on_checkBoxRepresentacao_clicked(bool checked) {
  if (checked) {
    for (int i = 0; i < model.rowCount(); ++i) {
      model.setData(model.index(i, model.fieldIndex("representacao")), 1);
    }
  } else {
    for (int i = 0; i < model.rowCount(); ++i) {
      model.setData(model.index(i, model.fieldIndex("representacao")), 0);
    }
  }
}

#ifdef TEST

void ImportaProdutos::TestImportacao() {
  file = "C:/temp/build-Loja-Desktop_Qt_5_4_0_MinGW_32bit-Test/Bellinzoni.xlsx";
  validade = 10;

  importarTabela();
  on_pushButtonSalvar_clicked();
}

#endif
