#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "ui_widgetcompradevolucao.h"
#include "widgetcompradevolucao.h"

WidgetCompraDevolucao::WidgetCompraDevolucao(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompraDevolucao) { ui->setupUi(this); }

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
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  model.setHeaderData("status", "Status");
  model.setHeaderData("fornecedor", "Fornecedor");
  model.setHeaderData("idVenda", "Venda");
  model.setHeaderData("produto", "Produto");
  model.setHeaderData("obs", "Obs.");
  model.setHeaderData("caixas", "Cx.");
  model.setHeaderData("quant", "Quant.");
  model.setHeaderData("un", "Un.");
  model.setHeaderData("unCaixa", "Un./Cx.");
  model.setHeaderData("codComercial", "Cód. Com.");
  model.setHeaderData("formComercial", "Form. Com.");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela produtos pendentes: " + model.lastError().text());
  }

  ui->table->setModel(&model);

  ui->table->sortByColumn("idVenda");
  ui->table->hideColumn("recebeu");
  ui->table->hideColumn("entregou");
  ui->table->hideColumn("selecionado");
  ui->table->hideColumn("idVendaProduto");
  ui->table->hideColumn("idCompra");
  ui->table->hideColumn("idNfeSaida");
  ui->table->hideColumn("idLoja");
  ui->table->hideColumn("idProduto");
  ui->table->hideColumn("prcUnitario");
  ui->table->hideColumn("descUnitario");
  ui->table->hideColumn("parcial");
  ui->table->hideColumn("desconto");
  ui->table->hideColumn("parcialDesc");
  ui->table->hideColumn("descGlobal");
  ui->table->hideColumn("total");
  ui->table->hideColumn("estoque_promocao");
  ui->table->hideColumn("dataPrevCompra");
  ui->table->hideColumn("dataRealCompra");
  ui->table->hideColumn("dataPrevConf");
  ui->table->hideColumn("dataRealConf");
  ui->table->hideColumn("dataPrevFat");
  ui->table->hideColumn("dataRealFat");
  ui->table->hideColumn("dataPrevColeta");
  ui->table->hideColumn("dataRealColeta");
  ui->table->hideColumn("dataPrevReceb");
  ui->table->hideColumn("dataRealReceb");
  ui->table->hideColumn("dataPrevEnt");
  ui->table->hideColumn("dataRealEnt");
  ui->table->resizeColumnsToContents();
}

void WidgetCompraDevolucao::on_pushButtonDevolucaoFornecedor_clicked() {
  // NOTE: colocar produto no fluxo da logistica para devolver ao fornecedor?
  // TODO: 2criar nota de devolucao caso tenha recebido nota (troca cfop, inverte remetente/destinario, nat. 'devolucao
  // de mercadoria'

  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Não selecionou nenhuma linha!");
    return;
  }

  for (auto const &item : list) {
    if (not model.setData(item.row(), "status", "PROCESSADO")) {
      QMessageBox::critical(this, "Erro!", "Erro marcando status: " + model.lastError().text());
      return;
    }
  }

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando status processado: " + model.lastError().text());
    return;
  }

  QMessageBox::information(this, "Aviso!", "Retornado para fornecedor!");
}

