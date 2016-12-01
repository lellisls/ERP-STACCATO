#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

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

  show();
  ui->tableProdutos->resizeColumnsToContents();
}

ProdutosPendentes::~ProdutosPendentes() { delete ui; }

void ProdutosPendentes::viewProduto(const QString &codComercial, const QString &idVenda) {
  this->codComercial = codComercial;

  modelProdutos.setFilter("codComercial = '" + codComercial + "' AND idVenda = '" + idVenda +
                          "' AND status = 'PENDENTE'");

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela view_produto_pendente: " + modelProdutos.lastError().text());
    return;
  }

  double quant = 0;

  for (int row = 0; row < modelProdutos.rowCount(); ++row) quant += modelProdutos.data(row, "quant").toDouble();

  ui->doubleSpinBoxQuantTotal->setValue(quant);
  ui->doubleSpinBoxComprar->setValue(quant);

  ui->doubleSpinBoxQuantTotal->setSuffix(" " + modelProdutos.data(0, "un").toString());
  ui->doubleSpinBoxComprar->setSuffix(" " + modelProdutos.data(0, "un").toString());

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

  modelEstoque.setFilter("`Cód Com` = '" + codComercial + "' AND `Quant Rest` > 0");

  if (not modelEstoque.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + modelEstoque.lastError().text());
    return;
  }

  ui->tableEstoque->resizeColumnsToContents();

  modelCompra.setFilter("codComercial = '" + codComercial + "'");

  if (not modelCompra.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela compra: " + modelCompra.lastError().text());
    return;
  }

  ui->tableCompra->resizeColumnsToContents();
}

void ProdutosPendentes::setupTables() {
  modelProdutos.setTable("view_produto_pendente");
  modelProdutos.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelProdutos.setHeaderData("status", "Status");
  modelProdutos.setHeaderData("fornecedor", "Fornecedor");
  modelProdutos.setHeaderData("idVenda", "Venda");
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
  ui->tableProdutos->setItemDelegateForColumn("quant", new DoubleDelegate(this, 3));
  ui->tableProdutos->setItemDelegateForColumn("custo", new ReaisDelegate(this));
  ui->tableProdutos->hideColumn("idVendaProduto");
  ui->tableProdutos->hideColumn("idProduto");
  ui->tableProdutos->hideColumn("idCompra");
  ui->tableProdutos->resizeColumnsToContents();

  modelEstoque.setTable("view_estoque");
  modelEstoque.setEditStrategy(QSqlTableModel::OnManualSubmit);

  ui->tableEstoque->setModel(&modelEstoque);
  ui->tableEstoque->hideColumn("idCompra");
  ui->tableEstoque->hideColumn("idVendaProduto");
  ui->tableEstoque->hideColumn("idProduto");
  ui->tableEstoque->hideColumn("idNFe");
  ui->tableEstoque->hideColumn("quantUpd");
  ui->tableEstoque->resizeColumnsToContents();

  modelCompra.setTable("view_pedido_fornecedor_livre");
  modelCompra.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelCompra.setHeaderData("idVenda", "Venda");
  modelCompra.setHeaderData("ordemCompra", "OC");
  modelCompra.setHeaderData("descricao", "Descrição");
  modelCompra.setHeaderData("quant", "Quant.");
  modelCompra.setHeaderData("dataPrevCompra", "Data Prev. Comp.");
  modelCompra.setHeaderData("status", "Status");

  ui->tableCompra->setModel(&modelCompra);
  ui->tableCompra->hideColumn("idPedido");
  ui->tableCompra->hideColumn("codComercial");
  ui->tableCompra->hideColumn("idVendaProduto");
  ui->tableCompra->hideColumn("idCompra");
  ui->tableCompra->hideColumn("caixas");
  ui->tableCompra->hideColumn("prcUnitario");
  ui->tableCompra->hideColumn("dataRealCompra");
  ui->tableCompra->hideColumn("dataPrevConf");
}

