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

bool WidgetCompraDevolucao::updateTables(QString &error) {
  if (model.tableName().isEmpty()) {
    setupTables();
    ui->radioButtonFiltroPendente->setChecked(true);
  }

  if (not model.select()){
   error = "Erro lendo tabela faturamento: " + model.lastError().text();
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

void WidgetCompraDevolucao::on_pushButtonRetornarEstoque_clicked() {
  auto list = ui->table->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Não selecionou nenhuma linha!");
    return;
  }

  QString status = model.data(list.first().row(), "status").toString();

  if (status == "PENDENTE" or status == "INICIADO" or status == "EM COMPRA" or status == "EM FATURAMENTO") {
    // se nao faturado nao faz nada
    model.setData(list.first().row(), "status", "PROCESSADO");
  }

  if (status == "EM COLETA" or status == "EM RECEBIMENTO" or status == "ESTOQUE") {
    // se faturado criar devolucao
    model.setData(list.first().row(), "status", "PROCESSADO");
    // 1.procurar em estoque pelo idVendaProduto
    // 2.copiar linha consumo mudando quant, status para devolucao e idCompra 0

    QSqlQuery query;
    query.prepare("SELECT idVendaProduto FROM venda_has_produto WHERE idVenda = :idVenda AND idProduto = :idProduto "
                  "AND status = 'DEVOLVIDO'");
    query.bindValue(":idVenda", model.data(list.first().row(), "idVenda")
                                    .toString()
                                    .left(model.data(list.first().row(), "idVenda").toString().size() - 1));
    query.bindValue(":idProduto", model.data(list.first().row(), "idProduto"));

    if (not query.exec() or not query.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando idVendaProduto: " + query.lastError().text());
      return;
    }

    QString idVendaProduto = query.value("idVendaProduto").toString();

    SqlTableModel modelEstoque;
    modelEstoque.setTable("estoque");
    modelEstoque.setFilter("idVendaProduto = " + idVendaProduto);

    modelEstoque.select();

    int newRow = modelEstoque.rowCount();
    modelEstoque.insertRow(newRow);

    for (int column = 0; column < modelEstoque.columnCount(); ++column) {
      modelEstoque.setData(newRow, column, modelEstoque.data(0, column));
    }

    modelEstoque.setData(newRow, "status", "DEVOLUÇÃO");
    modelEstoque.setData(newRow, "idCompra", 0);
    modelEstoque.setData(newRow, "quant", model.data(list.first().row(), "quant").toDouble() * -1);
    modelEstoque.setData(newRow, "quantUpd", 5);

    if (not modelEstoque.submitAll()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando devolução de estoque: " + modelEstoque.lastError().text());
      return;
    }
  }

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando status processado: " + model.lastError().text());
    return;
  }
}

void WidgetCompraDevolucao::on_radioButtonFiltroPendente_toggled(bool checked) {
  model.setFilter("quant < 0 AND status" + QString(checked ? "!=" : "=") + "'PROCESSADO'");
}

void WidgetCompraDevolucao::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }
