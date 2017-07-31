#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include "dateformatdelegate.h"
#include "doubledelegate.h"
#include "importaprodutos.h"
#include "importaprodutosproxy.h"
#include "porcentagemdelegate.h"
#include "ui_importaprodutos.h"
#include "validadedialog.h"

ImportaProdutos::ImportaProdutos(QWidget *parent) : QDialog(parent), ui(new Ui::ImportaProdutos) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setVariantMap();
  setProgressDialog();
  setupTables();
}

ImportaProdutos::~ImportaProdutos() { delete ui; }

bool ImportaProdutos::expiraPrecosAntigos() {
  QSqlQuery query;
  query.prepare("UPDATE produto_has_preco SET expirado = TRUE WHERE idProduto = :idProduto");
  query.bindValue(":idProduto", model.data(row, "idProduto"));

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro expirando preços antigos: " + query.lastError().text());
    return false;
  }

  return true;
}

void ImportaProdutos::importarProduto() {
  if (not readFile()) return;
  if (not readValidade()) return;

  tipo = Produto;
  importarTabela();
}

void ImportaProdutos::importarEstoque() {
  QMessageBox::critical(this, "Erro!", "Temporariamente desativado!");
  return;

  // NOTE: ao inves de cadastrar uma tabela de estoque, quando o estoque for gerado (importacao de xml) criar uma linha
  // correspondente na tabela produto com flag estoque, esse produto vai ser listado junto dos outros porem com cor
  // diferente

  // estoques gerados por tabela nao terao dados de impostos enquanto os de xml sim

  // obs1: o orcamento nao gera pré-consumo mas ao fechar pedido o estoque pode não estar mais disponivel
  // obs2: quando fechar pedido gerar pré-consumo
  // obs3: quando fechar pedido mudar status de 'pendente' para 'estoque' para nao aparecer na tela de compras
  // obs4: colocar na tabela produto um campo para indicar qual o estoque relacionado

  if (not readFile()) return;

  validade = 730;
  tipo = Estoque;

  importarTabela();
}

void ImportaProdutos::importarPromocao() {
  if (not readFile()) return;
  if (not readValidade()) return;

  tipo = Promocao;
  importarTabela();
}

bool ImportaProdutos::verificaSeRepresentacao() {
  QSqlQuery queryFornecedor;
  queryFornecedor.prepare("SELECT representacao FROM fornecedor WHERE razaoSocial = :razaoSocial");
  queryFornecedor.bindValue(":razaoSocial", fornecedor);

  if (not queryFornecedor.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela fornecedor: " + queryFornecedor.lastError().text());
    return false;
  }

  if (queryFornecedor.first()) ui->checkBoxRepresentacao->setChecked(queryFornecedor.value("representacao").toBool());

  return true;
}

bool ImportaProdutos::atualizaProduto() {
  if (camposForaDoPadrao()) return insereEmErro();

  row = hash.value(variantMap.value("fornecedor").toString() + variantMap.value("codComercial").toString() + variantMap.value("ui").toString() + QString::number(tipo));

  if (hashAtualizado.value(row) == true) {
    variantMap.insert("colecao", "REPETIDO");
    return insereEmErro();
  }

  hashAtualizado[row] = true;

  if (not atualizaCamposProduto()) return false;
  if (not guardaNovoPrecoValidade()) return false;
  if (not marcaProdutoNaoDescontinuado()) return false;

  return true;
}

