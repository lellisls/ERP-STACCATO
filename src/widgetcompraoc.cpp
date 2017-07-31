#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "acbr.h"
#include "doubledelegate.h"
#include "reaisdelegate.h"
#include "ui_widgetcompraoc.h"
#include "widgetcompraoc.h"

WidgetCompraOC::WidgetCompraOC(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompraOC) { ui->setupUi(this); }

WidgetCompraOC::~WidgetCompraOC() { delete ui; }

bool WidgetCompraOC::updateTables() {
  if (modelPedido.tableName().isEmpty()) setupTables();

  if (not modelPedido.select()) {
    emit errorSignal("Erro lendo tabela pedidos: " + modelPedido.lastError().text());
    return false;
  }

  if (not modelProduto.select()) {
    emit errorSignal("Erro lendo tabela produtos pedido: " + modelProduto.lastError().text());
    return false;
  }

  return true;
}

void WidgetCompraOC::setupTables() {
  modelPedido.setTable("view_ordemcompra");
  modelPedido.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelPedido.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela pedidos: " + modelPedido.lastError().text());
  }

  ui->tablePedido->setModel(&modelPedido);

  modelProduto.setTable("pedido_fornecedor_has_produto");
  modelProduto.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelProduto.setHeaderData("status", "Status");
  modelProduto.setHeaderData("idVenda", "Venda");
  modelProduto.setHeaderData("fornecedor", "Fornecedor");
  modelProduto.setHeaderData("descricao", "Produto");
  modelProduto.setHeaderData("colecao", "Coleção");
  modelProduto.setHeaderData("codComercial", "Cód. Com.");
  modelProduto.setHeaderData("quant", "Quant.");
  modelProduto.setHeaderData("un", "Un.");
  modelProduto.setHeaderData("un2", "Un2.");
  modelProduto.setHeaderData("caixas", "Cx.");
  modelProduto.setHeaderData("prcUnitario", "R$ Unit.");
  modelProduto.setHeaderData("preco", "R$");
  modelProduto.setHeaderData("kgcx", "Kg./Cx.");
  modelProduto.setHeaderData("formComercial", "Form. Com.");
  modelProduto.setHeaderData("codBarras", "Cód. Barras");
  modelProduto.setHeaderData("obs", "Obs.");

  modelProduto.setFilter("0");

  if (not modelProduto.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela produtos pedido: " + modelProduto.lastError().text());
  }

  ui->tableProduto->setModel(&modelProduto);
  ui->tableProduto->setItemDelegateForColumn("quant", new DoubleDelegate(this));
  ui->tableProduto->setItemDelegateForColumn("prcUnitario", new ReaisDelegate(this));
  ui->tableProduto->setItemDelegateForColumn("preco", new ReaisDelegate(this));
  ui->tableProduto->setItemDelegateForColumn("kgcx", new DoubleDelegate(this));
  ui->tableProduto->hideColumn("idPedido");
  ui->tableProduto->hideColumn("selecionado");
  ui->tableProduto->hideColumn("statusFinanceiro");
  ui->tableProduto->hideColumn("idVendaProduto");
  ui->tableProduto->hideColumn("ordemCompra");
  ui->tableProduto->hideColumn("idCompra");
  ui->tableProduto->hideColumn("idProduto");
  ui->tableProduto->hideColumn("quantUpd");
  ui->tableProduto->hideColumn("quantConsumida");
  ui->tableProduto->hideColumn("aliquotaSt");
  ui->tableProduto->hideColumn("st");
  ui->tableProduto->hideColumn("dataPrevCompra");
  ui->tableProduto->hideColumn("dataRealCompra");
  ui->tableProduto->hideColumn("dataPrevConf");
  ui->tableProduto->hideColumn("dataRealConf");
  ui->tableProduto->hideColumn("dataPrevFat");
  ui->tableProduto->hideColumn("dataRealFat");
  ui->tableProduto->hideColumn("dataPrevColeta");
  ui->tableProduto->hideColumn("dataRealColeta");
  ui->tableProduto->hideColumn("dataPrevReceb");
  ui->tableProduto->hideColumn("dataRealReceb");
  ui->tableProduto->hideColumn("dataPrevEnt");
  ui->tableProduto->hideColumn("dataRealEnt");

  modelNFe.setTable("view_ordemcompra_nfe");
  modelNFe.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelNFe.setHeaderData("numeroNFe", "NFe");

  modelNFe.setFilter("0");

  if (not modelNFe.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela nfe: " + modelNFe.lastError().text());
  }

  ui->tableNFe->setModel(&modelNFe);
  ui->tableNFe->hideColumn("ordemCompra");
  ui->tableNFe->hideColumn("idNFe");
}

