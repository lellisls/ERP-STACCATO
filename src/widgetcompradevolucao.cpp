#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "ui_widgetcompradevolucao.h"
#include "widgetcompradevolucao.h"

WidgetCompraDevolucao::WidgetCompraDevolucao(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompraDevolucao) {
  ui->setupUi(this);
}

WidgetCompraDevolucao::~WidgetCompraDevolucao() { delete ui; }

bool WidgetCompraDevolucao::updateTables() {
  if (model.tableName().isEmpty()) {
    setupTables();
    ui->radioButtonFiltroPendente->setChecked(true);
  }

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela faturamento: " + model.lastError().text());
    return false;
  }

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetCompraDevolucao::setupTables() {
  model.setTable("venda_has_produto");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela produtos pendentes: " + model.lastError().text());
  }

  ui->table->setModel(&model);

  ui->table->sortByColumn("idVenda");
  ui->table->hideColumn("selecionado");
  ui->table->resizeColumnsToContents();
}

void WidgetCompraDevolucao::on_pushButtonDevolucaoFornecedor_clicked() {
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Não selecionou nenhuma linha!");
    return;
  }

  const QString status = model.data(list.first().row(), "status").toString();

  if (status != "PENDENTE" and status != "INICIADO" and status != "EM COMPRA" and status != "EM FATURAMENTO") {
    QMessageBox::critical(this, "Erro!", "Ainda não implementado!");
    return;
  }

  if (status == "PENDENTE" or status == "INICIADO" or status == "EM COMPRA" or status == "EM FATURAMENTO") {
    // se nao faturado nao faz nada
    if (not model.setData(list.first().row(), "status", "PROCESSADO")) {
      QMessageBox::critical(this, "Erro!", "Erro marcando status: " + model.lastError().text());
      return;
    }

    const QString idVenda = model.data(list.first().row(), "idVenda").toString();

    QSqlQuery query;
    query.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'CANCELADO' WHERE idVenda = :idVenda AND "
                  "codComercial = :codComercial");
    query.bindValue(":idVenda", idVenda.left(idVenda.size() - 1));
    query.bindValue(":codComercial", model.data(list.first().row(), "codComercial"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro marcando compra como cancelada: " + query.lastError().text());
      return;
    }
  }
}

void WidgetCompraDevolucao::on_pushButtonRetornarEstoque_clicked() { // TODO: wrap in transaction
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Não selecionou nenhuma linha!");
    return;
  }

  const QString status = model.data(list.first().row(), "status").toString();

  if (status != "PENDENTE" and status != "INICIADO" and status != "EM COMPRA" and status != "EM FATURAMENTO" and
      status != "ME COLETA" and status != "EM RECEBIMENTO" and status != "ESTOQUE") {
    QMessageBox::critical(this, "Erro!", "Ainda não implementado!");
    return;
  }

  if (status == "PENDENTE" or status == "INICIADO" or status == "EM COMPRA" or status == "EM FATURAMENTO") {
    // se nao faturado nao faz nada
    model.setData(list.first().row(), "status", "PROCESSADO");
  }

  if (status == "EM COLETA" or status == "EM RECEBIMENTO" or status == "ESTOQUE") {
    // se faturado criar devolucao
    model.setData(list.first().row(), "status", "PROCESSADO");
    // 1.procurar em estoque pelo idVendaProduto
    // 2.copiar linha consumo mudando quant, status para devolucao e idCompra 0

    const QString idVenda = model.data(list.first().row(), "idVenda").toString();

    QSqlQuery query;
    query.prepare("SELECT idVendaProduto FROM venda_has_produto WHERE idVenda = :idVenda AND idProduto = :idProduto "
                  "AND status = 'DEVOLVIDO'");
    query.bindValue(":idVenda", idVenda.left(idVenda.size() - 1));
    query.bindValue(":idProduto", model.data(list.first().row(), "idProduto"));

    if (not query.exec() or not query.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando idVendaProduto: " + query.lastError().text());
      return;
    }

    const QString idVendaProduto = query.value("idVendaProduto").toString();

    SqlTableModel modelEstoque;
    modelEstoque.setTable("estoque_has_consumo");
    modelEstoque.setFilter("idVendaProduto = " + idVendaProduto);

    if (not modelEstoque.select()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando consumo estoque: " + modelEstoque.lastError().text());
      return;
    }

    if (modelEstoque.rowCount() > 0) {
      const int newRow = modelEstoque.rowCount();
      modelEstoque.insertRow(newRow);

      for (int column = 0; column < modelEstoque.columnCount(); ++column) {
        if (not modelEstoque.setData(newRow, column, modelEstoque.data(0, column))) {
          QMessageBox::critical(this, "Erro!", "Erro copiando dados do consumo: " + modelEstoque.lastError().text());
          return;
        }
      }

      modelEstoque.setData(newRow, "status", "DEVOLVIDO");
      modelEstoque.setData(newRow, "idCompra", 0);
      modelEstoque.setData(newRow, "quant", model.data(list.first().row(), "quant").toDouble() * -1);
      modelEstoque.setData(newRow, "quantUpd", 5);

      if (not modelEstoque.submitAll()) {
        QMessageBox::critical(this, "Erro!", "Erro salvando devolução de estoque: " + modelEstoque.lastError().text());
        return;
      }
    }
  }

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando status processado: " + model.lastError().text());
    return;
  }

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
}

void WidgetCompraDevolucao::on_radioButtonFiltroPendente_toggled(bool checked) {
  model.setFilter("quant < 0 AND status" + QString(checked ? "!=" : "=") + " 'PROCESSADO'");

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
}

void WidgetCompraDevolucao::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

// TODO: if filtering for 'PROCESSADO' dont let the user redo the actions