bool ProdutosPendentes::comprar() {
  InputDialog *inputDlg = new InputDialog(InputDialog::Carrinho, this);
  if (inputDlg->exec() != InputDialog::Accepted) return false;

  const QDateTime dataPrevista = inputDlg->getNextDate();

  //

  for (int row = 0; row < modelProdutos.rowCount(); ++row) {
    // insere em pedido_fornecedor

    QSqlQuery query;
    query.prepare(
        "INSERT INTO pedido_fornecedor_has_produto (idVenda, idVendaProduto, fornecedor, idProduto, descricao, "
        "colecao, quant, un, un2, caixas, prcUnitario, preco, kgcx, formComercial, codComercial, codBarras, "
        "dataPrevCompra) VALUES (:idVenda, :idVendaProduto, :fornecedor, :idProduto, :descricao, :colecao, :quant, "
        ":un, :un2, :caixas, :prcUnitario, :preco, :kgcx, :formComercial, :codComercial, :codBarras, :dataPrevCompra)");
    query.bindValue(":idVenda", modelProdutos.data(row, "idVenda"));
    query.bindValue(":idVendaProduto", modelProdutos.data(row, "idVendaProduto"));
    query.bindValue(":fornecedor", modelProdutos.data(row, "fornecedor"));
    query.bindValue(":idProduto", modelProdutos.data(row, "idProduto"));
    query.bindValue(":descricao", modelProdutos.data(row, "produto"));
    query.bindValue(":colecao", modelProdutos.data(row, "colecao"));
    query.bindValue(":quant", modelProdutos.data(row, "quant"));
    query.bindValue(":un", modelProdutos.data(row, "un"));
    query.bindValue(":un2", modelProdutos.data(row, "un2"));
    query.bindValue(":caixas", modelProdutos.data(row, "caixas"));
    query.bindValue(":prcUnitario", modelProdutos.data(row, "custo").toDouble());
    query.bindValue(":preco",
                    modelProdutos.data(row, "custo").toDouble() * modelProdutos.data(row, "quant").toDouble());
    query.bindValue(":kgcx", modelProdutos.data(row, "kgcx"));
    query.bindValue(":formComercial", modelProdutos.data(row, "formComercial"));
    query.bindValue(":codComercial", modelProdutos.data(row, "codComercial"));
    query.bindValue(":codBarras", modelProdutos.data(row, "codBarras"));
    query.bindValue(":dataPrevCompra", dataPrevista);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!",
                            "Erro inserindo dados em pedido_fornecedor_has_produto: " + query.lastError().text());
      return false;
    }

    // atualiza venda

    query.prepare(
        "UPDATE venda_has_produto SET status = 'INICIADO', dataPrevCompra = :dataPrevCompra WHERE idVendaProduto = "
        ":idVendaProduto");
    query.bindValue(":dataPrevCompra", dataPrevista);
    query.bindValue(":idVendaProduto", modelProdutos.data(row, "idVendaProduto"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!",
                            "Erro associando pedido_fornecedor a venda_has_produto: " + query.lastError().text());
      return false;
    }
  }

  // depois de inserir cada linha venda, inserir a linha do excedente

  const double excedente = ui->doubleSpinBoxComprar->value() - ui->doubleSpinBoxQuantTotal->value();

  QSqlQuery query;

  if (excedente > 0.) {
    query.prepare("INSERT INTO pedido_fornecedor_has_produto (fornecedor, idProduto, descricao, "
                  "colecao, quant, un, un2, caixas, prcUnitario, preco, kgcx, formComercial, codComercial, codBarras, "
                  "dataPrevCompra) VALUES (:fornecedor, :idProduto, :descricao, :colecao, :quant, :un, :un2, :caixas, "
                  ":prcUnitario, :preco, :kgcx, :formComercial, :codComercial, :codBarras, :dataPrevCompra)");
    query.bindValue(":fornecedor", modelProdutos.data(0, "fornecedor"));
    query.bindValue(":idProduto", modelProdutos.data(0, "idProduto"));
    query.bindValue(":descricao", modelProdutos.data(0, "produto"));
    query.bindValue(":colecao", modelProdutos.data(0, "colecao"));
    query.bindValue(":quant", excedente);
    query.bindValue(":un", modelProdutos.data(0, "un"));
    query.bindValue(":un2", modelProdutos.data(0, "un2"));
    query.bindValue(":caixas", modelProdutos.data(0, "caixas"));
    query.bindValue(":prcUnitario", modelProdutos.data(0, "custo").toDouble());
    query.bindValue(":preco", modelProdutos.data(0, "custo").toDouble() * excedente);
    query.bindValue(":kgcx", modelProdutos.data(0, "kgcx"));
    query.bindValue(":formComercial", modelProdutos.data(0, "formComercial"));
    query.bindValue(":codComercial", modelProdutos.data(0, "codComercial"));
    query.bindValue(":codBarras", modelProdutos.data(0, "codBarras"));
    query.bindValue(":dataPrevCompra", dataPrevista);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!",
                            "Erro inserindo dados em pedido_fornecedor_has_produto: " + query.lastError().text());
      return false;
    }
  }

  //

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    return false;
  }

  return true;
}

