#include <QDate>
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include "doubledelegate.h"
#include "estoque.h"
#include "inputdialog.h"
#include "produtospendentes.h"
#include "reaisdelegate.h"
#include "ui_produtospendentes.h"

ProdutosPendentes::ProdutosPendentes(QWidget *parent) : QDialog(parent), ui(new Ui::ProdutosPendentes) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables();

  connect(ui->tableProdutos->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ProdutosPendentes::recalcularQuantidade);

  show();
  ui->tableProdutos->resizeColumnsToContents();
}

ProdutosPendentes::~ProdutosPendentes() { delete ui; }

void ProdutosPendentes::recalcularQuantidade() {
  double quant = 0;

  for (auto const &item : ui->tableProdutos->selectionModel()->selectedRows()) {
    quant += modelViewProdutos.data(item.row(), "quant").toDouble();
  }

  ui->doubleSpinBoxComprar->setValue(quant);
}

void ProdutosPendentes::viewProduto(const QString &codComercial, const QString &idVenda) {
  // NOTE: put this in the constructor because this object always use this function?
  this->codComercial = codComercial;

  modelProdutos.setFilter("codComercial = '" + codComercial + "' AND idVenda = '" + idVenda + "' AND (status = 'PENDENTE' OR status = 'QUEBRA')");

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela venda_has_produto: " + modelProdutos.lastError().text());
    return;
  }

  modelViewProdutos.setFilter("codComercial = '" + codComercial + "' AND idVenda = '" + idVenda + "' AND (status = 'PENDENTE' OR status = 'QUEBRA')");

  if (not modelViewProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela view_produto_pendente: " + modelViewProdutos.lastError().text());
    return;
  }

  double quant = 0;

  for (int row = 0; row < modelViewProdutos.rowCount(); ++row) quant += modelViewProdutos.data(row, "quant").toDouble();

  ui->doubleSpinBoxQuantTotal->setValue(quant);
  ui->doubleSpinBoxComprar->setValue(quant);

  ui->doubleSpinBoxQuantTotal->setSuffix(" " + modelViewProdutos.data(0, "un").toString());
  ui->doubleSpinBoxComprar->setSuffix(" " + modelViewProdutos.data(0, "un").toString());

  QSqlQuery query;
  query.prepare("SELECT unCaixa FROM venda_has_produto WHERE idVendaProduto = :idVendaProduto");
  query.bindValue(":idVendaProduto", modelViewProdutos.data(0, "idVendaProduto"));

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando unCaixa: " + query.lastError().text());
    return;
  }

  const double step = query.value("unCaixa").toDouble();

  ui->doubleSpinBoxQuantTotal->setSingleStep(step);
  ui->doubleSpinBoxComprar->setSingleStep(step);
  ui->doubleSpinBoxComprar->setMinimum(step);

  ui->tableProdutos->resizeColumnsToContents();

  const QString fornecedor = modelViewProdutos.data(0, "fornecedor").toString();

  modelEstoque.setFilter("restante > 0 AND codComercial = '" + codComercial + "' AND fornecedor = '" + fornecedor + "' AND status != 'CANCELADO' AND status != 'QUEBRADO'");

  if (not modelEstoque.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela compra: " + modelEstoque.lastError().text());
    return;
  }

  ui->tableEstoque->resizeColumnsToContents();
}

