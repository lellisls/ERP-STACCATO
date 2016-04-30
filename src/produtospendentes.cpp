#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "estoque.h"
#include "inputdialog.h"
#include "produtospendentes.h"
#include "ui_produtospendentes.h"

ProdutosPendentes::ProdutosPendentes(QWidget *parent) : QDialog(parent), ui(new Ui::ProdutosPendentes) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);
  setupTables();

  show();
  ui->tableProdutos->resizeColumnsToContents();
}

ProdutosPendentes::~ProdutosPendentes() { delete ui; }

void ProdutosPendentes::viewProduto(const QString &codComercial, const QString &idVenda) {
  this->codComercial = codComercial;

  modelProdutos.setFilter("codComercial = '" + codComercial + "' AND idVenda = '" + idVenda + "'");

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela view_produto_pendente: " + modelProdutos.lastError().text());
    return;
  }

  double quant = 0;

  for (int row = 0; row < modelProdutos.rowCount(); ++row) {
    quant += modelProdutos.data(row, "quant").toDouble();
  }

  ui->doubleSpinBoxQuantTotal->setValue(quant);
  ui->doubleSpinBoxComprar->setValue(quant);

  QSqlQuery query;
  query.prepare("SELECT unCaixa FROM venda_has_produto WHERE codComercial = :codComercial");
  query.bindValue(":codComercial", codComercial);

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando unCaixa: " + query.lastError().text());
    return;
  }

  const double step = query.value("unCaixa").toDouble();

  ui->doubleSpinBoxQuantTotal->setSingleStep(step);
  ui->doubleSpinBoxComprar->setSingleStep(step);
  ui->doubleSpinBoxComprar->setMinimum(quant);

  ui->tableProdutos->resizeColumnsToContents();

  modelEstoque.setFilter("`Cód Com` = '" + codComercial + "' AND Quant > 0");

  if (not modelEstoque.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + modelEstoque.lastError().text());
    return;
  }

  ui->tableEstoque->resizeColumnsToContents();
}

void ProdutosPendentes::setupTables() {
  modelProdutos.setTable("view_produto_pendente");

  modelProdutos.setHeaderData("status", "Status");
  modelProdutos.setHeaderData("fornecedor", "Fornecedor");
  modelProdutos.setHeaderData("produto", "Descrição");
  modelProdutos.setHeaderData("colecao", "Coleção");
  modelProdutos.setHeaderData("formComercial", "Form. Com.");
  modelProdutos.setHeaderData("caixas", "Caixas");
  modelProdutos.setHeaderData("kgcx", "Kg./Cx.");
  modelProdutos.setHeaderData("quant", "Quant.");
  modelProdutos.setHeaderData("un", "Un.");
  modelProdutos.setHeaderData("codComercial", "Cód. Com.");
  modelProdutos.setHeaderData("codBarras", "Cód. Barras");
  modelProdutos.setHeaderData("custo", "Custo");

  ui->tableProdutos->setModel(&modelProdutos);
  ui->tableProdutos->hideColumn("idVendaProduto");
  ui->tableProdutos->hideColumn("idProduto");
  ui->tableProdutos->hideColumn("idCompra");
  ui->tableProdutos->resizeColumnsToContents();

  modelEstoque.setTable("view_estoque");
  modelEstoque.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelEstoque.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + modelEstoque.lastError().text());
  }

  ui->tableEstoque->setModel(&modelEstoque);
  ui->tableEstoque->hideColumn("idCompra");
  ui->tableEstoque->hideColumn("idVendaProduto");
  ui->tableEstoque->hideColumn("idProduto");
  ui->tableEstoque->hideColumn("idNFe");
  ui->tableEstoque->hideColumn("quantUpd");
  ui->tableEstoque->resizeColumnsToContents();
}

void ProdutosPendentes::on_pushButtonComprar_clicked() {
  InputDialog *inputDlg = new InputDialog(InputDialog::Carrinho, this);
  if (inputDlg->exec() != InputDialog::Accepted) return;
  QDate dataPrevista = inputDlg->getNextDate();

  insere(dataPrevista);
  atualizaVenda(dataPrevista);

  QSqlQuery query;

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    return;
  }

  QDialog::accept();
  close();
}