void ProdutosPendentes::on_pushButtonComprar_clicked() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not comprar()) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  QMessageBox::information(this, "Aviso!", "Produto enviado para carrinho!");

  close();
}

void ProdutosPendentes::on_pushButtonConsumirEstoque_clicked() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not consumirEstoque()) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  QMessageBox::information(this, "Aviso!", "Consumo criado com sucesso!");

  close();
}

bool ProdutosPendentes::consumirEstoque() {
  const auto listProduto = ui->tableProdutos->selectionModel()->selectedRows();

  if (listProduto.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum produto selecionado!");
    return false;
  }

  const auto listEstoque = ui->tableEstoque->selectionModel()->selectedRows();

  if (listEstoque.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum estoque selecionado!");
    return false;
  }

  const double quantProduto = modelProdutos.data(listProduto.first().row(), "quant").toDouble();
  const double quantEstoque = modelEstoque.data(listEstoque.first().row(), "Quant Rest").toDouble();

  if (quantProduto > quantEstoque) {
    QMessageBox::critical(this, "Erro!", "Estoque insuficiente!");
    return false;
  }

  Estoque *estoque = new Estoque(this);
  estoque->viewRegisterById(modelEstoque.data(listEstoque.first().row(), "idEstoque").toString());

  if (not estoque->criarConsumo(modelProdutos.data(listProduto.first().row(), "idVendaProduto").toInt())) {
    QMessageBox::critical(this, "Erro!", "Erro ao criar consumo!");
    return false;
  }

  QSqlQuery query;

  query.prepare("UPDATE venda_has_produto SET status = 'ESTOQUE' WHERE idVendaProduto = :idVendaProduto");
  query.bindValue(":idVendaProduto", modelProdutos.data(listProduto.first().row(), "idVendaProduto").toString());

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando status da venda: " + query.lastError().text());
    close();
    return false;
  }

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    close();
    return false;
  }

  modelEstoque.select();
  modelProdutos.select();

  estoque->close();

  return true;
}