void ProdutosPendentes::setupTables() {
  modelProdutos.setTable("venda_has_produto");
  modelProdutos.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelViewProdutos.setTable("view_produto_pendente");
  modelViewProdutos.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelViewProdutos.setHeaderData("status", "Status");
  modelViewProdutos.setHeaderData("fornecedor", "Fornecedor");
  modelViewProdutos.setHeaderData("idVenda", "Venda");
  modelViewProdutos.setHeaderData("produto", "Descrição");
  modelViewProdutos.setHeaderData("colecao", "Coleção");
  modelViewProdutos.setHeaderData("formComercial", "Form. Com.");
  modelViewProdutos.setHeaderData("caixas", "Caixas");
  modelViewProdutos.setHeaderData("kgcx", "Kg./Cx.");
  modelViewProdutos.setHeaderData("quant", "Quant.");
  modelViewProdutos.setHeaderData("un", "Un.");
  modelViewProdutos.setHeaderData("un2", "Un.2");
  modelViewProdutos.setHeaderData("codComercial", "Cód. Com.");
  modelViewProdutos.setHeaderData("codBarras", "Cód. Barras");
  modelViewProdutos.setHeaderData("custo", "Custo");
  modelViewProdutos.setHeaderData("obs", "Obs.");

  ui->tableProdutos->setModel(&modelViewProdutos);
  ui->tableProdutos->setItemDelegateForColumn("quant", new DoubleDelegate(this, 3));
  ui->tableProdutos->setItemDelegateForColumn("custo", new ReaisDelegate(this));
  ui->tableProdutos->hideColumn("idVendaProduto");
  ui->tableProdutos->hideColumn("idProduto");
  ui->tableProdutos->hideColumn("idCompra");
  ui->tableProdutos->resizeColumnsToContents();

  modelEstoque.setTable("view_estoque");
  modelEstoque.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelEstoque.setHeaderData("status", "Status");
  modelEstoque.setHeaderData("idEstoque", "Estoque");
  modelEstoque.setHeaderData("descricao", "Descrição");
  modelEstoque.setHeaderData("restante", "Disponível");
  modelEstoque.setHeaderData("unEst", "Un.");
  modelEstoque.setHeaderData("codComercial", "Cód. Com.");
  modelEstoque.setHeaderData("lote", "Lote");
  modelEstoque.setHeaderData("local", "Local");
  modelEstoque.setHeaderData("bloco", "Bloco");

  ui->tableEstoque->setModel(&modelEstoque);
  ui->tableEstoque->setItemDelegateForColumn("restante", new DoubleDelegate(this, 3));
  ui->tableEstoque->hideColumn("cnpjDest");
  ui->tableEstoque->hideColumn("unProd");
  ui->tableEstoque->hideColumn("idCompra");
  ui->tableEstoque->hideColumn("fornecedor");
  ui->tableEstoque->hideColumn("nfe");
  ui->tableEstoque->hideColumn("restante est");
  ui->tableEstoque->hideColumn("dataPrevColeta");
  ui->tableEstoque->hideColumn("dataRealColeta");
  ui->tableEstoque->hideColumn("dataPrevReceb");
  ui->tableEstoque->hideColumn("dataRealReceb");
}

bool ProdutosPendentes::comprar(const QModelIndexList &list, const QDate &dataPrevista) {
  for (auto const &item : list) {
    const int row = item.row();

    if (not atualizarVendaCompra(row, dataPrevista)) return false;
    if (not enviarProdutoParaCompra(row, dataPrevista)) return false;
    if (not enviarExcedenteParaCompra(row, dataPrevista)) return false;
  }

  if (not modelProdutos.submitAll()) {
    error = "Erro salvando tabela venda_produto: " + modelProdutos.lastError().text();
    return false;
  }

  return true;
}

void ProdutosPendentes::recarregarTabelas() {
  modelProdutos.select();
  modelViewProdutos.select();
  modelEstoque.select();

  double quant = 0;

  for (int row = 0; row < modelViewProdutos.rowCount(); ++row) quant += modelViewProdutos.data(row, "quant").toDouble();

  ui->doubleSpinBoxQuantTotal->setValue(quant);
  ui->doubleSpinBoxComprar->setValue(quant);

  if (modelViewProdutos.rowCount() == 0) close();
}

void ProdutosPendentes::on_pushButtonComprar_clicked() {
  auto list = ui->tableProdutos->selectionModel()->selectedRows();

  if (list.isEmpty() and modelViewProdutos.rowCount() == 1) {
    list << modelViewProdutos.index(0, 0);
  }

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum produto selecionado!");
    return;
  }

  InputDialog inputDlg(InputDialog::Carrinho);
  if (inputDlg.exec() != InputDialog::Accepted) return;

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not comprar(list, inputDlg.getNextDate())) {
    QSqlQuery("ROLLBACK").exec();
    if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
    return;
  }

  QSqlQuery("COMMIT").exec();

  QSqlQuery query;

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    return;
  }

  QMessageBox::information(this, "Aviso!", "Produto enviado para carrinho!");

  recarregarTabelas();
}