void WidgetCompraOC::on_tablePedido_clicked(const QModelIndex &index) {
  const QString oc = modelPedido.data(index.row(), "OC").toString();

  modelProduto.setFilter("ordemCompra = " + oc);

  modelNFe.setFilter("ordemCompra = " + oc);
}

void WidgetCompraOC::on_pushButtonDanfe_clicked() {
  const auto list = ui->tableNFe->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  ACBr::gerarDanfe(modelNFe.data(list.first().row(), "idNFe").toInt());
}

void WidgetCompraOC::on_tablePedido_entered(const QModelIndex &) { ui->tablePedido->resizeColumnsToContents(); }

void WidgetCompraOC::on_tableProduto_entered(const QModelIndex &) { ui->tableProduto->resizeColumnsToContents(); }

void WidgetCompraOC::on_tableNFe_entered(const QModelIndex &) { ui->tableNFe->resizeColumnsToContents(); }

void WidgetCompraOC::on_pushButtonDesfazerConsumo_clicked() {
  const auto list = ui->tableProduto->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  QMessageBox msgBox(QMessageBox::Question, "Desfazer consumo?", "Tem certeza?", QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Continuar");
  msgBox.setButtonText(QMessageBox::No, "Voltar");

  if (msgBox.exec() == QMessageBox::No) return;

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not desfazerConsumo(list)) {
    QSqlQuery("ROLLBACK").exec();
    if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
    return;
  }

  QSqlQuery("COMMIT").exec();

  updateTables();

  QMessageBox::information(this, "Aviso!", "Operação realizada com sucesso!");
}

bool WidgetCompraOC::desfazerConsumo(const QModelIndexList &list) {
  for (auto item : list) {
    const int idVendaProduto = modelProduto.data(item.row(), "idVendaProduto").toInt();

    QSqlQuery query;

    query.prepare("SELECT status FROM estoque_has_consumo WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":idVendaProduto", idVendaProduto);

    if (not query.exec()) {
      error = "Erro buscando status do consumo estoque: " + query.lastError().text();
      return false;
    }

    if (not query.first()) {
      error = "Não encontrou consumo!";
      return false;
    }

    const QString status = query.value("status").toString();

    if (status == "ENTREGA AGEND." or status == "EM ENTREGA" or status == "ENTREGUE") {
      error = "Consumo do estoque está em entrega/entregue!";
      return false;
    }

    query.prepare("UPDATE pedido_fornecedor_has_produto SET idVenda = NULL, idVendaProduto = NULL WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":idVendaProduto", idVendaProduto);

    if (not query.exec()) {
      error = "Erro atualizando pedido compra: " + query.lastError().text();
      return false;
    }

    query.prepare("UPDATE venda_has_produto SET status = 'PENDENTE' WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":idVendaProduto", idVendaProduto);

    if (not query.exec()) {
      error = "Erro atualizando pedido venda: " + query.lastError().text();
      return false;
    }

    query.prepare("DELETE FROM estoque_has_consumo WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":idVendaProduto", idVendaProduto);

    if (not query.exec()) {
      error = "Erro removendo consumo estoque: " + query.lastError().text();
      return false;
    }
  }

  return true;
}

void WidgetCompraOC::on_lineEditBusca_textChanged(const QString &text) {
  modelPedido.setFilter("Venda LIKE '%" + text + "%' OR OC LIKE '%" + text + "%'");

  if (not modelPedido.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela pedidos: " + modelPedido.lastError().text());
}