bool ProdutosPendentes::insere(const QDateTime &dataPrevista) {
  QSqlQuery query;
  query.prepare(
      "INSERT INTO pedido_fornecedor_has_produto (idVenda, idVendaProduto, fornecedor, idProduto, descricao, colecao, "
      "quant, un, un2, caixas, prcUnitario, preco, kgcx, formComercial, codComercial, codBarras, dataPrevCompra) "
      "VALUES (:idVenda, :idVendaProduto, :fornecedor, :idProduto, :descricao, :colecao, :quant, :un, :un2, :caixas, "
      ":prcUnitario, :preco, :kgcx, :formComercial, :codComercial, :codBarras, :dataPrevCompra)");
  query.bindValue(":idVenda", modelProdutos.data(0, "idVenda"));
  query.bindValue(":idVendaProduto", modelProdutos.data(0, "idVendaProduto"));
  query.bindValue(":fornecedor", modelProdutos.data(0, "fornecedor"));
  query.bindValue(":idProduto", modelProdutos.data(0, "idProduto"));
  query.bindValue(":descricao", modelProdutos.data(0, "produto"));
  query.bindValue(":colecao", modelProdutos.data(0, "colecao"));
  query.bindValue(":quant", ui->doubleSpinBoxComprar->value());
  query.bindValue(":un", modelProdutos.data(0, "un"));
  query.bindValue(":un2", modelProdutos.data(0, "un2"));
  query.bindValue(":caixas", modelProdutos.data(0, "caixas"));
  query.bindValue(":prcUnitario", modelProdutos.data(0, "custo").toDouble());
  query.bindValue(":preco", modelProdutos.data(0, "custo").toDouble() * ui->doubleSpinBoxComprar->value());
  query.bindValue(":kgcx", modelProdutos.data(0, "kgcx"));
  query.bindValue(":formComercial", modelProdutos.data(0, "formComercial"));
  query.bindValue(":codComercial", modelProdutos.data(0, "codComercial"));
  query.bindValue(":codBarras", modelProdutos.data(0, "codBarras"));
  query.bindValue(":dataPrevCompra", dataPrevista);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro inserindo dados em pedido_fornecedor_has_produto: " + query.lastError().text());
    return false;
  }

  return true;
}

void ProdutosPendentes::on_tableProdutos_entered(const QModelIndex &) { ui->tableProdutos->resizeColumnsToContents(); }

void ProdutosPendentes::on_tableEstoque_entered(const QModelIndex &) { ui->tableEstoque->resizeColumnsToContents(); }