void ProdutosPendentes::on_pushButtonConsumirEstoque_clicked() {
  auto listProduto = ui->tableProdutos->selectionModel()->selectedRows();

  if (listProduto.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum produto selecionado!");
    return;
  }

  auto listEstoque = ui->tableEstoque->selectionModel()->selectedRows();

  if (listEstoque.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhuma estoque selecionado!");
    return;
  }

  const double quantProduto = modelProdutos.data(listProduto.first().row(), "quant").toDouble();
  const double quantEstoque = modelEstoque.data(listEstoque.first().row(), "quant").toDouble();

  if (quantProduto > quantEstoque) {
    QMessageBox::critical(this, "Erro!", "Estoque insuficiente!");
    return;
  }

  Estoque *estoque = new Estoque(this);
  estoque->viewRegisterById(modelEstoque.data(listEstoque.first().row(), "idEstoque").toString());
  estoque->criarConsumo(modelProdutos.data(listProduto.first().row(), "idVendaProduto"));
  estoque->show();

  InputDialog *inputDlg = new InputDialog(InputDialog::Entrega, this);

  if (inputDlg->exec() != InputDialog::Accepted) return;

  const QString dataEntrega = inputDlg->getDate().toString("yyyy-MM-dd");

  QSqlQuery query;

  // TODO: verificar se esta preenchendo dataPrevEnt (nao preenche no pre-consumo)

  if (not query.exec("UPDATE venda_has_produto SET dataPrevEnt = '" + dataEntrega +
                     "', status = 'ESTOQUE' WHERE idVendaProduto = " +
                     modelProdutos.data(listProduto.first().row(), "idVendaProduto").toString())) {
    QMessageBox::critical(this, "Erro!", "Erro salvando status da venda: " + query.lastError().text());
    close();
    return;
  }

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    close();
    return;
  }

  modelEstoque.select();
  modelProdutos.select();
}

void ProdutosPendentes::insere(const QDate &dataPrevista) {
  QSqlQuery query;
  query.prepare(
      "INSERT INTO pedido_fornecedor_has_produto (idVenda, fornecedor, idProduto, descricao, colecao, quant, un, un2, "
      "caixas, preco, kgcx, formComercial, codComercial, codBarras, dataPrevCompra) VALUES (:idVenda, :fornecedor, "
      ":idProduto, :descricao, :colecao, :quant, :un, :un2, :caixas, :preco, :kgcx, :formComercial, :codComercial, "
      ":codBarras, :dataPrevCompra)");
  query.bindValue(":idVenda", modelProdutos.data(0, "idVenda"));
  query.bindValue(":fornecedor", modelProdutos.data(0, "fornecedor"));
  query.bindValue(":idProduto", modelProdutos.data(0, "idProduto"));
  query.bindValue(":descricao", modelProdutos.data(0, "produto"));
  query.bindValue(":colecao", modelProdutos.data(0, "colecao"));
  query.bindValue(":quant", ui->doubleSpinBoxComprar->value());
  query.bindValue(":un", modelProdutos.data(0, "un"));
  query.bindValue(":un2", modelProdutos.data(0, "un2"));
  query.bindValue(":caixas", modelProdutos.data(0, "caixas"));
  query.bindValue(":preco", modelProdutos.data(0, "custo").toDouble());
  query.bindValue(":kgcx", modelProdutos.data(0, "kgcx"));
  query.bindValue(":formComercial", modelProdutos.data(0, "formComercial"));
  query.bindValue(":codComercial", modelProdutos.data(0, "codComercial"));
  query.bindValue(":codBarras", modelProdutos.data(0, "codBarras"));
  query.bindValue(":dataPrevCompra", dataPrevista);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro inserindo dados em pedido_fornecedor_has_produto: " + query.lastError().text());
    return;
  }
}

void ProdutosPendentes::atualizaVenda(const QDate &dataPrevista) {
  QSqlQuery query;
  query.prepare("UPDATE venda_has_produto SET dataPrevCompra = :dataPrevCompra, "
                "status = 'INICIADO' WHERE idVenda = :idVenda AND idProduto = :idProduto");
  query.bindValue(":dataPrevCompra", dataPrevista);
  query.bindValue(":idVenda", modelProdutos.data(0, "idVenda"));
  query.bindValue(":idProduto", modelProdutos.data(0, "idProduto"));

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro associando pedido_fornecedor a venda_has_produto: " + query.lastError().text());
    return;
  }
}

void ProdutosPendentes::on_tableProdutos_entered(const QModelIndex &) { ui->tableProdutos->resizeColumnsToContents(); }

void ProdutosPendentes::on_tableEstoque_entered(const QModelIndex &) { ui->tableEstoque->resizeColumnsToContents(); }

// TODO: se o estoque estiver em coleta/recebimento alterar status do consumo para 'pré-consumo'
// TODO: quando compra a soma de dois produtos só um está tendo o status modificado (mudar para selecao de linhas?)
