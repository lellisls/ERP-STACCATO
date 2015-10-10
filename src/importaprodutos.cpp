#include <QSqlError>
#include <QSqlQuery>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlRecord>
#include <QSqlDriver>
#include <QDebug>

#include "importaprodutos.h"
#include "ui_importaprodutos.h"
#include "importaprodutosproxy.h"
#include "dateformatdelegate.h"
#include "validadedialog.h"
#include "doubledelegate.h"
#include "porcentagemdelegate.h"

ImportaProdutos::ImportaProdutos(QWidget *parent) : QDialog(parent), ui(new Ui::ImportaProdutos) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setVariantMap();
  setProgressDialog();
  setupTables();
}

ImportaProdutos::~ImportaProdutos() { delete ui; }

void ImportaProdutos::expiraPrecosAntigos(QSqlQuery &produto, const QString idProduto) {
  produto.prepare("UPDATE produto_has_preco SET expirado = TRUE WHERE idProduto = :idProduto");
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
    const QSqlRecord record = db.record("BASE$");

    if (not verificaTabela(record)) {
      db.close();
      return;
    }

    QSqlQuery query("SELECT * FROM [BASE$]", db);

    cadastraFornecedores(query);

    if (fornecedores.size() > 0) {
      mostraApenasEstesFornecedores();
      model.setFilter(ids);

      QString red = QString::number(Red);
      modelErro.setFilter("(" + ids + ") AND (m2cxUpd = " + red + " OR pccxUpd = " + red + " OR codComercialUpd = " +
                          red + " OR custoUpd = " + red + " OR precoVendaUpd = " + red + ")");

      if (not model.select()) {
        qDebug() << "erro model: " << model.lastError();
        return;
      }

      marcaTodosProdutosDescontinuados();
      contaProdutos();

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
          verificaSeProdutoJaCadastradoNoBD(produto);

          if (produto.next()) {
            const QString idProduto = produto.value("idProduto").toString();
            // these are the slow functions
            atualizaCamposProduto(produto, idProduto);
            guardaNovoPrecoValidade(produto, idProduto);
            marcaProdutoNaoDescontinuado(produto, idProduto);
          } else {
            verificaSeProdutoJaCadastradoNoModel();
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
    show();
    ui->tableProdutos->resizeColumnsToContents();

    const QString resultado = "Produtos importados: " + QString::number(itensImported) + "\nProdutos atualizados: " +
                              QString::number(itensUpdated) + "\nNão modificados: " + QString::number(itensNotChanged) +
                              "\nDescontinuados: " + QString::number(itensExpired) + "\nCom erro: " +
                              QString::number(itensError);
    QMessageBox::information(0, "Resultado", resultado);
  }
}

void ImportaProdutos::setProgressDialog() {
  progressDialog = new QProgressDialog(this);
  progressDialog->reset(); // NOTE: Qt 5.5 bug
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

void ImportaProdutos::setupTables() {
  model.setTable("produto");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not model.select()) {
    qDebug() << "erro model: " << model.lastError();
    return;
  }

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

  ui->tableProdutos->setModel(new ImportaProdutosProxy(&model, model.fieldIndex("descontinuado"), this));

  ui->tableProdutos->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableProdutos->horizontalHeader()->setResizeContentsPrecision(0);

  for (int i = 1, fieldIndex = model.fieldIndex("descontinuadoUpd"); i <= fieldIndex; i += 2) {
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
  ui->tableProdutos->setColumnHidden(model.fieldIndex("icms"), true);
  ui->tableProdutos->setColumnHidden(model.fieldIndex("cst"), true);
  ui->tableProdutos->setColumnHidden(model.fieldIndex("ipi"), true);
  ui->tableProdutos->setColumnHidden(model.fieldIndex("st"), true);

  ui->tableProdutos->setItemDelegateForColumn(model.fieldIndex("validade"), new DateFormatDelegate("dd-MM-yyyy", this));

  DoubleDelegate *doubledelegate = new DoubleDelegate(this);
  ui->tableProdutos->setItemDelegateForColumn(model.fieldIndex("m2cx"), doubledelegate);
  ui->tableProdutos->setItemDelegateForColumn(model.fieldIndex("kgcx"), doubledelegate);
  ui->tableProdutos->setItemDelegateForColumn(model.fieldIndex("qtdPallet"), doubledelegate);
  ui->tableProdutos->setItemDelegateForColumn(model.fieldIndex("custo"), doubledelegate);
  ui->tableProdutos->setItemDelegateForColumn(model.fieldIndex("precoVenda"), doubledelegate);

  PorcentagemDelegate *porcDelegate = new PorcentagemDelegate(true, this);
  ui->tableProdutos->setItemDelegateForColumn(model.fieldIndex("icms"), porcDelegate);
  ui->tableProdutos->setItemDelegateForColumn(model.fieldIndex("ipi"), porcDelegate);
  ui->tableProdutos->setItemDelegateForColumn(model.fieldIndex("markup"), porcDelegate);
  ui->tableProdutos->setItemDelegateForColumn(model.fieldIndex("st"), porcDelegate);

  //-------------------------------------------------------------//

  modelErro.setTable("produto");
  modelErro.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelErro.select()) {
    qDebug() << "erro model: " << modelErro.lastError();
    return;
  }

  modelErro.setHeaderData(modelErro.fieldIndex("fornecedor"), Qt::Horizontal, "Fornecedor");
  modelErro.setHeaderData(modelErro.fieldIndex("descricao"), Qt::Horizontal, "Descrição");
  modelErro.setHeaderData(modelErro.fieldIndex("un"), Qt::Horizontal, "Un.");
  modelErro.setHeaderData(modelErro.fieldIndex("colecao"), Qt::Horizontal, "Coleção");
  modelErro.setHeaderData(modelErro.fieldIndex("tipo"), Qt::Horizontal, "Tipo");
  modelErro.setHeaderData(modelErro.fieldIndex("m2cx"), Qt::Horizontal, "M./Cx.");
  modelErro.setHeaderData(modelErro.fieldIndex("pccx"), Qt::Horizontal, "Pç./Cx.");
  modelErro.setHeaderData(modelErro.fieldIndex("kgcx"), Qt::Horizontal, "Kg./Cx.");
  modelErro.setHeaderData(modelErro.fieldIndex("formComercial"), Qt::Horizontal, "Form. Com.");
  modelErro.setHeaderData(modelErro.fieldIndex("codComercial"), Qt::Horizontal, "Cód. Com.");
  modelErro.setHeaderData(modelErro.fieldIndex("codBarras"), Qt::Horizontal, "Cód. Barras");
  modelErro.setHeaderData(modelErro.fieldIndex("ncm"), Qt::Horizontal, "NCM");
  modelErro.setHeaderData(modelErro.fieldIndex("ncmEx"), Qt::Horizontal, "NCM EX");
  modelErro.setHeaderData(modelErro.fieldIndex("icms"), Qt::Horizontal, "ICMS");
  modelErro.setHeaderData(modelErro.fieldIndex("cst"), Qt::Horizontal, "CST");
  modelErro.setHeaderData(modelErro.fieldIndex("qtdPallet"), Qt::Horizontal, "Qt. Pallet");
  modelErro.setHeaderData(modelErro.fieldIndex("custo"), Qt::Horizontal, "Custo");
  modelErro.setHeaderData(modelErro.fieldIndex("ipi"), Qt::Horizontal, "IPI");
  modelErro.setHeaderData(modelErro.fieldIndex("st"), Qt::Horizontal, "ST");
  modelErro.setHeaderData(modelErro.fieldIndex("precoVenda"), Qt::Horizontal, "Preço Venda");
  modelErro.setHeaderData(modelErro.fieldIndex("comissao"), Qt::Horizontal, "Comissão");
  modelErro.setHeaderData(modelErro.fieldIndex("observacoes"), Qt::Horizontal, "Obs.");
  modelErro.setHeaderData(modelErro.fieldIndex("origem"), Qt::Horizontal, "Origem");
  modelErro.setHeaderData(modelErro.fieldIndex("ui"), Qt::Horizontal, "UI");
  modelErro.setHeaderData(modelErro.fieldIndex("validade"), Qt::Horizontal, "Validade");
  modelErro.setHeaderData(modelErro.fieldIndex("markup"), Qt::Horizontal, "Markup");

  ui->tableErro->setModel(new ImportaProdutosProxy(&modelErro, modelErro.fieldIndex("descontinuado"), this));

  ui->tableErro->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableErro->horizontalHeader()->setResizeContentsPrecision(0);

  for (int i = 1, fieldIndex = modelErro.fieldIndex("descontinuadoUpd"); i <= fieldIndex; i += 2) {
    ui->tableErro->setColumnHidden(i, true);
  }

  ui->tableErro->setColumnHidden(modelErro.fieldIndex("idProduto"), true);
  ui->tableErro->setColumnHidden(modelErro.fieldIndex("idFornecedor"), true);
  ui->tableErro->setColumnHidden(modelErro.fieldIndex("desativado"), true);
  ui->tableErro->setColumnHidden(modelErro.fieldIndex("descontinuado"), true);
  ui->tableErro->setColumnHidden(modelErro.fieldIndex("estoque"), true);
  ui->tableErro->setColumnHidden(modelErro.fieldIndex("cfop"), true);
  ui->tableErro->setColumnHidden(modelErro.fieldIndex("atualizarTabelaPreco"), true);
  ui->tableErro->setColumnHidden(modelErro.fieldIndex("temLote"), true);
  ui->tableErro->setColumnHidden(modelErro.fieldIndex("tipo"), true);
  ui->tableErro->setColumnHidden(modelErro.fieldIndex("comissao"), true);
  ui->tableErro->setColumnHidden(modelErro.fieldIndex("observacoes"), true);
  ui->tableErro->setColumnHidden(modelErro.fieldIndex("origem"), true);
  ui->tableErro->setColumnHidden(modelErro.fieldIndex("representacao"), true);

  ui->tableErro->setItemDelegateForColumn(modelErro.fieldIndex("validade"), new DateFormatDelegate("dd-MM-yyyy", this));

  ui->tableErro->setItemDelegateForColumn(modelErro.fieldIndex("m2cx"), doubledelegate);
  ui->tableErro->setItemDelegateForColumn(modelErro.fieldIndex("kgcx"), doubledelegate);
  ui->tableErro->setItemDelegateForColumn(modelErro.fieldIndex("qtdPallet"), doubledelegate);
  ui->tableErro->setItemDelegateForColumn(modelErro.fieldIndex("custo"), doubledelegate);
  ui->tableErro->setItemDelegateForColumn(modelErro.fieldIndex("precoVenda"), doubledelegate);

  ui->tableErro->setItemDelegateForColumn(modelErro.fieldIndex("icms"), porcDelegate);
  ui->tableErro->setItemDelegateForColumn(modelErro.fieldIndex("ipi"), porcDelegate);
  ui->tableErro->setItemDelegateForColumn(modelErro.fieldIndex("markup"), porcDelegate);
  ui->tableErro->setItemDelegateForColumn(modelErro.fieldIndex("st"), porcDelegate);
}

void ImportaProdutos::setVariantMap() {
  variantMap.insert("fornecedor", QVariant(QVariant::String));
  variantMap.insert("descricao", QVariant(QVariant::String));
  //  variantMap.insert("estoque", QVariant(QVariant::Int));
  variantMap.insert("un", QVariant(QVariant::String));
  variantMap.insert("colecao", QVariant(QVariant::String));
  variantMap.insert("m2cx", QVariant(QVariant::Double));
  variantMap.insert("pccx", QVariant(QVariant::Int));
  variantMap.insert("kgcx", QVariant(QVariant::Double));
  variantMap.insert("formComercial", QVariant(QVariant::String));
  variantMap.insert("codComercial", QVariant(QVariant::String));
  variantMap.insert("codBarras", QVariant(QVariant::String));
  variantMap.insert("ncm", QVariant(QVariant::String));
  //  variantMap.insert("icms", QVariant(QVariant::Double));
  //  variantMap.insert("cst", QVariant(QVariant::String));
  variantMap.insert("qtdPallet", QVariant(QVariant::Double));
  variantMap.insert("custo", QVariant(QVariant::Double));
  //  variantMap.insert("ipi", QVariant(QVariant::Double));
  //  variantMap.insert("st", QVariant(QVariant::Double));
  variantMap.insert("precoVenda", QVariant(QVariant::Double));
  //  variantMap.insert("comissao", QVariant(QVariant::Double));
  //  variantMap.insert("observacoes", QVariant(QVariant::String));
  //  variantMap.insert("origem", QVariant(QVariant::Int));
  //  variantMap.insert("descontinuado", QVariant(QVariant::Int));
  //  variantMap.insert("representacao", QVariant(QVariant::String));
  variantMap.insert("ui", QVariant(QVariant::String));
}

void ImportaProdutos::cadastraFornecedores(QSqlQuery &query) {
  query.first();

  do {
    const QString fornecedor = query.value(0).toString();

    if (not fornecedor.isEmpty()) {
      const int id = buscarCadastrarFornecedor(fornecedor);
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
    model.setData(row, "descontinuado", true);
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

  //  if (variantMap.value("estoque").isNull()) {
  //    variantMap.insert("estoque", 0);
  //  }

  //  if (variantMap.value("cst").isNull()) {
  //    variantMap.insert("cst", "000");
  //  }

  //  if (variantMap.value("representacao").isNull()) {
  //    variantMap.insert("representacao", 0);
  //  }

  //  if (variantMap.value("descontinuado").isNull()) {
  //    variantMap.insert("descontinuado", 0);
  //  }

  if (variantMap.value("ui").isNull()) {
    variantMap.insert("ui", 0);
  }

  const QString un = variantMap.value("un").toString().toUpper();

  if (un == "M2" or un == "M²") {
    variantMap.insert("un", "M²");
  } else {
    variantMap.insert("un", un);
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

void ImportaProdutos::leituraProduto(const QSqlQuery &query, const QSqlRecord &record) {
  for (int i = 0, size = variantMap.keys().size(); i < size; ++i) {
    QVariant value = query.value(record.indexOf(variantMap.keys().at(i)));

    if (value.type() == QVariant::Double) {
      value = QString::number(value.toDouble(), 'f', 2).toDouble();
    }

    variantMap.insert(variantMap.keys().at(i), value);
  }
}

void ImportaProdutos::atualizaCamposProduto(const QSqlQuery &produto, const QString idProduto) {
  bool changed = false;

  const int row = model.match(model.index(0, model.fieldIndex("idProduto")), Qt::DisplayRole, idProduto, 1,
                              Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap))
                  .first()
                  .row();

  const QString validadeStr = QDate::currentDate().addDays(validade).toString("yyyy-MM-dd");

  if (produto.value("validade") != validadeStr) {
    model.setData(row, "validade", validadeStr);
    model.setData(row, "validadeUpd", Yellow);
    changed = true;
  } else {
    model.setData(row, "validadeUpd", White);
  }

  double markup = (variantMap.value("precoVenda").toDouble() / variantMap.value("custo").toDouble()) - 1.0;
  QString markupRound = QString::number(markup, 'f', 2);
  markup = markupRound.toDouble();

  if (produto.value("markup") != markup) {
    model.setData(row, "markup", markup);
    model.setData(row, "markupUpd", Yellow);
    changed = true;
  } else {
    model.setData(row, "markupUpd", White);
  }

  if (variantMap.value("ncm").toString().length() == 10) {
    const QString ncmEx = variantMap.value("ncm").toString().right(2);
    variantMap.insert("ncm", variantMap.value("ncm").toString().left(8));

    if (produto.value("ncmEx") != ncmEx) {
      model.setData(row, "ncmEx", ncmEx);
      model.setData(row, "ncmExUpd", Yellow);
      changed = true;
    } else {
      model.setData(row, "ncmExUpd", White);
    }
  } else {
    model.setData(row, "ncmExUpd", White);
  }

  for (int i = 0, size = variantMap.keys().size(); i < size; ++i) {
    if (not variantMap.values().at(i).isNull() and
        produto.value(variantMap.keys().at(i)) != variantMap.values().at(i)) {
      qDebug() << "old: " << produto.value(variantMap.keys().at(i)) << " - new: " << variantMap.values().at(i);

      model.setData(row, variantMap.keys().at(i), variantMap.values().at(i));
      model.setData(row, variantMap.keys().at(i) + "Upd", Yellow);
      changed = true;
    } else {
      model.setData(row, variantMap.keys().at(i) + "Upd", White);
    }
  }

  pintarCamposForaDoPadrao(row);

  if (changed) {
    itensUpdated++;
  } else {
    itensNotChanged++;
  }
}

void ImportaProdutos::marcaProdutoNaoDescontinuado(QSqlQuery &produto, const QString idProduto) {
  produto.prepare("UPDATE produto SET descontinuado = 0 WHERE idProduto = :idProduto");
  produto.bindValue(":idProduto", idProduto);

  if (not produto.exec()) {
    qDebug() << "Erro marcando produto atualizado como não descontinuado: " << produto.lastError();
  }

  const int row = model.match(model.index(0, model.fieldIndex("idProduto")), Qt::DisplayRole, idProduto, 1,
                              Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap))
                  .first()
                  .row();
  model.setData(row, "descontinuado", 0);

  itensExpired--;
}

void ImportaProdutos::guardaNovoPrecoValidade(QSqlQuery &produto, const QString idProduto) {
  if (produto.value("precoVenda") != variantMap.value("precoVenda")) {
    expiraPrecosAntigos(produto, idProduto);

    produto.prepare("INSERT INTO produto_has_preco (idProduto, preco, validadeInicio, validadeFim) VALUES (:idProduto, "
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

void ImportaProdutos::verificaSeProdutoJaCadastradoNoBD(QSqlQuery &produto) {
  produto.prepare("SELECT * FROM produto WHERE fornecedor = :fornecedor AND codComercial = "
                  ":codComercial AND ui = :ui");
  produto.bindValue(":fornecedor", variantMap.value("fornecedor"));
  produto.bindValue(":codComercial", variantMap.value("codComercial"));
  produto.bindValue(":ui", variantMap.value("ui"));

  if (not produto.exec()) {
    qDebug() << "Erro buscando produto: " << produto.lastError();
  }
}

bool ImportaProdutos::verificaSeProdutoJaCadastradoNoModel() {
  auto list =
      model.match(model.index(0, model.fieldIndex("codComercial")), Qt::DisplayRole, variantMap.value("codComercial"));

  if (list.size() > 0) {
    variantMap.insert("m2cx", -1);
    variantMap.insert("pccx", -1);

    return true;
  }

  return false;
}

void ImportaProdutos::pintarCamposForaDoPadrao(const int row) {
  // Fora do padrão
  if (variantMap.value("ncm").toString() == "0" or variantMap.value("ncm").toString().isEmpty() or
      (variantMap.value("ncm").toString().length() != 8 and variantMap.value("ncm").toString().length() != 10)) {
    modelErro.setData(row, "ncmUpd", Gray);
  }

  if (variantMap.value("codBarras").toString() == "0" or variantMap.value("codBarras").toString().isEmpty()) {
    modelErro.setData(row, "codBarrasUpd", Gray);
  }

  // Errados
  if ((variantMap.value("un").toString() == "M2" or variantMap.value("un").toString() == "M²") and
      variantMap.value("m2cx") <= 0.) {
    modelErro.setData(row, "m2cxUpd", Red);
    hasError = true;
    itensError++;
  }

  if (variantMap.value("un").toString() != "M2" and variantMap.value("un").toString() != "M²" and
      variantMap.value("pccx") < 1) {
    modelErro.setData(row, "pccxUpd", Red);
    hasError = true;
    itensError++;
  }

  if (variantMap.value("codComercial").toString() == "0" or variantMap.value("codComercial").toString().isEmpty()) {
    modelErro.setData(row, "codComercialUpd", Red);
    hasError = true;
    itensError++;
  }

  if (variantMap.value("custo") <= 0.) {
    modelErro.setData(row, "custoUpd", Red);
    hasError = true;
    itensError++;
  }

  if (variantMap.value("precoVenda") <= 0.) {
    modelErro.setData(row, "precoVendaUpd", Red);
    hasError = true;
    itensError++;
  }
}

bool ImportaProdutos::camposForaDoPadrao() {
  // Errados
  QString un = variantMap.value("un").toString();

  if ((un == "M2" or un == "M²" or un == "ML") and variantMap.value("m2cx") <= 0.) {
    return true;
  }

  if (un != "M2" and un != "M²" and un != "ML" and variantMap.value("pccx") < 1) {
    return true;
  }

  if (variantMap.value("codComercial").toString() == "0" or variantMap.value("codComercial").toString().isEmpty()) {
    return true;
  }

  if (variantMap.value("custo") <= 0.) {
    return true;
  }

  if (variantMap.value("precoVenda") <= 0.) {
    return true;
  }

  return false;
}

void ImportaProdutos::cadastraProduto() {
  if (camposForaDoPadrao()) {
    const int row = modelErro.rowCount();
    modelErro.insertRow(row);

    modelErro.setData(row, "idFornecedor", fornecedores.value(variantMap.value("fornecedor").toString()));

    modelErro.setData(row, "atualizarTabelaPreco", true);
    modelErro.setData(row, "validade", QDate::currentDate().addDays(validade).toString("yyyy-MM-dd"));
    modelErro.setData(row, "validadeUpd", Green);

    const double markup = (variantMap.value("precoVenda").toDouble() / variantMap.value("custo").toDouble()) - 1.0;
    modelErro.setData(row, "markup", markup);
    modelErro.setData(row, "markupUpd", Green);

    if (variantMap.value("ncm").toString().length() == 10) {
      modelErro.setData(row, "ncmEx", variantMap.value("ncm").toString().right(2));
      modelErro.setData(row, "ncmExUpd", Green);
      variantMap.insert("ncm", variantMap.value("ncm").toString().left(8));
    } else {
      modelErro.setData(row, "ncmExUpd", Green);
    }

    for (int i = 0, size = variantMap.keys().size(); i < size; ++i) {
      modelErro.setData(row, variantMap.keys().at(i), variantMap.values().at(i));
      modelErro.setData(row, variantMap.keys().at(i) + "Upd", Green);
    }

    pintarCamposForaDoPadrao(row);

  } else {
    const int row = model.rowCount();
    model.insertRow(row);

    model.setData(row, "idFornecedor", fornecedores.value(variantMap.value("fornecedor").toString()));

    model.setData(row, "atualizarTabelaPreco", true);
    model.setData(row, "validade", QDate::currentDate().addDays(validade).toString("yyyy-MM-dd"));
    model.setData(row, "validadeUpd", Green);

    const double markup = (variantMap.value("precoVenda").toDouble() / variantMap.value("custo").toDouble()) - 1.0;
    model.setData(row, "markup", markup);
    model.setData(row, "markupUpd", Green);

    if (variantMap.value("ncm").toString().length() == 10) {
      model.setData(row, "ncmEx", variantMap.value("ncm").toString().right(2));
      model.setData(row, "ncmExUpd", Green);
      variantMap.insert("ncm", variantMap.value("ncm").toString().left(8));
    } else {
      model.setData(row, "ncmExUpd", Green);
    }

    for (int i = 0, size = variantMap.keys().size(); i < size; ++i) {
      model.setData(row, variantMap.keys().at(i), variantMap.values().at(i));
      model.setData(row, variantMap.keys().at(i) + "Upd", Green);
    }
  }

  itensImported++;
}

int ImportaProdutos::buscarCadastrarFornecedor(const QString fornecedor) {
  QSqlQuery queryFornecedor;
  queryFornecedor.prepare("SELECT * FROM fornecedor WHERE razaoSocial = :razaoSocial");
  queryFornecedor.bindValue(":razaoSocial", fornecedor);

  if (not queryFornecedor.exec()) {
    qDebug() << "Erro buscando fornecedor: " << queryFornecedor.lastError();
  }

  if (queryFornecedor.next()) {
    return queryFornecedor.value("idFornecedor").toInt();
  } else {
    queryFornecedor.prepare("INSERT INTO fornecedor (razaoSocial) VALUES (:razaoSocial)");
    queryFornecedor.bindValue(":razaoSocial", fornecedor);

    if (not queryFornecedor.exec()) {
      qDebug() << "Erro cadastrando fornecedor: " << queryFornecedor.lastError();
    } else {
      return queryFornecedor.lastInsertId().toInt();
    }
  }

  return 0;
}

void ImportaProdutos::on_pushButtonCancelar_clicked() {
  QSqlQuery("ROLLBACK").exec();
  close();
}

void ImportaProdutos::salvar() {
//  for (int row = 0; row < model.rowCount(); ++row) {
//    for (int col = 1; col < model.columnCount(); col += 2) {
//      model.setData(row, col, White);
//    }
//  }

  if (model.submitAll()) {
    QSqlQuery("COMMIT").exec();

    QSqlQuery queryPrecos;
    queryPrecos.prepare("INSERT INTO produto_has_preco (idProduto, preco, validadeInicio, validadeFim) SELECT "
                        "idProduto, precoVenda, :validadeInicio AS validadeInicio, :validadeFim AS validadeFim FROM "
                        "produto WHERE atualizarTabelaPreco = TRUE");
    queryPrecos.bindValue(":validadeInicio", QDate::currentDate().toString("yyyy-MM-dd"));
    queryPrecos.bindValue(":validadeFim", QDate::currentDate().addDays(validade).toString("yyyy-MM-dd"));

    if (queryPrecos.exec()) {
      queryPrecos.exec("UPDATE produto SET atualizarTabelaPreco = FALSE");
      queryPrecos.exec("COMMIT");
    } else {
      qDebug() << "Erro inserindo preços: " << queryPrecos.lastError();
      qDebug() << "query: " << queryPrecos.lastQuery();
    }

    close();

  } else {
    qDebug() << "Erro submetendo model: " << model.lastError();
    qDebug() << "query: " << model.query().lastQuery();
    qDebug() << "Last query: "
             << model.database().driver()->sqlStatement(QSqlDriver::InsertStatement, model.tableName(), model.record(),
                                                        false);
    QMessageBox::warning(this, "Aviso!", "Ocorreu um erro: " + model.lastError().text());
    QSqlQuery("ROLLBACK").exec();
  }
}

void ImportaProdutos::on_pushButtonSalvar_clicked() {
  if (hasError) {
    QMessageBox msgBox(QMessageBox::Warning, "Atenção!", "Produtos com erro não serão salvos. Deseja continuar?",
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

bool ImportaProdutos::verificaTabela(const QSqlRecord &record) {
  for (int i = 0, size = variantMap.size(); i < size; ++i) {
    if (not record.contains(variantMap.keys().at(i))) {
      QMessageBox::warning(this, "Aviso!", "Tabela não possui coluna \"" + variantMap.keys().at(i) + "\"");
      return false;
    }
  }

  return true;
}

void ImportaProdutos::on_checkBoxRepresentacao_clicked(const bool checked) {
  for (int i = 0, rowCount = model.rowCount(); i < rowCount; ++i) {
    model.setData(i, "representacao", checked);
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

void ImportaProdutos::on_tableProdutos_entered(const QModelIndex &index) {
  Q_UNUSED(index);

  ui->tableProdutos->resizeColumnsToContents();
}

void ImportaProdutos::on_tableErro_entered(const QModelIndex &index) {
  Q_UNUSED(index);

  ui->tableErro->resizeColumnsToContents();
}

void ImportaProdutos::on_tabWidget_currentChanged(int index) {
  if (index == 0) {
    ui->tableProdutos->resizeColumnsToContents();
  }

  if (index == 1) {
    ui->tableErro->resizeColumnsToContents();
  }
}

// TODO: hide empty columns?