bool ImportaProdutos::importar() {
  db = QSqlDatabase::contains("Excel Connection") ? QSqlDatabase::database("Excel Connection") : QSqlDatabase::addDatabase("QODBC", "Excel Connection");

  db.setDatabaseName("DRIVER={Microsoft Excel Driver (*.xls, *.xlsx, *.xlsm, *.xlsb)};HDR=Yes;MaxScanRows=0;DBQ=" + file);

  if (not db.open()) {
    QMessageBox::critical(this, "Erro!", "Ocorreu um erro ao abrir o arquivo, verifique se o mesmo não está aberto: " + db.lastError().text());
    return false;
  }

  const QSqlRecord record = db.record("BASE$");

  if (not verificaTabela(record)) return false;
  if (not cadastraFornecedores()) return false;
  mostraApenasEstesFornecedores();
  if (not verificaSeRepresentacao()) return false;
  if (not marcaTodosProdutosDescontinuados()) return false;

  model.setFilter("idFornecedor IN (" + idsFornecedor.join(",") + ") AND estoque_promocao = " + QString::number(tipo));

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela produto: " + model.lastError().text());
    return false;
  }

  const QString red = QString::number(Red);

  modelErro.setFilter("idFornecedor IN (" + idsFornecedor.join(",") + ") AND estoque_promocao = " + QString::number(tipo) + " AND (m2cxUpd = " + red + " OR pccxUpd = " + red +
                      " OR codComercialUpd = " + red + " OR custoUpd = " + red + " OR precoVendaUpd = " + red + ")");

  if (not modelErro.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela erro: " + modelErro.lastError().text());
  }

  itensExpired = model.rowCount();

  for (int row = 0, rowCount = model.rowCount(); row < rowCount; ++row) {
    hash[model.data(row, "fornecedor").toString() + model.data(row, "codComercial").toString() + model.data(row, "ui").toString() + model.data(row, "estoque_promocao").toString()] = row;
  }

  contaProdutos();

  int current = 0;

  QSqlQuery query("SELECT * FROM [BASE$]", db);

  bool canceled = false;

  while (query.next()) {
    if (progressDialog->wasCanceled()) {
      canceled = true;
      break;
    }

    if (query.value(record.indexOf("fornecedor")).toString().isEmpty()) continue;

    variantMap.insert("fornecedor", query.value(record.indexOf("fornecedor")));
    progressDialog->setValue(current++);

    leituraProduto(query, record);
    consistenciaDados();

    if (not(verificaSeProdutoJaCadastrado() ? atualizaProduto() : cadastraProduto())) return false;
  }

  progressDialog->cancel();

  if (canceled) return false;

  show();
  ui->tableProdutos->resizeColumnsToContents();

  db.close();

  const QString resultado = "Produtos importados: " + QString::number(itensImported) + "\nProdutos atualizados: " + QString::number(itensUpdated) + "\nNão modificados: " +
                            QString::number(itensNotChanged) + "\nDescontinuados: " + QString::number(itensExpired) + "\nCom erro: " + QString::number(itensError);
  QMessageBox::information(this, "Resultado", resultado);

  return true;
}

void ImportaProdutos::importarTabela() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not importar()) {
    QSqlQuery("ROLLBACK").exec();
    close();
  }
}

void ImportaProdutos::setProgressDialog() {
  progressDialog = new QProgressDialog(this);
  progressDialog->reset();
  progressDialog->setCancelButton(nullptr);
  progressDialog->setLabelText("Importando...");
  progressDialog->setWindowTitle("ERP Staccato");
  progressDialog->setWindowModality(Qt::WindowModal);
  progressDialog->setMinimum(0);
  progressDialog->setMaximum(0);
  progressDialog->setCancelButtonText("Cancelar");
}

bool ImportaProdutos::readFile() {
  file = QFileDialog::getOpenFileName(this, "Importar tabela genérica", QDir::currentPath(), tr("Excel (*.xlsx)"));

  if (file.isEmpty()) return false;

  setWindowTitle(file);

  return true;
}

bool ImportaProdutos::readValidade() {
  auto *validadeDlg = new ValidadeDialog();

  if (not validadeDlg->exec()) return false;

  validade = validadeDlg->getValidade();

  return true;
}