bool ProdutosPendentes::insere(const QDateTime &dataPrevista) {
  QSqlQuery query;
  query.prepare("INSERT INTO pedido_fornecedor_has_produto (idVenda, idVendaProduto, fornecedor, idProduto, descricao, colecao, "
                "quant, un, un2, caixas, prcUnitario, preco, kgcx, formComercial, codComercial, codBarras, dataPrevCompra) "
                "VALUES (:idVenda, :idVendaProduto, :fornecedor, :idProduto, :descricao, :colecao, :quant, :un, :un2, :caixas, "
                ":prcUnitario, :preco, :kgcx, :formComercial, :codComercial, :codBarras, :dataPrevCompra)");
  query.bindValue(":idVenda", modelViewProdutos.data(0, "idVenda"));
  query.bindValue(":idVendaProduto", modelViewProdutos.data(0, "idVendaProduto"));
  query.bindValue(":fornecedor", modelViewProdutos.data(0, "fornecedor"));
  query.bindValue(":idProduto", modelViewProdutos.data(0, "idProduto"));
  query.bindValue(":descricao", modelViewProdutos.data(0, "produto"));
  query.bindValue(":colecao", modelViewProdutos.data(0, "colecao"));
  query.bindValue(":quant", ui->doubleSpinBoxComprar->value());
  query.bindValue(":un", modelViewProdutos.data(0, "un"));
  query.bindValue(":un2", modelViewProdutos.data(0, "un2"));
  query.bindValue(":caixas", modelViewProdutos.data(0, "caixas"));
  query.bindValue(":prcUnitario", modelViewProdutos.data(0, "custo").toDouble());
  query.bindValue(":preco", modelViewProdutos.data(0, "custo").toDouble() * ui->doubleSpinBoxComprar->value());
  query.bindValue(":kgcx", modelViewProdutos.data(0, "kgcx"));
  query.bindValue(":formComercial", modelViewProdutos.data(0, "formComercial"));
  query.bindValue(":codComercial", modelViewProdutos.data(0, "codComercial"));
  query.bindValue(":codBarras", modelViewProdutos.data(0, "codBarras"));
  query.bindValue(":dataPrevCompra", dataPrevista);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro inserindo dados em pedido_fornecedor_has_produto: " + query.lastError().text());
    return false;
  }

  return true;
}

void ProdutosPendentes::on_tableProdutos_entered(const QModelIndex &) { ui->tableProdutos->resizeColumnsToContents(); }

bool ProdutosPendentes::consumirEstoque(const int rowProduto, const int rowEstoque, const double quantConsumir, const double quantTotalVenda, const double quantEstoque) {
  // TODO: 1pensar em alguma forma de poder consumir compra que nao foi faturado ainda (provavelmente vou restaurar o
  // processo antigo e sincronizar as tabelas)

  const double consumir = qMin(quantEstoque, quantConsumir); // TODO: maybe dont need min, quantConsumir is already limited to quantEstoque

  auto *estoque = new Estoque(this);
  if (not estoque->viewRegisterById(modelEstoque.data(rowEstoque, "idEstoque").toString(), false)) return false;
  if (not estoque->criarConsumo(modelViewProdutos.data(rowProduto, "idVendaProduto").toInt(), consumir)) return false;

  // separar venda se quantidade nao bate

  if (quantConsumir < quantTotalVenda) {
    if (not quebrarVendaConsumo(quantConsumir, quantTotalVenda, rowProduto)) return false;
  }

  if (not modelProdutos.setData(rowProduto, "status", modelEstoque.data(rowEstoque, "status").toString())) return false;

  // model submit
  if (not modelProdutos.submitAll()) {
    error = "Erro salvando tabela venda_produto: " + modelProdutos.lastError().text();
    return false;
  }

  return true;
}

