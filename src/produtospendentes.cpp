#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include "produtospendentes.h"
#include "ui_produtospendentes.h"
#include "inputdialog.h"

ProdutosPendentes::ProdutosPendentes(QWidget *parent) : QDialog(parent), ui(new Ui::ProdutosPendentes) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);
  setupTables();

  show();
  ui->tableProdutos->resizeColumnsToContents();
}

ProdutosPendentes::~ProdutosPendentes() { delete ui; }

void ProdutosPendentes::viewProduto(const QString codComercial, const QString status) {
  this->codComercial = codComercial;

  modelProdutos.setQuery(
        "SELECT v.fornecedor, v.idVenda, v.idProduto, v.produto, p.colecao, v.formComercial, v.quant "
        "AS quant, v.un, v.codComercial, p.codBarras, v.idCompra, v.status, (SELECT SUM(quant) FROM "
        "estoque WHERE codComercial = '" +
        codComercial +
        "') AS estoque FROM venda_has_produto AS v LEFT JOIN estoque AS e ON v.idProduto = e.idProduto LEFT JOIN produto "
        "AS p ON v.idProduto = p.idProduto WHERE v.codComercial = '" +
        codComercial + "' AND v.status = '" + status + "' GROUP BY idVenda");

  modelProdutos.setHeaderData(modelProdutos.record().indexOf("fornecedor"), Qt::Horizontal, "Fornecedor");
  modelProdutos.setHeaderData(modelProdutos.record().indexOf("produto"), Qt::Horizontal, "Descrição");
  modelProdutos.setHeaderData(modelProdutos.record().indexOf("formComercial"), Qt::Horizontal, "Form. Com.");
  modelProdutos.setHeaderData(modelProdutos.record().indexOf("quant"), Qt::Horizontal, "Quant.");
  modelProdutos.setHeaderData(modelProdutos.record().indexOf("un"), Qt::Horizontal, "Un.");
  modelProdutos.setHeaderData(modelProdutos.record().indexOf("codComercial"), Qt::Horizontal, "Cód. Com.");
  modelProdutos.setHeaderData(modelProdutos.record().indexOf("status"), Qt::Horizontal, "Status");
  modelProdutos.setHeaderData(modelProdutos.record().indexOf("estoque"), Qt::Horizontal, "Estoque");

  double quant = 0;

  for (int i = 0; i < modelProdutos.rowCount(); ++i) {
    quant += modelProdutos.data(modelProdutos.index(i, modelProdutos.record().indexOf("quant"))).toDouble();
  }

  ui->doubleSpinBoxQuantTotal->setValue(quant);
  ui->doubleSpinBoxComprar->setValue(quant);

  QSqlQuery query;
  query.prepare("SELECT unCaixa FROM venda_has_produto WHERE codComercial = :codComercial");
  query.bindValue(":codComercial", codComercial);

  if (not query.exec() or not query.first()) {
    qDebug() << "Erro buscando unCaixa: " << query.lastError();
  }

  double step = query.value("unCaixa").toDouble();

  ui->doubleSpinBoxQuantTotal->setSingleStep(step);
  ui->doubleSpinBoxComprar->setSingleStep(step);
  ui->doubleSpinBoxComprar->setMinimum(quant);

  ui->tableProdutos->resizeColumnsToContents();

  if (modelProdutos.data(modelProdutos.index(0, modelProdutos.record().indexOf("status"))).toString() == "PENDENTE") {
    ui->pushButtonConsumirEstoque->setDisabled(true);
  }

  if (modelProdutos.data(modelProdutos.index(0, modelProdutos.record().indexOf("status"))).toString() == "EM COMPRA") {
    ui->pushButtonConsumirEstoque->setDisabled(true);
  }
}

void ProdutosPendentes::setupTables() {
  ui->tableProdutos->setModel(&modelProdutos);

  ui->tableProdutos->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableProdutos->horizontalHeader()->setResizeContentsPrecision(0);
}

