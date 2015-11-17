#include <QSqlError>
#include <QSqlQuery>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlRecord>
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
  setAttribute(Qt::WA_DeleteOnClose);

  setVariantMap();
  setProgressDialog();
  setupTables();
}

ImportaProdutos::~ImportaProdutos() { delete ui; }

void ImportaProdutos::expiraPrecosAntigos() {
  QSqlQuery query;
  query.prepare("UPDATE produto_has_preco SET expirado = TRUE WHERE idProduto = :idProduto");
  query.bindValue(":idProduto", model.data(row, "idProduto"));

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro expirando preços antigos: " + query.lastError().text());
    return;
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

void ImportaProdutos::verificaSeRepresentacao() {
  QSqlQuery queryFornecedor;
  queryFornecedor.prepare("SELECT representacao FROM fornecedor WHERE razaoSocial = :razaoSocial");
  queryFornecedor.bindValue(":razaoSocial", fornecedor);

  if (not queryFornecedor.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela fornecedor: " + queryFornecedor.lastError().text());
    return;
  }

  if (queryFornecedor.first()) {
    ui->checkBoxRepresentacao->setChecked(queryFornecedor.value(0).toBool());
  }
}

void ImportaProdutos::atualizaProduto() {
  if (camposForaDoPadrao()) {
    insereEmErro();
    return;
  }

  row = hash.value(variantMap.value("fornecedor").toString() + variantMap.value("codComercial").toString() +
                   variantMap.value("ui").toString());

  atualizaCamposProduto();
  guardaNovoPrecoValidade();
  marcaProdutoNaoDescontinuado();
}

void ImportaProdutos::importarTabela() {
  bool canceled = false;

  QSqlQuery("SET AUTOCOMMIT=0").exec();
  QSqlQuery("START TRANSACTION").exec();

  db = QSqlDatabase::contains("Excel Connection") ? QSqlDatabase::database("Excel Connection")
                                                  : QSqlDatabase::addDatabase("QODBC", "Excel Connection");

  db.setDatabaseName("DRIVER={Microsoft Excel Driver (*.xls, *.xlsx, *.xlsm, *.xlsb)};DBQ=" + file);

  if (not db.open()) {
    QMessageBox::critical(this, "Erro!", "Ocorreu um erro ao abrir o arquivo, verifique se o mesmo não está aberto: " +
                          db.lastError().text());
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  const QSqlRecord record = db.record("BASE$");

  if (not verificaTabela(record)) {
    db.close();
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  cadastraFornecedores();

  if (fornecedores.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Erro ao cadastrar fornecedores.");
    db.close();
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  verificaSeRepresentacao();
  mostraApenasEstesFornecedores();

  model.setFilter(ids);

  modelErro.setFilter("(" + ids + ") AND (m2cxUpd = " + Red + " OR pccxUpd = " + Red + " OR codComercialUpd = " + Red +
                      " OR custoUpd = " + Red + " OR precoVendaUpd = " + Red + ")");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela produto: " + model.lastError().text());
    db.close();
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  for (int i = 0; i < model.rowCount(); ++i) {
    hash[model.data(i, "fornecedor").toString() + model.data(i, "codComercial").toString() +
        model.data(i, "ui").toString()] = i;
  }

  marcaTodosProdutosDescontinuados();
  contaProdutos();

  int current = 0;

  QSqlQuery query("SELECT * FROM [BASE$]", db);

  while (query.next()) {
    if (progressDialog->wasCanceled()) {
      canceled = true;
      break;
    }

    if (query.value(record.indexOf("fornecedor")).toString().isEmpty()) {
      continue;
    }

    variantMap.insert("fornecedor", query.value(record.indexOf("fornecedor")));
    progressDialog->setValue(current++);

    leituraProduto(query, record);
    consistenciaDados();

    verificaSeProdutoJaCadastrado() ? atualizaProduto() : cadastraProduto();
  }

  progressDialog->cancel();

  db.close();

  if (canceled) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  show();
  ui->tableProdutos->resizeColumnsToContents();

  const QString resultado = "Produtos importados: " + QString::number(itensImported) + "\nProdutos atualizados: " +
                            QString::number(itensUpdated) + "\nNão modificados: " + QString::number(itensNotChanged) +
                            "\nDescontinuados: " + QString::number(itensExpired) + "\nCom erro: " +
                            QString::number(itensError);
  QMessageBox::information(this, "Resultado", resultado);
}

void ImportaProdutos::setProgressDialog() {
  progressDialog = new QProgressDialog(this);
  progressDialog->reset(); // NOTE: Qt 5.5 bug https://bugreports.qt.io/browse/QTBUG-47042
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

  if (not validadeDlg->exec()) {
    return false;
  }

  validade = validadeDlg->getValidade();

  return true;
}

void ImportaProdutos::setupTables() {
  model.setTable("produto");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela produto: " + model.lastError().text());
    return;
  }

  model.setHeaderData("fornecedor", "Fornecedor");
  model.setHeaderData("descricao", "Descrição");
  model.setHeaderData("un", "Un.");
  model.setHeaderData("un2", "Un.2");
  model.setHeaderData("colecao", "Coleção");
  model.setHeaderData("tipo", "Tipo");
  model.setHeaderData("m2cx", "M./Cx.");
  model.setHeaderData("pccx", "Pç./Cx.");
  model.setHeaderData("kgcx", "Kg./Cx.");
  model.setHeaderData("formComercial", "Form. Com.");
  model.setHeaderData("codComercial", "Cód. Com.");
  model.setHeaderData("codBarras", "Cód. Barras");
  model.setHeaderData("ncm", "NCM");
  model.setHeaderData("ncmEx", "NCM EX");
  model.setHeaderData("icms", "ICMS");
  model.setHeaderData("cst", "CST");
  model.setHeaderData("qtdPallet", "Qt. Pallet");
  model.setHeaderData("custo", "Custo");
  model.setHeaderData("ipi", "IPI");
  model.setHeaderData("st", "ST");
  model.setHeaderData("precoVenda", "Preço Venda");
  model.setHeaderData("comissao", "Comissão");
  model.setHeaderData("observacoes", "Obs.");
  model.setHeaderData("origem", "Origem");
  model.setHeaderData("ui", "UI");
  model.setHeaderData("validade", "Validade");
  model.setHeaderData("markup", "Markup");

  ui->tableProdutos->setModel(new ImportaProdutosProxy(&model, model.fieldIndex("descontinuado"), this));

  for (int i = 1, fieldIndex = model.fieldIndex("descontinuadoUpd"); i <= fieldIndex; i += 2) {
    ui->tableProdutos->setColumnHidden(i, true);
  }

  ui->tableProdutos->hideColumn("idProduto");
  ui->tableProdutos->hideColumn("idFornecedor");
  ui->tableProdutos->hideColumn("desativado");
  ui->tableProdutos->hideColumn("descontinuado");
  ui->tableProdutos->hideColumn("estoque");
  ui->tableProdutos->hideColumn("cfop");
  ui->tableProdutos->hideColumn("atualizarTabelaPreco");
  ui->tableProdutos->hideColumn("temLote");
  ui->tableProdutos->hideColumn("tipo");
  ui->tableProdutos->hideColumn("comissao");
  ui->tableProdutos->hideColumn("observacoes");
  ui->tableProdutos->hideColumn("origem");
  ui->tableProdutos->hideColumn("representacao");
  ui->tableProdutos->hideColumn("icms");
  ui->tableProdutos->hideColumn("cst");
  ui->tableProdutos->hideColumn("ipi");
  ui->tableProdutos->hideColumn("st");

  ui->tableProdutos->setItemDelegateForColumn("validade", new DateFormatDelegate(this));

  DoubleDelegate *doubledelegate = new DoubleDelegate(4, this);
  ui->tableProdutos->setItemDelegateForColumn("m2cx", doubledelegate);
  ui->tableProdutos->setItemDelegateForColumn("kgcx", doubledelegate);
  ui->tableProdutos->setItemDelegateForColumn("qtdPallet", doubledelegate);
  ui->tableProdutos->setItemDelegateForColumn("custo", doubledelegate);
  ui->tableProdutos->setItemDelegateForColumn("precoVenda", doubledelegate);

  PorcentagemDelegate *porcDelegate = new PorcentagemDelegate(this);
  ui->tableProdutos->setItemDelegateForColumn("icms", porcDelegate);
  ui->tableProdutos->setItemDelegateForColumn("ipi", porcDelegate);
  ui->tableProdutos->setItemDelegateForColumn("markup", porcDelegate);
  ui->tableProdutos->setItemDelegateForColumn("st", porcDelegate);

  //-------------------------------------------------------------//

  modelErro.setTable("produto");
  modelErro.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelErro.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela produto: " + modelErro.lastError().text());
    return;
  }

  modelErro.setHeaderData("fornecedor", "Fornecedor");
  modelErro.setHeaderData("descricao", "Descrição");
  modelErro.setHeaderData("un", "Un.");
  modelErro.setHeaderData("colecao", "Coleção");
  modelErro.setHeaderData("tipo", "Tipo");
  modelErro.setHeaderData("m2cx", "M./Cx.");
  modelErro.setHeaderData("pccx", "Pç./Cx.");
  modelErro.setHeaderData("kgcx", "Kg./Cx.");
  modelErro.setHeaderData("formComercial", "Form. Com.");
  modelErro.setHeaderData("codComercial", "Cód. Com.");
  modelErro.setHeaderData("codBarras", "Cód. Barras");
  modelErro.setHeaderData("ncm", "NCM");
  modelErro.setHeaderData("ncmEx", "NCM EX");
  modelErro.setHeaderData("icms", "ICMS");
  modelErro.setHeaderData("cst", "CST");
  modelErro.setHeaderData("qtdPallet", "Qt. Pallet");
  modelErro.setHeaderData("custo", "Custo");
  modelErro.setHeaderData("ipi", "IPI");
  modelErro.setHeaderData("st", "ST");
  modelErro.setHeaderData("precoVenda", "Preço Venda");
  modelErro.setHeaderData("comissao", "Comissão");
  modelErro.setHeaderData("observacoes", "Obs.");
  modelErro.setHeaderData("origem", "Origem");
  modelErro.setHeaderData("ui", "UI");
  modelErro.setHeaderData("validade", "Validade");
  modelErro.setHeaderData("markup", "Markup");

  ui->tableErro->setModel(new ImportaProdutosProxy(&modelErro, modelErro.fieldIndex("descontinuado"), this));

  for (int i = 1, fieldIndex = modelErro.fieldIndex("descontinuadoUpd"); i <= fieldIndex; i += 2) {
    ui->tableErro->setColumnHidden(i, true);
  }

  ui->tableErro->hideColumn("idProduto");
  ui->tableErro->hideColumn("idFornecedor");
  ui->tableErro->hideColumn("desativado");
  ui->tableErro->hideColumn("descontinuado");
  ui->tableErro->hideColumn("estoque");
  ui->tableErro->hideColumn("cfop");
  ui->tableErro->hideColumn("atualizarTabelaPreco");
  ui->tableErro->hideColumn("temLote");
  ui->tableErro->hideColumn("tipo");
  ui->tableErro->hideColumn("comissao");
  ui->tableErro->hideColumn("observacoes");
  ui->tableErro->hideColumn("origem");
  ui->tableErro->hideColumn("representacao");

  ui->tableErro->setItemDelegateForColumn("validade", new DateFormatDelegate(this));

  ui->tableErro->setItemDelegateForColumn("m2cx", doubledelegate);
  ui->tableErro->setItemDelegateForColumn("kgcx", doubledelegate);
  ui->tableErro->setItemDelegateForColumn("qtdPallet", doubledelegate);
  ui->tableErro->setItemDelegateForColumn("custo", doubledelegate);
  ui->tableErro->setItemDelegateForColumn("precoVenda", doubledelegate);

  ui->tableErro->setItemDelegateForColumn("icms", porcDelegate);
  ui->tableErro->setItemDelegateForColumn("ipi", porcDelegate);
  ui->tableErro->setItemDelegateForColumn("markup", porcDelegate);
  ui->tableErro->setItemDelegateForColumn("st", porcDelegate);
}

void ImportaProdutos::setVariantMap() {
  variantMap.insert("fornecedor", QVariant(QVariant::String));
  variantMap.insert("descricao", QVariant(QVariant::String));
  variantMap.insert("un", QVariant(QVariant::String));
  variantMap.insert("colecao", QVariant(QVariant::String));
  variantMap.insert("m2cx", QVariant(QVariant::Double));
  variantMap.insert("pccx", QVariant(QVariant::Int));
  variantMap.insert("kgcx", QVariant(QVariant::Double));
  variantMap.insert("formComercial", QVariant(QVariant::String));
  variantMap.insert("codComercial", QVariant(QVariant::String));
  variantMap.insert("codBarras", QVariant(QVariant::String));
  variantMap.insert("ncm", QVariant(QVariant::String));
  variantMap.insert("qtdPallet", QVariant(QVariant::Double));
  variantMap.insert("custo", QVariant(QVariant::Double));
  variantMap.insert("precoVenda", QVariant(QVariant::Double));
  variantMap.insert("ui", QVariant(QVariant::String));
  variantMap.insert("un2", QVariant(QVariant::String));
}

void ImportaProdutos::cadastraFornecedores() {
  QSqlQuery query("SELECT DISTINCT(fornecedor) FROM [BASE$]", db);

  while (query.next()) {
    if (query.value("fornecedor").toString().isEmpty()) {
      continue;
    }

    fornecedor = query.value("fornecedor").toString();
    fornecedores.insert(fornecedor, buscarCadastrarFornecedor(fornecedor));
  }
}

void ImportaProdutos::mostraApenasEstesFornecedores() {
  for (auto fornecedor : fornecedores) {
    ids.append(QString(ids.isEmpty() ? "" : " OR ") + "idFornecedor = " + QString::number(fornecedor));
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
  for (auto key : variantMap.keys()) {
    if (variantMap.value(key).toString().contains("*")) {
      variantMap.insert(key, variantMap.value(key).toString().remove("*"));
    }
  }

  if (variantMap.value("ui").isNull()) {
    variantMap.insert("ui", 0);
  }

  const QString un = variantMap.value("un").toString().toUpper();

  (un == "M2" or un == "M²") ? variantMap.insert("un", "M²") : variantMap.insert("un", un);

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
  for (auto key : variantMap.keys()) {
    QVariant value = query.value(record.indexOf(key));

    if (value.type() == QVariant::Double) {
      value = QString::number(value.toDouble(), 'f', 4).toDouble();
    }

    variantMap.insert(key, value);
  }
}

void ImportaProdutos::atualizaCamposProduto() {
  bool changed = false;

  for (auto key : variantMap.keys()) {
    if (not variantMap.value(key).isNull() and model.data(row, key) != variantMap.value(key)) {
      //      qDebug() << "old: " << model.data(row, key) << " - new: " << variantMap.value(key);
      model.setData(row, key, variantMap.value(key));
      model.setData(row, key + "Upd", Yellow);
    } else {
      model.setData(row, key + "Upd", White);
    }
  }

  const QString validadeStr = QDate::currentDate().addDays(validade).toString("yyyy-MM-dd");

  if (model.data(row, "validade") != validadeStr) {
    model.setData(row, "validade", validadeStr);
    model.setData(row, "validadeUpd", Yellow);
    changed = true;
  } else {
    model.setData(row, "validadeUpd", White);
  }

  double markup = (variantMap.value("precoVenda").toDouble() / variantMap.value("custo").toDouble()) - 1.0;
  QString markupRound = QString::number(markup, 'f', 2);
  markup = markupRound.toDouble();

  if (model.data(row, "markup") != markup) {
    model.setData(row, "markup", markup);
    model.setData(row, "markupUpd", Yellow);
    changed = true;
  } else {
    model.setData(row, "markupUpd", White);
  }

  if (variantMap.value("ncm").toString().length() == 10) {
    const QString ncmEx = variantMap.value("ncm").toString().right(2);
    variantMap.insert("ncm", variantMap.value("ncm").toString().left(8));

    if (model.data(row, "ncmEx") != ncmEx) {
      model.setData(row, "ncmEx", ncmEx);
      model.setData(row, "ncmExUpd", Yellow);
      changed = true;
    } else {
      model.setData(row, "ncmExUpd", White);
    }
  } else {
    model.setData(row, "ncmExUpd", White);
  }

  changed ? itensUpdated++ : itensNotChanged++;
}

void ImportaProdutos::marcaProdutoNaoDescontinuado() {
  QSqlQuery query;
  query.prepare("UPDATE produto SET descontinuado = FALSE WHERE idProduto = :idProduto");
  query.bindValue(":idProduto", model.data(row, "idProduto"));

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro marcando produto atualizado como não descontiunado: " + query.lastError().text());
    return;
  }

  model.setData(row, "descontinuado", false);

  itensExpired--;
}

void ImportaProdutos::guardaNovoPrecoValidade() {
  if (model.data(row, "precoVenda") != variantMap.value("precoVenda")) {
    expiraPrecosAntigos();

    QSqlQuery query;
    query.prepare("INSERT INTO produto_has_preco (idProduto, preco, validadeInicio, validadeFim) VALUES (:idProduto, "
                  ":preco, :validadeInicio, :validadeFim)");
    query.bindValue(":idProduto", model.data(row, "idProduto"));
    query.bindValue(":preco", variantMap.value("precoVenda"));
    query.bindValue(":validadeInicio", QDate::currentDate().toString("yyyy-MM-dd"));
    query.bindValue(":validadeFim", QDate::currentDate().addDays(validade).toString("yyyy-MM-dd"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro inserindo dados em produto_has_preco: " + query.lastError().text());
      return;
    }
  }
}

void ImportaProdutos::verificaSeProdutoJaCadastradoNoModel() {
  if (hash.contains(variantMap.value("fornecedor").toString() + variantMap.value("codComercial").toString() +
                    variantMap.value("ui").toString())) {
    variantMap.insert("colecao", "REPETIDO");
    variantMap.insert("m2cx", -1);
    variantMap.insert("pccx", -1);
  }
}

bool ImportaProdutos::verificaSeProdutoJaCadastrado() {
  if (hash.contains(variantMap.value("fornecedor").toString() + variantMap.value("codComercial").toString() +
                    variantMap.value("ui").toString())) {
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

void ImportaProdutos::insereEmErro() {
  const int row = modelErro.rowCount();
  modelErro.insertRow(row);

  for (auto key : variantMap.keys()) {
    modelErro.setData(row, key, variantMap.value(key));
    modelErro.setData(row, key + "Upd", Green);
  }

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

  pintarCamposForaDoPadrao(row);
}

void ImportaProdutos::insereEmOk() {
  const int row = model.rowCount();
  model.insertRow(row);

  for (auto key : variantMap.keys()) {
    model.setData(row, key, variantMap.value(key));
    model.setData(row, key + "Upd", Green);
  }

  model.setData(row, "idFornecedor", fornecedores.value(variantMap.value("fornecedor").toString()));

  model.setData(row, "atualizarTabelaPreco", true);
  model.setData(row, "validade", QDate::currentDate().addDays(validade).toString("yyyy-MM-dd"));
  model.setData(row, "validadeUpd", Green);

  const double markup = (variantMap.value("precoVenda").toDouble() / variantMap.value("custo").toDouble()) - 1.;
  model.setData(row, "markup", markup);
  model.setData(row, "markupUpd", Green);

  if (variantMap.value("ncm").toString().length() == 10) {
    model.setData(row, "ncmEx", variantMap.value("ncm").toString().right(2));
    model.setData(row, "ncmExUpd", Green);
    variantMap.insert("ncm", variantMap.value("ncm").toString().left(8));
  } else {
    model.setData(row, "ncmExUpd", Green);
  }

  hash[model.data(row, "fornecedor").toString() + model.data(row, "codComercial").toString() +
      model.data(row, "ui").toString()] = row;
}

void ImportaProdutos::cadastraProduto() {
  verificaSeProdutoJaCadastradoNoModel();

  camposForaDoPadrao() ? insereEmErro() : insereEmOk();

  itensImported++;
}

int ImportaProdutos::buscarCadastrarFornecedor(const QString fornecedor) {
  QSqlQuery queryFornecedor;
  queryFornecedor.prepare("SELECT * FROM fornecedor WHERE razaoSocial = :razaoSocial");
  queryFornecedor.bindValue(":razaoSocial", fornecedor);

  if (not queryFornecedor.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando fornecedor: " + queryFornecedor.lastError().text());
    return 0;
  }

  if (not queryFornecedor.next()) {
    queryFornecedor.prepare("INSERT INTO fornecedor (razaoSocial) VALUES (:razaoSocial)");
    queryFornecedor.bindValue(":razaoSocial", fornecedor);

    if (not queryFornecedor.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro cadastrando fornecedor: " + queryFornecedor.lastError().text());
      return 0;
    }

    return queryFornecedor.lastInsertId().toInt();
  }

  return queryFornecedor.value("idFornecedor").toInt();
}

void ImportaProdutos::on_pushButtonCancelar_clicked() {
  QSqlQuery("ROLLBACK").exec();
  close();
}

void ImportaProdutos::salvar() {
  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Ocorreu um erro ao salvar os dados: " + model.lastError().text());
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  QSqlQuery queryPrecos;
  queryPrecos.prepare("INSERT INTO produto_has_preco (idProduto, preco, validadeInicio, validadeFim) SELECT "
                      "idProduto, precoVenda, :validadeInicio AS validadeInicio, :validadeFim AS validadeFim FROM "
                      "produto WHERE atualizarTabelaPreco = TRUE");
  queryPrecos.bindValue(":validadeInicio", QDate::currentDate().toString("yyyy-MM-dd"));
  queryPrecos.bindValue(":validadeFim", QDate::currentDate().addDays(validade).toString("yyyy-MM-dd"));

  if (not queryPrecos.exec()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro inserindo dados em produto_has_preco: " + queryPrecos.lastError().text());
    return;
  }

  queryPrecos.exec("UPDATE produto SET atualizarTabelaPreco = FALSE");
  queryPrecos.exec("COMMIT");

  close();
}

void ImportaProdutos::on_pushButtonSalvar_clicked() {
  if (hasError) {
    QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Produtos com erro não serão salvos. Deseja continuar?",
                       QMessageBox::Yes | QMessageBox::No, this);
    msgBox.setButtonText(QMessageBox::Yes, "Sim");
    msgBox.setButtonText(QMessageBox::No, "Não");

    if (msgBox.exec() == QMessageBox::Yes) {
      salvar();
    }

    return;
  }

  salvar();
}

bool ImportaProdutos::verificaTabela(const QSqlRecord &record) {
  for (auto key : variantMap.keys()) {
    if (not record.contains(key)) {
      QMessageBox::critical(this, "Erro!", "Tabela não possui coluna \"" + key + "\"");
      return false;
    }
  }

  return true;
}

void ImportaProdutos::on_checkBoxRepresentacao_clicked(const bool checked) {
  for (int i = 0, rowCount = model.rowCount(); i < rowCount; ++i) {
    model.setData(i, "representacao", checked);
  }

  QSqlQuery query;
  query.prepare("UPDATE fornecedor SET representacao = TRUE WHERE razaoSocial = :razaoSocial");
  query.bindValue(":razaoSocial", fornecedor);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro guardando Representacao em Fornecedor: " + query.lastError().text());
    return;
  }
}

void ImportaProdutos::on_tableProdutos_entered(const QModelIndex &) { ui->tableProdutos->resizeColumnsToContents(); }

void ImportaProdutos::on_tableErro_entered(const QModelIndex &) { ui->tableErro->resizeColumnsToContents(); }

void ImportaProdutos::on_tabWidget_currentChanged(int index) {
  if (index == 0) {
    ui->tableProdutos->resizeColumnsToContents();
  }

  if (index == 1) {
    ui->tableErro->resizeColumnsToContents();
  }
}

// TODO: colocar um sistema que importa a tabela toda para verificar erros, pergunta se quer remover os erros e modifica
// o excel, depois faz a importacao normal?