void ProdutosPendentes::on_pushButtonConsumirEstoque_clicked() {
  const auto listProduto = ui->tableProdutos->selectionModel()->selectedRows();

  if (listProduto.size() > 1) {
    QMessageBox::critical(this, "Erro!", "Selecione apenas um item na tabela de produtos!");
    return;
  }

  if (listProduto.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum produto selecionado!");
    return;
  }

  const auto listEstoque = ui->tableEstoque->selectionModel()->selectedRows();

  if ((modelEstoque.rowCount() == 0 or modelEstoque.rowCount() > 1) and listEstoque.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum estoque selecionado!");
    return;
  }

  const int rowProduto = listProduto.isEmpty() ? 0 : listProduto.first().row();
  const int rowEstoque = listEstoque.isEmpty() ? 0 : listEstoque.first().row();

  const double quantTotalVenda = modelViewProdutos.data(rowProduto, "quant").toDouble();
  const double quantEstoque = modelEstoque.data(rowEstoque, "restante").toDouble();

  bool ok;

  const double quantConsumir = QInputDialog::getDouble(this, "Consumo", "Quantidade a consumir: ", quantTotalVenda, 0, qMin(quantTotalVenda, quantEstoque), 3, &ok);
  // arredondar valor para multiplo caixa?

  if (not ok) return;

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not consumirEstoque(rowProduto, rowEstoque, quantConsumir, quantTotalVenda, quantEstoque)) {
    QSqlQuery("ROLLBACK").exec();
    if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
    return;
  }

  QSqlQuery("COMMIT").exec();

  QMessageBox::information(this, "Aviso!", "Consumo criado com sucesso!");

  recarregarTabelas();
}

bool ProdutosPendentes::enviarExcedenteParaCompra(const int row, const QDate &dataPrevista) {
  const double excedente = ui->doubleSpinBoxComprar->value() - ui->doubleSpinBoxQuantTotal->value();

  QSqlQuery query;

  if (excedente > 0.) {
    query.prepare("INSERT INTO pedido_fornecedor_has_produto (fornecedor, idProduto, descricao, "
                  "colecao, quant, un, un2, caixas, prcUnitario, preco, kgcx, formComercial, codComercial, codBarras, "
                  "dataPrevCompra) VALUES (:fornecedor, :idProduto, :descricao, :colecao, :quant, :un, :un2, :caixas, "
                  ":prcUnitario, :preco, :kgcx, :formComercial, :codComercial, :codBarras, :dataPrevCompra)");
    query.bindValue(":fornecedor", modelViewProdutos.data(row, "fornecedor"));
    query.bindValue(":idProduto", modelViewProdutos.data(row, "idProduto"));
    query.bindValue(":descricao", modelViewProdutos.data(row, "produto"));
    query.bindValue(":colecao", modelViewProdutos.data(row, "colecao"));
    query.bindValue(":quant", excedente);
    query.bindValue(":un", modelViewProdutos.data(row, "un"));
    query.bindValue(":un2", modelViewProdutos.data(row, "un2"));
    query.bindValue(":caixas", excedente * modelProdutos.data(row, "unCaixa").toDouble());
    query.bindValue(":prcUnitario", modelViewProdutos.data(row, "custo").toDouble());
    query.bindValue(":preco", modelViewProdutos.data(row, "custo").toDouble() * excedente);
    query.bindValue(":kgcx", modelViewProdutos.data(row, "kgcx"));
    query.bindValue(":formComercial", modelViewProdutos.data(row, "formComercial"));
    query.bindValue(":codComercial", modelViewProdutos.data(row, "codComercial"));
    query.bindValue(":codBarras", modelViewProdutos.data(row, "codBarras"));
    query.bindValue(":dataPrevCompra", dataPrevista);

    if (not query.exec()) {
      error = "Erro inserindo dados em pedido_fornecedor_has_produto: " + query.lastError().text();
      return false;
    }
  }

  return true;
}