bool ProdutosPendentes::consumirCompra() {
  // TODO: se o pedido_fornecedor possuir estoque ('em coleta' para frente) criar consumo estoque

  const auto listProduto = ui->tableProdutos->selectionModel()->selectedRows();

  if (modelProdutos.rowCount() > 1 and listProduto.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum produto selecionado!");
    return false;
  }

  const auto listCompra = ui->tableCompra->selectionModel()->selectedRows();

  if ((modelCompra.rowCount() == 0 or modelCompra.rowCount() > 1) and listCompra.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhuma compra selecionada!");
    return false;
  }

  const int rowProduto = listProduto.size() == 0 ? 0 : listProduto.first().row();
  const int rowCompra = listCompra.size() == 0 ? 0 : listCompra.first().row();

  QSqlQuery query;

  query.prepare("INSERT INTO pedido_fornecedor_has_produto (ordemCompra, idCompra, idVenda, idVendaProduto, "
                "fornecedor, idProduto, descricao, colecao, quant, un, un2, caixas, prcUnitario, preco, kgcx, "
                "formComercial, codComercial, codBarras, dataPrevCompra, dataRealCompra, dataPrevConf, status) VALUES "
                "(:ordemCompra, :idCompra, :idVenda, :idVendaProduto, :fornecedor, :idProduto, :descricao, :colecao, "
                ":quant, :un, :un2, :caixas, :prcUnitario, :preco, :kgcx, :formComercial, :codComercial, :codBarras, "
                ":dataPrevCompra, :dataRealCompra, :dataPrevConf, :status)");
  query.bindValue(":ordemCompra", modelCompra.data(rowCompra, "ordemCompra"));
  query.bindValue(":idCompra", modelCompra.data(rowCompra, "idCompra"));
  query.bindValue(":idVenda", modelProdutos.data(rowProduto, "idVenda"));
  query.bindValue(":idVendaProduto", modelProdutos.data(rowProduto, "idVendaProduto"));
  query.bindValue(":fornecedor", modelProdutos.data(rowProduto, "fornecedor"));
  query.bindValue(":idProduto", modelProdutos.data(rowProduto, "idProduto"));
  query.bindValue(":descricao", modelProdutos.data(rowProduto, "produto"));
  query.bindValue(":colecao", modelProdutos.data(rowProduto, "colecao"));
  query.bindValue(":quant", modelProdutos.data(rowProduto, "quant"));
  query.bindValue(":un", modelProdutos.data(rowProduto, "un"));
  query.bindValue(":un2", modelProdutos.data(rowProduto, "un2"));
  query.bindValue(":caixas", modelProdutos.data(rowProduto, "caixas"));
  query.bindValue(":prcUnitario", modelProdutos.data(rowProduto, "custo").toDouble());
  query.bindValue(":preco", modelProdutos.data(rowProduto, "custo").toDouble() *
                                modelProdutos.data(rowProduto, "quant").toDouble());
  query.bindValue(":kgcx", modelProdutos.data(rowProduto, "kgcx"));
  query.bindValue(":formComercial", modelProdutos.data(rowProduto, "formComercial"));
  query.bindValue(":codComercial", modelProdutos.data(rowProduto, "codComercial"));
  query.bindValue(":codBarras", modelProdutos.data(rowProduto, "codBarras"));
  query.bindValue(":dataPrevCompra", modelCompra.data(rowCompra, "dataPrevCompra"));
  query.bindValue(":dataRealCompra", modelCompra.data(rowCompra, "dataRealCompra"));
  query.bindValue(":dataPrevConf", modelCompra.data(rowCompra, "dataPrevConf"));
  query.bindValue(":status", modelCompra.data(rowCompra, "status"));

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro inserindo dados em pedido_fornecedor_has_produto: " + query.lastError().text());
    return false;
  }

  const double quant =
      modelCompra.data(rowCompra, "quant").toDouble() - modelProdutos.data(rowProduto, "quant").toDouble();
  const double caixas =
      modelCompra.data(rowCompra, "caixas").toInt() - modelProdutos.data(rowProduto, "caixas").toInt();

  if (quant > 0) {
    query.prepare("UPDATE pedido_fornecedor_has_produto SET quant = :quant, caixas = :caixas, preco = :preco WHERE "
                  "idPedido = :idPedido");
    query.bindValue(":quant", quant);
    query.bindValue(":caixas", caixas);
    query.bindValue(":preco", quant * modelCompra.data(rowCompra, "prcUnitario").toDouble());
    query.bindValue(":idPedido", modelCompra.data(rowCompra, "idPedido"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando quantidade em pedido_fornecedor_has_produto: " +
                                               query.lastError().text());
      return false;
    }
  }

  if (quant == 0) {
    query.prepare("DELETE FROM pedido_fornecedor_has_produto WHERE idPedido = :idPedido");
    query.bindValue(":idPedido", modelCompra.data(rowCompra, "idPedido"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!",
                            "Erro apagando em pedido_fornecedor_has_produto: " + query.lastError().text());
      return false;
    }
  }

  // atualiza venda

  query.prepare(
      "UPDATE venda_has_produto SET status = :status, dataPrevCompra = :dataPrevCompra WHERE idVendaProduto = "
      ":idVendaProduto");
  query.bindValue(":status", modelCompra.data(rowCompra, "status"));
  query.bindValue(":dataPrevCompra", modelCompra.data(rowCompra, "dataPrevCompra"));
  query.bindValue(":idVendaProduto", modelProdutos.data(rowProduto, "idVendaProduto"));

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro associando pedido_fornecedor a venda_has_produto: " + query.lastError().text());
    return false;
  }

  return true;
}

void ProdutosPendentes::on_pushButtonConsumirCompra_clicked() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not consumirCompra()) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  QMessageBox::information(this, "Aviso!", "Consumo criado com sucesso!");

  close();
}

// TODO: deixar apenas o botao de consumir compra pois ele faz todas as associacoes de tabelas (remover o botao de
// consumir estoque e verificar quais os casos que precisa tratar)
// TODO: se o estoque estiver em coleta/recebimento alterar status do consumo para 'pré-consumo'
// NOTE: se o estoque for consumido gerar comissao 2% senao gerar comissao padrao
