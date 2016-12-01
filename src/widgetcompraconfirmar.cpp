#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "inputdialogfinanceiro.h"
#include "reaisdelegate.h"
#include "ui_widgetcompraconfirmar.h"
#include "widgetcompraconfirmar.h"

WidgetCompraConfirmar::WidgetCompraConfirmar(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompraConfirmar) {
  ui->setupUi(this);
}

WidgetCompraConfirmar::~WidgetCompraConfirmar() { delete ui; }

void WidgetCompraConfirmar::setupTables() {
  model.setTable("view_compras");

  model.setHeaderData("dataPrevConf", "Prev. Conf.");

  ui->table->setModel(&model);
  ui->table->setItemDelegateForColumn("Total", new ReaisDelegate(this));
  ui->table->hideColumn("Compra");
}

bool WidgetCompraConfirmar::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela compras: " + model.lastError().text());
    return false;
  }

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetCompraConfirmar::on_pushButtonConfirmarCompra_clicked() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not confirmarCompra()) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  updateTables();
  QMessageBox::information(this, "Aviso!", "Confirmado compra.");
}

bool WidgetCompraConfirmar::confirmarCompra() {
  if (ui->table->selectionModel()->selectedRows().size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return false;
  }

  const int row = ui->table->selectionModel()->selectedRows().first().row();
  const QString idCompra = model.data(row, "Compra").toString();

  InputDialogFinanceiro *inputDlg = new InputDialogFinanceiro(InputDialogFinanceiro::ConfirmarCompra, this);
  if (not inputDlg->setFilter(idCompra)) return false;

  if (inputDlg->exec() != InputDialogFinanceiro::Accepted) return false;

  const QDateTime dataConf = inputDlg->getDate();
  const QDateTime dataPrevista = inputDlg->getNextDate();

  QSqlQuery query;
  // TODO: instead of using selecionado use a list of idPedido?
  query.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'EM FATURAMENTO', dataRealConf = :dataRealConf, "
                "dataPrevFat = :dataPrevFat WHERE idCompra = :idCompra AND selecionado = 1");
  query.bindValue(":dataRealConf", dataConf);
  query.bindValue(":dataPrevFat", dataPrevista);
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status da compra: " + query.lastError().text());
    return false;
  }

  // salvar status na venda
  // TODO: in the future use idVendaProduto in place of idProduto/idCompra
  query.prepare("UPDATE venda_has_produto SET status = 'EM FATURAMENTO', dataRealConf = :dataRealConf, dataPrevFat = "
                ":dataPrevFat WHERE idProduto IN (SELECT idProduto FROM pedido_fornecedor_has_produto "
                "WHERE idCompra = :idCompra AND selecionado = 1) AND idCompra = :idCompra");
  query.bindValue(":dataRealConf", dataConf);
  query.bindValue(":dataPrevFat", dataPrevista);
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando status da venda: " + query.lastError().text());
    return false;
  }

  query.prepare(
      "UPDATE pedido_fornecedor_has_produto SET selecionado = 0 WHERE idCompra = :idCompra AND selecionado = 1");
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro setando selecionado 0: " + query.lastError().text());
    return false;
  }

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    return false;
  }
  //

  return true;
}

void WidgetCompraConfirmar::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

bool WidgetCompraConfirmar::cancelar() {
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return false;
  }

  QMessageBox msgBox(QMessageBox::Question, "Cancelar?", "Tem certeza que deseja cancelar?",
                     QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Cancelar");
  msgBox.setButtonText(QMessageBox::No, "Voltar");

  if (msgBox.exec() == QMessageBox::No) return false;

  const int row = list.first().row();

  QSqlQuery query;
  query.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'CANCELADO' WHERE ordemCompra = :ordemCompra");
  query.bindValue(":ordemCompra", model.data(row, "OC"));

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados: " + query.lastError().text());
    return false;
  }

  query.prepare("SELECT idVendaProduto FROM pedido_fornecedor_has_produto WHERE ordemCompra = :ordemCompra");
  query.bindValue(":ordemCompra", model.data(row, "OC"));

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados: " + query.lastError().text());
    return false;
  }

  while (query.next()) {
    QSqlQuery query2;
    query2.prepare("UPDATE venda_has_produto SET status = 'PENDENTE' WHERE idVendaProduto = :idVendaProduto AND "
                   "(status != 'DEVOLVIDO' OR status != 'CANCELADO')");
    query2.bindValue(":idVendaProduto", query.value("idVendaProduto"));

    if (not query2.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro voltando status do produto: " + query2.lastError().text());
      return false;
    }
  }

  return true;
}

void WidgetCompraConfirmar::on_pushButtonCancelarCompra_clicked() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not cancelar()) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  updateTables();
  QMessageBox::information(this, "Aviso!", "Itens cancelados!");
}

// TODO: poder confirmar dois pedidos juntos (quando vem um espelho só)
// NOTE: permitir na tela de compras alterar uma venda para quebrar um produto em dois para os casos de lotes
// diferentes: 50 -> 40+10