bool ProdutosPendentes::enviarProdutoParaCompra(const int row, const QDate &dataPrevista) {
  QSqlQuery query;

  // inserir em pedido_fornecedor
  query.prepare("INSERT INTO pedido_fornecedor_has_produto (idVenda, idVendaProduto, fornecedor, idProduto, descricao, "
                "obs, colecao, quant, un, un2, caixas, prcUnitario, preco, kgcx, formComercial, codComercial, codBarras, "
                "dataPrevCompra) VALUES (:idVenda, :idVendaProduto, :fornecedor, :idProduto, :descricao, :obs, :colecao, :quant, "
                ":un, :un2, :caixas, :prcUnitario, :preco, :kgcx, :formComercial, :codComercial, :codBarras, :dataPrevCompra)");
  query.bindValue(":idVenda", modelViewProdutos.data(row, "idVenda"));
  query.bindValue(":idVendaProduto", modelViewProdutos.data(row, "idVendaProduto"));
  query.bindValue(":fornecedor", modelViewProdutos.data(row, "fornecedor"));
  query.bindValue(":idProduto", modelViewProdutos.data(row, "idProduto"));
  query.bindValue(":descricao", modelViewProdutos.data(row, "produto"));
  query.bindValue(":obs", modelViewProdutos.data(row, "obs"));
  query.bindValue(":colecao", modelViewProdutos.data(row, "colecao"));
  query.bindValue(":quant", modelViewProdutos.data(row, "quant"));
  query.bindValue(":un", modelViewProdutos.data(row, "un"));
  query.bindValue(":un2", modelViewProdutos.data(row, "un2"));
  query.bindValue(":caixas", modelViewProdutos.data(row, "quant").toDouble() / ui->doubleSpinBoxQuantTotal->singleStep());
  query.bindValue(":prcUnitario", modelViewProdutos.data(row, "custo").toDouble());
  query.bindValue(":preco", modelViewProdutos.data(row, "custo").toDouble() * modelViewProdutos.data(row, "quant").toDouble());
  query.bindValue(":kgcx", modelViewProdutos.data(row, "kgcx"));
  query.bindValue(":formComercial", modelViewProdutos.data(row, "formComercial"));
  query.bindValue(":codComercial", modelViewProdutos.data(row, "codComercial"));
  query.bindValue(":codBarras", modelViewProdutos.data(row, "codBarras"));
  query.bindValue(":dataPrevCompra", dataPrevista);

  if (not query.exec()) {
    error = "Erro inserindo dados em pedido_fornecedor_has_produto: " + query.lastError().text();
    return false;
  }

  return true;
}

bool ProdutosPendentes::atualizarVendaCompra(const int row, const QDate &dataPrevista) {
  // TODO: fix this so it is made later, somehow make this submitAll go after the for(...)

  if (ui->doubleSpinBoxComprar->value() < modelViewProdutos.data(row, "quant").toDouble()) {
    if (not quebrarVenda(row, dataPrevista)) return false;
  }

  if (not modelProdutos.setData(row, "status", "INICIADO")) return false;

  return true;
}

bool ProdutosPendentes::quebrarVenda(const int row, const QDate &dataPrevista) {
  // quebrar venda_has_produto

  const int newRow = modelProdutos.rowCount();
  modelProdutos.insertRow(newRow);

  // copiar colunas
  for (int column = 0, columnCount = modelProdutos.columnCount(); column < columnCount; ++column) {
    if (modelProdutos.fieldIndex("idVendaProduto") == column) continue;
    if (modelProdutos.fieldIndex("created") == column) continue;
    if (modelProdutos.fieldIndex("lastUpdated") == column) continue;

    const QVariant value = modelProdutos.data(row, column);

    if (not modelProdutos.setData(newRow, column, value)) return false;
  }

  // alterar quant, precos, etc da linha antiga

  const double quantNova = ui->doubleSpinBoxComprar->value();
  const double quantAntiga = modelProdutos.data(row, "quant").toDouble();
  const double unCaixa = modelProdutos.data(newRow, "unCaixa").toDouble();

  const double proporcao = quantNova / quantAntiga;
  const double parcial = modelProdutos.data(row, "parcial").toDouble() * proporcao;
  const double parcialDesc = modelProdutos.data(row, "parcialDesc").toDouble() * proporcao;
  const double total = modelProdutos.data(row, "total").toDouble() * proporcao;

  if (not modelProdutos.setData(row, "quant", quantNova)) return false;
  if (not modelProdutos.setData(row, "caixas", quantNova * unCaixa)) return false;
  if (not modelProdutos.setData(row, "parcial", parcial)) return false;
  if (not modelProdutos.setData(row, "parcialDesc", parcialDesc)) return false;
  if (not modelProdutos.setData(row, "total", total)) return false;
  if (not modelProdutos.setData(row, "dataPrevCompra", dataPrevista)) return false;

  // alterar quant, precos, etc da linha nova
  const double proporcaoNovo = (quantAntiga - quantNova) / quantAntiga;
  const double parcialNovo = modelProdutos.data(newRow, "parcial").toDouble() * proporcaoNovo;
  const double parcialDescNovo = modelProdutos.data(newRow, "parcialDesc").toDouble() * proporcaoNovo;
  const double totalNovo = modelProdutos.data(newRow, "total").toDouble() * proporcaoNovo;

  if (not modelProdutos.setData(newRow, "quant", quantAntiga - quantNova)) return false;
  if (not modelProdutos.setData(newRow, "caixas", (quantAntiga - quantNova) * unCaixa)) return false;
  if (not modelProdutos.setData(newRow, "parcial", parcialNovo)) return false;
  if (not modelProdutos.setData(newRow, "parcialDesc", parcialDescNovo)) return false;
  if (not modelProdutos.setData(newRow, "total", totalNovo)) return false;

  return true;
}