bool WidgetCompraDevolucao::retornarEstoque(const QModelIndexList &list) {
  for (auto const &item : list) {
    const QString status = model.data(item.row(), "status").toString();

    // TODO: refazer isso para bloquear o botao
    if (status == "PENDENTE" or status == "INICIADO" or status == "EM COMPRA" or status == "EM FATURAMENTO") {
      // se nao faturado nao faz nada
      if (not model.setData(item.row(), "status", "PROCESSADO")) return false;

      // TODO: perguntar se quer cancelar o produto correspondente da compra/ ou a compra inteira (verificar pelo
      // idVendaProduto)
      // TODO: colocar uma linha de pagamento negativa no fluxo da compra para quando corrigir fluxo ter o valor total
      // alterado
      // TODO: criar uma tabelinha de coisas pendentes para o financeiro

    } else {
      //    else if (status == "EM COLETA" or status == "EM RECEBIMENTO" or status == "ESTOQUE") {
      // se faturado criar devolucao estoque_has_consumo
      // 1.procurar em estoque pelo idVendaProduto
      // 2.copiar linha consumo mudando quant, status para devolucao e idCompra 0
      if (not model.setData(item.row(), "status", "PROCESSADO")) return false;

      const QString idVenda = model.data(item.row(), "idVenda").toString();

      QSqlQuery query;
      // TODO: verificar se estou pegando o idVendaProduto da devolucao e nao o original
      query.prepare("SELECT idVendaProduto FROM venda_has_produto WHERE idVenda = :idVenda AND idProduto = :idProduto "
                    "AND status = 'DEVOLVIDO'");
      query.bindValue(":idVenda", idVenda.left(11));
      query.bindValue(":idProduto", model.data(item.row(), "idProduto"));

      if (not query.exec()) {
        error = "Erro buscando idVendaProduto: " + query.lastError().text();
        return false;
      }

      if (not query.first()) {
        error = "Estoque não possui consumo!";
        return false;
      }

      const QString idVendaProduto = query.value("idVendaProduto").toString();

      SqlTableModel modelEstoque;
      modelEstoque.setTable("estoque_has_consumo");
      modelEstoque.setFilter("idVendaProduto = " + idVendaProduto);

      if (not modelEstoque.select()) {
        error = "Erro buscando consumo estoque: " + modelEstoque.lastError().text();
        return false;
      }

      if (modelEstoque.rowCount() == 0) {
        error = "Não encontrou estoque!";
        return false;
      }

      const int newRow = modelEstoque.rowCount();
      modelEstoque.insertRow(newRow);

      for (int column = 0; column < modelEstoque.columnCount(); ++column) {
        if (modelEstoque.fieldIndex("idConsumo") == column) continue;
        if (modelEstoque.fieldIndex("created") == column) continue;
        if (modelEstoque.fieldIndex("lastUpdated") == column) continue;

        if (not modelEstoque.setData(newRow, column, modelEstoque.data(0, column))) {
          error = "Erro copiando dados do consumo: " + modelEstoque.lastError().text();
          return false;
        }
      }

      modelEstoque.setData(newRow, "status", "DEVOLVIDO");
      modelEstoque.setData(newRow, "quant", model.data(item.row(), "quant").toDouble() * -1);
      modelEstoque.setData(newRow, "quantUpd", 5);
      // TODO: 2substituir idVendaProduto pelo id da devolucao

      if (not modelEstoque.submitAll()) {
        error = "Erro salvando devolução de estoque: " + modelEstoque.lastError().text();
        return false;
      }
    }
    //    else {
    //      error = "Status inválido!";
    //      return false;
    //    }
  }

  if (not model.submitAll()) {
    error = "Erro salvando status processado: " + model.lastError().text();
    return false;
  }

  return true;
}

void WidgetCompraDevolucao::on_pushButtonRetornarEstoque_clicked() {
  // TODO: 0verificar se o sistema nao esta tentando fazer a devolucao do idVendaProduto da devolucao e nao do original

  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Não selecionou nenhuma linha!");
    return;
  }

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not retornarEstoque(list)) {
    QSqlQuery("ROLLBACK").exec();
    if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
    return;
  }

  QSqlQuery("COMMIT").exec();

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());

  QMessageBox::information(this, "Aviso!", "Retornado para estoque!");
}

void WidgetCompraDevolucao::on_radioButtonFiltroPendente_toggled(bool checked) {
  ui->pushButtonDevolucaoFornecedor->setEnabled(checked);
  ui->pushButtonRetornarEstoque->setEnabled(checked);

  model.setFilter("quant < 0 AND status" + QString(checked ? "!=" : "=") + " 'PROCESSADO'");

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
}

void WidgetCompraDevolucao::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }
