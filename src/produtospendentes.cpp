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

void ProdutosPendentes::viewProduto(const QString &codComercial, const QString &status) {
  this->codComercial = codComercial;

  modelProdutos.setFilter("codComercial = '" + codComercial + "' AND status = '" + status + "'");

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

  double step = query.value("unCaixa").toDouble();

  ui->doubleSpinBoxQuantTotal->setSingleStep(step);
  ui->doubleSpinBoxComprar->setSingleStep(step);
  ui->doubleSpinBoxComprar->setMinimum(quant);

  ui->tableProdutos->resizeColumnsToContents();

  modelEstoque.setFilter("codComercial = '" + codComercial + "'");

  if (not modelEstoque.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + modelEstoque.lastError().text());
    return;
  }

  for (int column = 0; column < modelEstoque.columnCount(); ++column) {
    if (modelEstoque.fieldIndex("xml") == column) continue;

    ui->tableEstoque->resizeColumnToContents(column);
  }
}

void ProdutosPendentes::setupTables() {
  modelProdutos.setTable("view_produto_pendente");

  modelProdutos.setHeaderData("fornecedor", "Fornecedor");
  modelProdutos.setHeaderData("produto", "Descrição");
  modelProdutos.setHeaderData("formComercial", "Form. Com.");
  modelProdutos.setHeaderData("quant", "Quant.");
  modelProdutos.setHeaderData("un", "Un.");
  modelProdutos.setHeaderData("codComercial", "Cód. Com.");
  modelProdutos.setHeaderData("status", "Status");
  modelProdutos.setHeaderData("estoque", "Estoque");

  ui->tableProdutos->setModel(&modelProdutos);

  modelEstoque.setTable("estoque");
  modelEstoque.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelEstoque.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + modelEstoque.lastError().text());
  }

  ui->tableEstoque->setModel(&modelEstoque);
  ui->tableEstoque->hideColumn("idEstoque");
  ui->tableEstoque->hideColumn("idCompra");
  ui->tableEstoque->hideColumn("idVendaProduto");
  ui->tableEstoque->hideColumn("idProduto");
  ui->tableEstoque->hideColumn("idNFe");
  ui->tableEstoque->hideColumn("quantUpd");
}

void ProdutosPendentes::on_pushButtonComprar_clicked() {
  InputDialog *inputDlg = new InputDialog(InputDialog::Carrinho, this);

  if (inputDlg->exec() != InputDialog::Accepted) return;

  QDate dataPrevista = inputDlg->getNextDate();

  QSqlQuery query;
  query.prepare(
        "SELECT * FROM pedido_fornecedor_has_produto WHERE codComercial = :codComercial AND status = 'PENDENTE'");
  query.bindValue(":codComercial", codComercial);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando produto: " + query.lastError().text());
    return;
  }

  query.first() ? atualiza(query) : insere(dataPrevista);

  atualizaVenda(dataPrevista);

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    return;
  }

  QDialog::accept();
  close();
}

void ProdutosPendentes::on_pushButtonConsumirEstoque_clicked() {
  Estoque *estoque = new Estoque(this);
  QString cod = modelProdutos.data(0, "codComercial").toString();
  qDebug() << "cod: " << cod;
  estoque->viewRegisterById(cod);
  estoque->show();
}

void ProdutosPendentes::atualiza(const QSqlQuery &query) {
  QSqlQuery query2;
  query2.prepare("UPDATE pedido_fornecedor_has_produto SET quant = :quant WHERE codComercial = :codComercial AND "
                 "status = 'PENDENTE'");
  query2.bindValue(":quant", query.value("quant").toDouble() + ui->doubleSpinBoxComprar->value());
  query2.bindValue(":codComercial", codComercial);

  if (not query2.exec()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro atualizando pedido_fornecedor_has_produto: " + query2.lastError().text());
    return;
  }
}

void ProdutosPendentes::insere(const QDate &dataPrevista) {
  QSqlQuery query;
  query.prepare(
        "INSERT INTO pedido_fornecedor_has_produto (fornecedor, idProduto, descricao, colecao, quant, un, un2, caixas, "
        "preco, kgcx, formComercial, codComercial, codBarras, dataPrevCompra) VALUES (:fornecedor, :idProduto, "
        ":descricao, :colecao, :quant, :un, :un2, :caixas, :preco, :kgcx, :formComercial, :codComercial, :codBarras, "
        ":dataPrevCompra)");
  query.bindValue(":fornecedor", modelProdutos.data(0, "fornecedor"));
  query.bindValue(":idProduto", modelProdutos.data(0, "idProduto"));
  query.bindValue(":descricao", modelProdutos.data(0, "produto"));
  query.bindValue(":colecao", modelProdutos.data(0, "colecao"));
  query.bindValue(":quant", ui->doubleSpinBoxComprar->value());
  query.bindValue(":un", modelProdutos.data(0, "un"));
  query.bindValue(":un2", modelProdutos.data(0, "un2"));
  query.bindValue(":caixas", modelProdutos.data(0, "caixas"));
  query.bindValue(":preco", modelProdutos.data(0, "custo").toDouble() * ui->doubleSpinBoxComprar->value());
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

  for (int row = 0; row < modelProdutos.rowCount(); ++row) {
    query.prepare("UPDATE venda_has_produto SET dataPrevCompra = :dataPrevCompra, "
                  "status = 'INICIADO' WHERE idVenda = :idVenda AND idProduto = :idProduto");
    query.bindValue(":dataPrevCompra", dataPrevista);
    query.bindValue(":idVenda", modelProdutos.data(row, "idVenda"));
    query.bindValue(":idProduto", modelProdutos.data(row, "idProduto"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!",
                            "Erro associando pedido_fornecedor a venda_has_produto: " + query.lastError().text());
      return;
    }
  }
}