bool ProdutosPendentes::quebrarVendaConsumo(const double quantConsumir, const double quantTotalVenda, const int rowProduto) {
  // quebrar linha de cima em dois para poder comprar a outra parte?

  const int newRow = modelProdutos.rowCount();
  modelProdutos.insertRow(newRow);

  // copiar colunas
  for (int column = 0, columnCount = modelProdutos.columnCount(); column < columnCount; ++column) {
    if (modelProdutos.fieldIndex("idVendaProduto") == column) continue;
    if (modelProdutos.fieldIndex("created") == column) continue;
    if (modelProdutos.fieldIndex("lastUpdated") == column) continue;

    const QVariant value = modelProdutos.data(rowProduto, column);

    if (not modelProdutos.setData(newRow, column, value)) return false;
  }

  const double unCaixa = modelProdutos.data(rowProduto, "unCaixa").toDouble();

  const double proporcao = quantConsumir / quantTotalVenda;
  const double parcial = modelProdutos.data(rowProduto, "parcial").toDouble() * proporcao;
  const double parcialDesc = modelProdutos.data(rowProduto, "parcialDesc").toDouble() * proporcao;
  const double total = modelProdutos.data(rowProduto, "total").toDouble() * proporcao;

  if (not modelProdutos.setData(rowProduto, "quant", quantConsumir)) return false;
  if (not modelProdutos.setData(rowProduto, "caixas", quantConsumir / unCaixa)) return false;
  if (not modelProdutos.setData(rowProduto, "parcial", parcial)) return false;
  if (not modelProdutos.setData(rowProduto, "parcialDesc", parcialDesc)) return false;
  if (not modelProdutos.setData(rowProduto, "total", total)) return false;

  // alterar quant, precos, etc da linha nova
  const double proporcaoNovo = (quantTotalVenda - quantConsumir) / quantTotalVenda;
  const double parcialNovo = modelProdutos.data(newRow, "parcial").toDouble() * proporcaoNovo;
  const double parcialDescNovo = modelProdutos.data(newRow, "parcialDesc").toDouble() * proporcaoNovo;
  const double totalNovo = modelProdutos.data(newRow, "total").toDouble() * proporcaoNovo;

  if (not modelProdutos.setData(newRow, "quant", quantTotalVenda - quantConsumir)) return false;
  if (not modelProdutos.setData(newRow, "caixas", (quantTotalVenda - quantConsumir) / unCaixa)) return false;
  if (not modelProdutos.setData(newRow, "parcial", parcialNovo)) return false;
  if (not modelProdutos.setData(newRow, "parcialDesc", parcialDescNovo)) return false;
  if (not modelProdutos.setData(newRow, "total", totalNovo)) return false;

  return true;
}

// NOTE: se o estoque for consumido gerar comissao 2% senao gerar comissao padrao
