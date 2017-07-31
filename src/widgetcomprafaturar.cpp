#include <QDate>
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "importarxml.h"
#include "inputdialog.h"
#include "inputdialogproduto.h"
#include "reaisdelegate.h"
#include "ui_widgetcomprafaturar.h"
#include "widgetcomprafaturar.h"

WidgetCompraFaturar::WidgetCompraFaturar(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompraFaturar) { ui->setupUi(this); }

WidgetCompraFaturar::~WidgetCompraFaturar() { delete ui; }

void WidgetCompraFaturar::setupTables() {
  model.setTable("view_faturamento");

  model.setFilter("representacao = " + QString(ui->checkBoxRepresentacao->isChecked() ? "1" : "0"));

  model.setHeaderData("dataPrevFat", "Prev. Fat.");

  ui->table->setModel(&model);
  ui->table->setItemDelegateForColumn("Total", new ReaisDelegate(this));
  ui->table->hideColumn("idCompra");
  ui->table->hideColumn("representacao");
}

bool WidgetCompraFaturar::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela faturamento: " + model.lastError().text());
    return false;
  }

  ui->table->resizeColumnsToContents();

  return true;
}

bool WidgetCompraFaturar::faturarCompra(const QDateTime &dataReal, const QStringList &idsCompra) {
  QSqlQuery query;

  for (auto const &idCompra : idsCompra) {
    query.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'EM ENTREGA', dataRealFat = :dataRealFat WHERE idCompra = :idCompra");
    query.bindValue(":dataRealFat", dataReal);
    query.bindValue(":idCompra", idCompra);

    if (not query.exec()) {
      error = "Erro atualizando status da compra: " + query.lastError().text();
      return false;
    }

    query.prepare("UPDATE venda_has_produto SET status = 'EM ENTREGA' WHERE idCompra = :idCompra");
    query.bindValue(":idCompra", idCompra);

    if (not query.exec()) {
      error = "Erro atualizando status do produto da venda: " + query.lastError().text();
      return false;
    }
  }

  return true;
}

void WidgetCompraFaturar::on_pushButtonMarcarFaturado_clicked() {
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Não selecionou nenhuma compra!");
    return;
  }

  QStringList idsCompra;
  QStringList fornecedores;

  for (auto const &item : list) {
    idsCompra << model.data(item.row(), "idCompra").toString();
    fornecedores << model.data(item.row(), "fornecedor").toString();
  }

  const int size = fornecedores.size();

  if (fornecedores.removeDuplicates() != size - 1) {
    QMessageBox::critical(this, "Erro!", "Fornecedores diferentes!");
    return;
  }

  InputDialogProduto inputDlg(InputDialogProduto::Faturamento);
  if (not inputDlg.setFilter(idsCompra)) return;
  if (inputDlg.exec() != InputDialogProduto::Accepted) return;

  const QDateTime dataReal = inputDlg.getDate();

  // TODO: quando a sigla CAMB pular

  const bool pularNota = ui->checkBoxRepresentacao->isChecked() or fornecedores.first() == "ATELIER" ? true : false;

  if (pularNota) {
    QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
    QSqlQuery("START TRANSACTION").exec();

    if (not faturarCompra(dataReal, idsCompra)) {
      QSqlQuery("ROLLBACK").exec();
      if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
      return;
    }

    QSqlQuery("COMMIT").exec();
  } else {
    auto *import = new ImportarXML(idsCompra, dataReal, this);
    import->setAttribute(Qt::WA_DeleteOnClose);
    import->showMaximized();

    if (import->exec() != QDialog::Accepted) return;
  }

  QSqlQuery query;

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    return;
  }

  updateTables();
  QMessageBox::information(this, "Aviso!", "Confirmado faturamento!");
}

void WidgetCompraFaturar::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetCompraFaturar::on_checkBoxRepresentacao_toggled(bool checked) {
  model.setFilter("representacao = " + QString(checked ? "1" : "0"));

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
}