void ProdutosPendentes::on_pushButtonComprar_clicked() {
  InputDialog *inputDlg = new InputDialog(InputDialog::Carrinho, this);
  QDate dataPrevista;

  if (inputDlg->exec() == InputDialog::Accepted) {
    dataPrevista = inputDlg->getNextDate();
  } else {
    return;
  }

  QSqlQuery query;

  query.prepare("SELECT * FROM pedido_fornecedor_has_produto WHERE codComercial = :codComercial");
  query.bindValue(":codComercial", codComercial);

  if (not query.exec()) {
    qDebug() << "Erro buscando produto: " << query.lastError();
    return;
  }

  if (query.first() and query.value("status").toString() == "PENDENTE") {
    double quant = query.value("quant").toDouble();

    query.prepare("UPDATE pedido_fornecedor_has_produto SET quant = :quant WHERE codComercial = :codComercial AND "
                  "status = 'PENDENTE'");
    query.bindValue(":quant", quant + ui->doubleSpinBoxComprar->value());
    query.bindValue(":codComercial", codComercial);

    if (not query.exec()) {
      qDebug() << "Erro atualizando pedido_fornecedor_has_produto: " << query.lastError();
      return;
    }
  } else {
    query.prepare("SELECT custo FROM produto WHERE idProduto = :idProduto");
    query.bindValue(":idProduto",
                    modelProdutos.data(modelProdutos.index(0, modelProdutos.record().indexOf("idProduto"))));

    if (not query.exec() or not query.first()) {
      qDebug() << "Erro buscando custo do produto: " << query.lastError();
    }

    double custo = query.value(0).toDouble() * ui->doubleSpinBoxComprar->value();

    query.prepare(
          "INSERT INTO pedido_fornecedor_has_produto (fornecedor, idProduto, descricao, colecao, quant, un, "
          "preco, formComercial, codComercial, codBarras, dataPrevCompra) VALUES (:fornecedor, :idProduto, "
          ":descricao, :colecao, :quant, :un, :preco, :formComercial, :codComercial, :codBarras, :dataPrevCompra)");
    query.bindValue(":fornecedor",
                    modelProdutos.data(modelProdutos.index(0, modelProdutos.record().indexOf("fornecedor"))));
    query.bindValue(":idProduto",
                    modelProdutos.data(modelProdutos.index(0, modelProdutos.record().indexOf("idProduto"))));
    query.bindValue(":descricao",
                    modelProdutos.data(modelProdutos.index(0, modelProdutos.record().indexOf("produto"))));
    query.bindValue(":colecao", modelProdutos.data(modelProdutos.index(0, modelProdutos.record().indexOf("colecao"))));
    query.bindValue(":quant", ui->doubleSpinBoxComprar->value());
    query.bindValue(":un", modelProdutos.data(modelProdutos.index(0, modelProdutos.record().indexOf("un"))));
    query.bindValue(":preco", custo);
    query.bindValue(":formComercial",
                    modelProdutos.data(modelProdutos.index(0, modelProdutos.record().indexOf("formComercial"))));
    query.bindValue(":codComercial",
                    modelProdutos.data(modelProdutos.index(0, modelProdutos.record().indexOf("codComercial"))));
    query.bindValue(":codBarras",
                    modelProdutos.data(modelProdutos.index(0, modelProdutos.record().indexOf("codBarras"))));
    query.bindValue(":dataPrevCompra", dataPrevista);

    if (not query.exec()) {
      qDebug() << "Erro inserindo em pedido_fornecedor_has_produto: " << query.lastError();
      return;
    }

    for (int i = 0; i < modelProdutos.rowCount(); ++i) {
      query.prepare("UPDATE venda_has_produto SET dataPrevCompra = :dataPrevCompra, "
                    "status = 'INICIADO' WHERE idVenda = :idVenda AND idProduto = :idProduto");
      query.bindValue(":dataPrevCompra", dataPrevista);
      query.bindValue(":idVenda",
                      modelProdutos.data(modelProdutos.index(i, modelProdutos.record().indexOf("idVenda"))));
      query.bindValue(":idProduto",
                      modelProdutos.data(modelProdutos.index(i, modelProdutos.record().indexOf("idProduto"))));

      if (not query.exec()) {
        qDebug() << "Erro associando pedido_fornecedor a venda_has_produto: " << query.lastError();
      }
    }
  }

  if (not query.exec("CALL update_venda_status()")) {
    qDebug() << "Erro atualizando status das vendas: " << query.lastError();
  }

  QDialog::accept();
  close();
}

void ProdutosPendentes::on_pushButtonConsumirEstoque_clicked() {
  QMessageBox::information(this, "Aviso!", "Ainda não implementado");
}