void ImportaProdutos::setupTables() {
  model.setTable("produto");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  model.setHeaderData("fornecedor", "Fornecedor");
  model.setHeaderData("descricao", "Descrição");
  model.setHeaderData("un", "Un.");
  model.setHeaderData("un2", "Un.2");
  model.setHeaderData("colecao", "Coleção");
  model.setHeaderData("tipo", "Tipo");
  model.setHeaderData("minimo", "Mínimo");
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

  ui->tableProdutos->setModel(new ImportaProdutosProxy(&model, this));

  for (int column = 0; column < model.columnCount(); ++column) {
    if (model.record().fieldName(column).endsWith("Upd")) ui->tableProdutos->setColumnHidden(column, true);
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
  ui->tableProdutos->hideColumn("estoque_promocao");
  ui->tableProdutos->hideColumn("idProdutoRelacionado");

  ui->tableProdutos->setItemDelegateForColumn("validade", new DateFormatDelegate(this));

  auto *doubledelegate = new DoubleDelegate(this, 4);
  ui->tableProdutos->setItemDelegateForColumn("m2cx", doubledelegate);
  ui->tableProdutos->setItemDelegateForColumn("kgcx", doubledelegate);
  ui->tableProdutos->setItemDelegateForColumn("qtdPallet", doubledelegate);
  ui->tableProdutos->setItemDelegateForColumn("custo", doubledelegate);
  ui->tableProdutos->setItemDelegateForColumn("precoVenda", doubledelegate);

  auto *porcDelegate = new PorcentagemDelegate(this);
  ui->tableProdutos->setItemDelegateForColumn("icms", porcDelegate);
  ui->tableProdutos->setItemDelegateForColumn("ipi", porcDelegate);
  ui->tableProdutos->setItemDelegateForColumn("markup", porcDelegate);
  ui->tableProdutos->setItemDelegateForColumn("st", porcDelegate);

  //-------------------------------------------------------------//

  modelErro.setTable("produto");
  modelErro.setEditStrategy(QSqlTableModel::OnManualSubmit);

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

  ui->tableErro->setModel(new ImportaProdutosProxy(&modelErro, this));

  for (int column = 0; column < modelErro.columnCount(); ++column) {
    if (modelErro.record().fieldName(column).endsWith("Upd")) ui->tableErro->setColumnHidden(column, true);
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
  ui->tableErro->hideColumn("icms");
  ui->tableErro->hideColumn("cst");
  ui->tableErro->hideColumn("ipi");
  ui->tableErro->hideColumn("st");
  ui->tableErro->hideColumn("estoque_promocao");
  ui->tableErro->hideColumn("idProdutoRelacionado");

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
  variantMap.insert("pccx", QVariant(QVariant::Double));
  variantMap.insert("kgcx", QVariant(QVariant::Double));
  variantMap.insert("minimo", QVariant(QVariant::Double));
  variantMap.insert("formComercial", QVariant(QVariant::String));
  variantMap.insert("codComercial", QVariant(QVariant::String));
  variantMap.insert("codBarras", QVariant(QVariant::String));
  variantMap.insert("ncm", QVariant(QVariant::String));
  variantMap.insert("qtdPallet", QVariant(QVariant::Double));
  variantMap.insert("custo", QVariant(QVariant::Double));
  variantMap.insert("precoVenda", QVariant(QVariant::Double));
  variantMap.insert("ui", QVariant(QVariant::String));
  variantMap.insert("un2", QVariant(QVariant::String));
  variantMap.insert("estoque_promocao", QVariant(QVariant::String));
  variantMap.insert("idProdutoRelacionado", QVariant(QVariant::Int));
}

bool ImportaProdutos::cadastraFornecedores() {
  QSqlQuery query("SELECT DISTINCT(fornecedor) FROM [BASE$]", db);

  while (query.next()) {
    if (query.value("fornecedor").toString().isEmpty()) continue;

    fornecedor = query.value("fornecedor").toString();

    QSqlQuery queryFornecedor;
    queryFornecedor.prepare("UPDATE fornecedor SET validadeProdutos = :validade WHERE razaoSocial = :razaoSocial");
    queryFornecedor.bindValue(":validade", QDate::currentDate().addDays(validade));
    queryFornecedor.bindValue(":razaoSocial", fornecedor);

    if (not queryFornecedor.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando validade: " + queryFornecedor.lastError().text());
      return false;
    }

    int idFornecedor;
    if (not buscarCadastrarFornecedor(fornecedor, idFornecedor)) return false;

    fornecedores.insert(fornecedor, idFornecedor);
  }

  if (fornecedores.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Erro ao cadastrar fornecedores.");
    return false;
  }

  return true;
}

void ImportaProdutos::mostraApenasEstesFornecedores() {
  for (auto const &fornecedor : fornecedores) idsFornecedor.append(QString::number(fornecedor));
}

bool ImportaProdutos::marcaTodosProdutosDescontinuados() {
  QSqlQuery query;

  if (not query.exec("UPDATE produto SET descontinuado = TRUE WHERE idFornecedor IN (" + idsFornecedor.join(",") + ") AND estoque_promocao = " + QString::number(tipo) + "")) {
    QMessageBox::critical(this, "Erro!", "Erro marcando produtos descontinuados: " + query.lastError().text());
    return false;
  }

  return true;
}

void ImportaProdutos::contaProdutos() {
  QSqlQuery queryProdSize("SELECT COUNT(*) FROM [BASE$]", db);
  queryProdSize.first();
  progressDialog->setMaximum(queryProdSize.value(0).toInt());
}

void ImportaProdutos::consistenciaDados() {
  for (auto const &key : variantMap.keys()) {
    if (variantMap.value(key).toString().contains("*")) {
      variantMap.insert(key, variantMap.value(key).toString().remove("*"));
    }
  }

  if (variantMap.value("ui").isNull()) variantMap.insert("ui", 0);

  if (variantMap.value("ncm").toString().length() == 10) {
    variantMap.insert("ncmEx", variantMap.value("ncm").toString().right(2));
    variantMap.insert("ncm", variantMap.value("ncm").toString().left(8));
  }

  const QString un = variantMap.value("un").toString().toUpper();

  (un == "M2" or un == "M²") ? variantMap.insert("un", "M²") : variantMap.insert("un", un);

  variantMap.insert("ncm", variantMap.value("ncm").toString().remove(".").remove(",").remove("-").remove(" "));
  variantMap.insert("codBarras", variantMap.value("codBarras").toString().remove(".").remove(","));
  variantMap.insert("codComercial", variantMap.value("codComercial").toString().remove(".").remove(","));

  variantMap.insert("minimo", variantMap.value("minimo").toDouble());

  // NOTE: cast other fields to the correct type?
}

void ImportaProdutos::leituraProduto(const QSqlQuery &query, const QSqlRecord &record) {
  for (auto const &key : variantMap.keys()) {
    if (key == "ncmEx") continue;
    if (key == "estoque_promocao") continue;
    if (key == "idProdutoRelacionado") continue;

    QVariant value = query.value(record.indexOf(key));

    if (value.type() == QVariant::Double) value = QString::number(value.toDouble(), 'f', 4).toDouble();

    variantMap.insert(key, value);
  }
}

bool ImportaProdutos::atualizaCamposProduto() {
  bool changed = false;

  for (auto const &key : variantMap.keys()) {
    if (not variantMap.value(key).isNull() and model.data(row, key) != variantMap.value(key)) {
      if (not model.setData(row, key, variantMap.value(key))) return false;
      if (not model.setData(row, key + "Upd", Yellow)) return false;
    } else {
      if (not model.setData(row, key + "Upd", White)) return false;
    }
  }

  if (model.data(row, "ncmEx").toString().isEmpty()) {
    if (not model.setData(row, "ncmExUpd", White)) return false;
  }

  const QString validadeStr = QDate::currentDate().addDays(validade).toString("yyyy-MM-dd");

  if (model.data(row, "validade") != validadeStr) {
    if (not model.setData(row, "validade", validadeStr)) return false;
    if (not model.setData(row, "validadeUpd", Yellow)) return false;
    changed = true;
  } else {
    if (not model.setData(row, "validadeUpd", White)) return false;
  }

  double markup = 100 * ((variantMap.value("precoVenda").toDouble() / variantMap.value("custo").toDouble()) - 1.);
  QString markupRound = QString::number(markup, 'f', 4);
  markup = markupRound.toDouble();

  if (model.data(row, "markup") != markup) {
    if (not model.setData(row, "markup", markup)) return false;
    if (not model.setData(row, "markupUpd", Yellow)) return false;
    changed = true;
  } else {
    if (not model.setData(row, "markupUpd", White)) return false;
  }

  changed ? itensUpdated++ : itensNotChanged++;

  return true;
}

bool ImportaProdutos::marcaProdutoNaoDescontinuado() {
  if (not model.setData(row, "descontinuado", false)) return false;

  itensExpired--;

  return true;
}

bool ImportaProdutos::guardaNovoPrecoValidade() {
  if (not expiraPrecosAntigos()) return false;

  QSqlQuery query;
  query.prepare("INSERT INTO produto_has_preco (idProduto, preco, validadeInicio, validadeFim) VALUES (:idProduto, :preco, :validadeInicio, :validadeFim)");
  query.bindValue(":idProduto", model.data(row, "idProduto"));
  query.bindValue(":preco", variantMap.value("precoVenda"));
  query.bindValue(":validadeInicio", QDate::currentDate());
  query.bindValue(":validadeFim", QDate::currentDate().addDays(validade));

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro inserindo dados em produto_has_preco: " + query.lastError().text());
    return false;
  }

  return true;
}

bool ImportaProdutos::verificaSeProdutoJaCadastrado() {
  return hash.contains(variantMap.value("fornecedor").toString() + variantMap.value("codComercial").toString() + variantMap.value("ui").toString() + QString::number(tipo));
}

bool ImportaProdutos::pintarCamposForaDoPadrao(const int row) {
  // Fora do padrão
  if (variantMap.value("ncm").toString() == "0" or variantMap.value("ncm").toString().isEmpty() or
      (variantMap.value("ncm").toString().length() != 8 and variantMap.value("ncm").toString().length() != 10)) {
    if (not modelErro.setData(row, "ncmUpd", Gray)) return false;
  }

  if (variantMap.value("codBarras").toString() == "0" or variantMap.value("codBarras").toString().isEmpty()) {
    if (not modelErro.setData(row, "codBarrasUpd", Gray)) return false;
  }

  // Errados
  if (variantMap.value("colecao").toString() == "REPETIDO") modelErro.setData(row, "colecaoUpd", Red);

  if ((variantMap.value("un").toString() == "M2" or variantMap.value("un").toString() == "M²" or variantMap.value("un").toString() == "ML") and variantMap.value("m2cx") <= 0.) {
    if (not modelErro.setData(row, "m2cxUpd", Red)) return false;
  }

  if (variantMap.value("un").toString() != "M2" and variantMap.value("un").toString() != "M²" and variantMap.value("un").toString() != "ML" and variantMap.value("pccx") < 1) {
    if (not modelErro.setData(row, "pccxUpd", Red)) return false;
  }

  if (variantMap.value("codComercial").toString() == "0" or variantMap.value("codComercial").toString().isEmpty()) {
    if (not modelErro.setData(row, "codComercialUpd", Red)) return false;
  }

  if (variantMap.value("custo") <= 0.) {
    if (not modelErro.setData(row, "custoUpd", Red)) return false;
  }

  if (variantMap.value("precoVenda") <= 0.) {
    if (not modelErro.setData(row, "precoVendaUpd", Red)) return false;
  }

  if (variantMap.value("precoVenda") < variantMap.value("custo")) {
    if (not modelErro.setData(row, "precoVendaUpd", Red)) return false;
  }

  return true;
}

bool ImportaProdutos::camposForaDoPadrao() {
  // Errados
  if (variantMap.value("colecao").toString() == "REPETIDO") return true;

  const QString un = variantMap.value("un").toString();
  if ((un == "M2" or un == "M²" or un == "ML") and variantMap.value("m2cx") <= 0.) return true;
  if (un != "M2" and un != "M²" and un != "ML" and variantMap.value("pccx") < 1) return true;

  const QString codComercial = variantMap.value("codComercial").toString();
  if (codComercial == "0" or codComercial.isEmpty()) return true;

  if (variantMap.value("custo") <= 0.) return true;
  if (variantMap.value("precoVenda") <= 0.) return true;
  if (variantMap.value("precoVenda") < variantMap.value("custo")) return true;

  return false;
}

bool ImportaProdutos::insereEmErro() {
  const int row = modelErro.rowCount();
  if (not modelErro.insertRow(row)) return false;

  for (auto const &key : variantMap.keys()) {
    if (not modelErro.setData(row, key, variantMap.value(key))) return false;
    if (not modelErro.setData(row, key + "Upd", Green)) return false;
  }

  if (not modelErro.setData(row, "idFornecedor", fornecedores.value(variantMap.value("fornecedor").toString()))) return false;

  if (not modelErro.setData(row, "atualizarTabelaPreco", true)) return false;
  const QString data = QDate::currentDate().addDays(validade).toString("yyyy-MM-dd");
  if (not modelErro.setData(row, "validade", data)) return false;
  if (not modelErro.setData(row, "validadeUpd", Green)) return false;

  const double markup = 100 * ((variantMap.value("precoVenda").toDouble() / variantMap.value("custo").toDouble()) - 1.);
  if (not modelErro.setData(row, "markup", markup)) return false;
  if (not modelErro.setData(row, "markupUpd", Green)) return false;

  if (variantMap.value("ncm").toString().length() == 10) {
    if (not modelErro.setData(row, "ncmEx", variantMap.value("ncm").toString().right(2))) return false;
    if (not modelErro.setData(row, "ncmExUpd", Green)) return false;
    variantMap.insert("ncm", variantMap.value("ncm").toString().left(8));
  } else {
    if (not modelErro.setData(row, "ncmExUpd", Green)) return false;
  }

  if (not pintarCamposForaDoPadrao(row)) return false;

  hasError = true;
  itensError++;

  return true;
}

bool ImportaProdutos::insereEmOk() {
  const int row = model.rowCount();

  if (not model.insertRow(row)) return false;

  for (auto const &key : variantMap.keys()) {
    if (not model.setData(row, key, variantMap.value(key))) return false;
    if (not model.setData(row, key + "Upd", Green)) return false;
  }

  if (not model.setData(row, "estoque_promocao", tipo)) return false;

  if (tipo == Estoque or tipo == Promocao) {
    QSqlQuery query;
    query.prepare("SELECT idProduto FROM produto WHERE idFornecedor = :idFornecedor AND codComercial = :codComercial AND estoque_promocao = 0");
    query.bindValue(":idFornecedor", fornecedores.value(variantMap.value("fornecedor").toString()));
    query.bindValue(":codComercial", model.data(row, "codComercial"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando produto relacionado: " + query.lastError().text());
      return false;
    }

    if (query.first() and not model.setData(row, "idProdutoRelacionado", query.value("idProduto"))) return false;
  }

  const int idFornecedor = fornecedores.value(variantMap.value("fornecedor").toString());
  if (not model.setData(row, "idFornecedor", idFornecedor)) return false;

  if (not model.setData(row, "atualizarTabelaPreco", true)) return false;
  if (not model.setData(row, "validade", QDate::currentDate().addDays(validade).toString("yyyy-MM-dd"))) return false;
  if (not model.setData(row, "validadeUpd", Green)) return false;

  const double markup = 100 * ((variantMap.value("precoVenda").toDouble() / variantMap.value("custo").toDouble()) - 1.);
  if (not model.setData(row, "markup", markup)) return false;
  if (not model.setData(row, "markupUpd", Green)) return false;

  if (variantMap.value("ncm").toString().length() == 10) {
    if (not model.setData(row, "ncmEx", variantMap.value("ncm").toString().right(2))) return false;
    if (not model.setData(row, "ncmExUpd", Green)) return false;
    variantMap.insert("ncm", variantMap.value("ncm").toString().left(8));
  } else {
    if (not model.setData(row, "ncmExUpd", Green)) return false;
  }

  hash[model.data(row, "fornecedor").toString() + model.data(row, "codComercial").toString() + model.data(row, "ui").toString() + model.data(row, "estoque_promocao").toString()] = row;

  hashAtualizado[row] = true;

  itensImported++;

  return true;
}

bool ImportaProdutos::cadastraProduto() { return (camposForaDoPadrao() ? insereEmErro() : insereEmOk()); }

bool ImportaProdutos::buscarCadastrarFornecedor(const QString &fornecedor, int &id) {
  QSqlQuery queryFornecedor;
  queryFornecedor.prepare("SELECT idFornecedor FROM fornecedor WHERE razaoSocial = :razaoSocial");
  queryFornecedor.bindValue(":razaoSocial", fornecedor);

  if (not queryFornecedor.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando fornecedor: " + queryFornecedor.lastError().text());
    return false;
  }

  if (not queryFornecedor.next()) {
    QMessageBox::information(this, "Aviso!", "Fornecedor ainda não cadastrado! Cadastrando...");

    queryFornecedor.prepare("INSERT INTO fornecedor (razaoSocial) VALUES (:razaoSocial)");
    queryFornecedor.bindValue(":razaoSocial", fornecedor);

    if (not queryFornecedor.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro cadastrando fornecedor: " + queryFornecedor.lastError().text());
      return false;
    }

    if (queryFornecedor.lastInsertId().isNull()) {
      QMessageBox::critical(this, "Erro!", "Erro lastInsertId");
      return false;
    }

    id = queryFornecedor.lastInsertId().toInt();
    return true;
  }

  id = queryFornecedor.value("idFornecedor").toInt();
  return true;
}

void ImportaProdutos::salvar() {
  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Ocorreu um erro ao salvar os dados: " + model.lastError().text());
    // TODO: refactor this because after this runs the transaction is no more
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery queryPrecos;
  queryPrecos.prepare("INSERT INTO produto_has_preco (idProduto, preco, validadeInicio, validadeFim) SELECT idProduto, precoVenda, :validadeInicio AS validadeInicio, :validadeFim AS validadeFim FROM "
                      "produto WHERE atualizarTabelaPreco = TRUE");
  queryPrecos.bindValue(":validadeInicio", QDate::currentDate().toString("yyyy-MM-dd"));
  queryPrecos.bindValue(":validadeFim", QDate::currentDate().addDays(validade).toString("yyyy-MM-dd"));

  if (not queryPrecos.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro inserindo dados em produto_has_preco: " + queryPrecos.lastError().text());
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  queryPrecos.exec("UPDATE produto SET atualizarTabelaPreco = FALSE");

  QSqlQuery("COMMIT").exec();

  QMessageBox::information(this, "Aviso!", "Tabela salva com sucesso!");

  close();
}

void ImportaProdutos::on_pushButtonSalvar_clicked() {
  if (hasError) {
    QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Produtos com erro não serão salvos. Deseja continuar?", QMessageBox::Yes | QMessageBox::No, this);
    msgBox.setButtonText(QMessageBox::Yes, "Continuar");
    msgBox.setButtonText(QMessageBox::No, "Voltar");

    if (msgBox.exec() == QMessageBox::No) return;
  }

  salvar();
}

bool ImportaProdutos::verificaTabela(const QSqlRecord &record) {
  for (auto const &key : variantMap.keys()) {
    if (key == "estoque_promocao") continue;
    if (key == "idProdutoRelacionado") continue;

    if (not record.contains(key)) {
      QMessageBox::critical(this, "Erro!", R"(Tabela não possui coluna ")" + key + R"(")");
      return false;
    }
  }

  return true;
}

void ImportaProdutos::on_tableProdutos_entered(const QModelIndex &) { ui->tableProdutos->resizeColumnsToContents(); }

void ImportaProdutos::on_tableErro_entered(const QModelIndex &) { ui->tableErro->resizeColumnsToContents(); }

void ImportaProdutos::on_tabWidget_currentChanged(const int index) {
  if (index == 0) ui->tableProdutos->resizeColumnsToContents();
  if (index == 1) ui->tableErro->resizeColumnsToContents();
}

void ImportaProdutos::closeEvent(QCloseEvent *event) {
  QSqlQuery("ROLLBACK").exec();

  QDialog::closeEvent(event);
}

void ImportaProdutos::on_checkBoxRepresentacao_toggled(const bool checked) {
  for (int row = 0, rowCount = model.rowCount(); row < rowCount; ++row) {
    if (not model.setData(row, "representacao", checked)) {
      QMessageBox::critical(this, "Erro!", "Erro guardando 'Representacao' em Produto: " + model.lastError().text());
    }
  }

  QSqlQuery query;
  if (not query.exec("UPDATE fornecedor SET representacao = " + QString(checked ? "TRUE" : "FALSE") + " WHERE idFornecedor IN (" + idsFornecedor.join(",") + ")")) {
    QMessageBox::critical(this, "Erro!", "Erro guardando 'Representacao' em Fornecedor: " + query.lastError().text());
    return;
  }
}

// NOTE: verificar o que esta deixando a importacao lenta ao longo do tempo
// NOTE: 3colocar tabela relacao para precos diferenciados por loja (associar produto_has_preco <->
// produto_has_preco_has_loja ou guardar idLoja em produto_has_preco)
// NOTE: remover idProdutoRelacionado?
// TODO: 2unificar m2cx/pccx em uncx, podendo ter uma segunda coluna pccx/kgcx

// TODO: *SUL*colocar flag na importacao para dizer qual tipo, se é sul ou nao (para diferenciar precos por regiao)
// TODO: como esta tabela é colorida usar o delegate para pintar texto de branco quando for fundo escuro
// TODO: markup esta exibindo errado ou salvando errado
// TODO: nao mostrar promocao descontinuado
// TODO: se der erro durante a leitura o arquivo nao é fechado
// TODO: nao marcou produtos representacao com flag 1