bool WidgetCompraFaturar::cancelar(const QModelIndexList &list) {
  // TODO: nas outras telas com cancelamento verificar se estou filtrando

  for (auto const &item : list) {
    QSqlQuery query;
    query.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'CANCELADO' WHERE ordemCompra = :ordemCompra AND status = 'EM FATURAMENTO'");
    query.bindValue(":ordemCompra", model.data(item.row(), "OC"));

    if (not query.exec()) {
      error = "Erro salvando dados: " + query.lastError().text();
      return false;
    }

    query.prepare("SELECT idVendaProduto FROM pedido_fornecedor_has_produto WHERE ordemCompra = :ordemCompra AND status = 'EM FATURAMENTO'");
    query.bindValue(":ordemCompra", model.data(item.row(), "OC"));

    if (not query.exec()) {
      error = "Erro buscando dados: " + query.lastError().text();
      return false;
    }

    while (query.next()) {
      QSqlQuery query2;
      query2.prepare("UPDATE venda_has_produto SET status = 'PENDENTE' WHERE idVendaProduto = :idVendaProduto AND status = 'EM FATURAMENTO'");
      query2.bindValue(":idVendaProduto", query.value("idVendaProduto"));

      if (not query2.exec()) {
        error = "Erro voltando status do produto: " + query2.lastError().text();
        return false;
      }
    }

    // TODO: verificar como tratar isso
    //    query.prepare("UPDATE conta_a_pagar_has_pagamento SET status = 'CANCELADO' WHERE idCompra = :idCompra");
    //    query.bindValue(":idCompra", model.data(item.row(), "idCompra"));

    //    if (not query.exec()) {
    //      error = "Erro salvando pagamentos: " + query.lastError().text();
    //      return false;
    //    }
  }

  return true;
}

void WidgetCompraFaturar::on_pushButtonCancelarCompra_clicked() {
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  QMessageBox msgBox(QMessageBox::Question, "Cancelar?", "Tem certeza que deseja cancelar?", QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Cancelar");
  msgBox.setButtonText(QMessageBox::No, "Voltar");

  if (msgBox.exec() == QMessageBox::No) return;

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not cancelar(list)) {
    QSqlQuery("ROLLBACK").exec();
    if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
    return;
  }

  QSqlQuery("COMMIT").exec();

  updateTables();
  QMessageBox::information(this, "Aviso!", "Itens cancelados!");
}

void WidgetCompraFaturar::on_pushButtonReagendar_clicked() {
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  InputDialog input(InputDialog::Faturamento);
  if (input.exec() != InputDialog::Accepted) return;

  const QDate dataPrevista = input.getNextDate();

  for (auto const &item : list) {
    const int idCompra = model.data(item.row(), "idCompra").toInt();

    QSqlQuery query;
    query.prepare("UPDATE pedido_fornecedor_has_produto SET dataPrevFat = :dataPrevFat WHERE idCompra = :idCompra");
    query.bindValue(":dataPrevFat", dataPrevista);
    query.bindValue(":idCompra", idCompra);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro query pedido_fornecedor: " + query.lastError().text());
      return;
    }

    query.prepare("UPDATE venda_has_produto SET dataPrevFat = :dataPrevFat WHERE idCompra = :idCompra");
    query.bindValue(":dataPrevFat", dataPrevista);
    query.bindValue(":idCompra", idCompra);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro query venda_has_produto: " + query.lastError().text());
      return;
    }
  }

  updateTables();

  QMessageBox::information(this, "Aviso!", "Operação realizada com sucesso!");
}

// TODO: quando importar nota vincular com as contas_pagar
// TODO: reimportar nota id 4936 que veio com o produto dividido para testar o quantConsumido
// TODO: reestruturar na medida do possivel de forma que cada estoque tenha apenas uma nota/compra
// TODO: colocar tela de busca
