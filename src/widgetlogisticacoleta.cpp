#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "inputdialog.h"
#include "ui_widgetlogisticacoleta.h"
#include "widgetlogisticacoleta.h"

WidgetLogisticaColeta::WidgetLogisticaColeta(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetLogisticaColeta) {
  ui->setupUi(this);
}

WidgetLogisticaColeta::~WidgetLogisticaColeta() { delete ui; }

bool WidgetLogisticaColeta::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return false;
  }

  for (int row = 0; row < model.rowCount(); ++row) ui->table->openPersistentEditor(row, "selecionado");

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetLogisticaColeta::TableFornLogistica_activated(const QString &fornecedor) {
  model.setFilter("fornecedor = '" + fornecedor + "'");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return;
  }

  for (int row = 0; row < model.rowCount(); ++row) ui->table->openPersistentEditor(row, "selecionado");

  ui->checkBoxMarcarTodos->setChecked(false);

  ui->table->resizeColumnsToContents();
}

void WidgetLogisticaColeta::setupTables() {
  model.setTable("view_coleta");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  model.setHeaderData("numeroNFe", "NFe");
  model.setHeaderData("produto", "Produto");
  model.setHeaderData("codComercial", "Cód. Com.");
  model.setHeaderData("quant", "Quant.");
  model.setHeaderData("un", "Un.");
  model.setHeaderData("caixas", "Caixas");
  model.setHeaderData("kgcx", "Kg./Cx.");
  model.setHeaderData("idVenda", "Venda");
  model.setHeaderData("ordemCompra", "OC");
  model.setHeaderData("local", "Local");
  model.setHeaderData("dataPrevColeta", "Data Prev. Col.");

  model.setFilter("0");

  ui->table->setModel(&model);
  //  ui->table->hideColumn("idEstoque");
  ui->table->hideColumn("fornecedor");
}

void WidgetLogisticaColeta::on_pushButtonMarcarColetado_clicked() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not cadastrar()) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  if (not updateTables()) return;

  QMessageBox::information(this, "Aviso!", "Confirmado coleta.");
}

bool WidgetLogisticaColeta::cadastrar() {
  auto list = ui->table->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return false;
  }

  const QString filtro = model.filter();

  InputDialog *inputDlg = new InputDialog(InputDialog::Coleta, this);

  if (inputDlg->exec() != InputDialog::Accepted) return false;

  const QDate dataColeta = inputDlg->getDate();
  const QDate dataPrevReceb = inputDlg->getNextDate();

  model.setFilter(filtro);
  model.select();

  QSqlQuery query;

  for (auto const &item : list) {

    query.prepare("UPDATE estoque SET status = 'EM RECEBIMENTO' WHERE idEstoque = :idEstoque");
    query.bindValue(":idEstoque", model.data(item.row(), "idEstoque"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando status do estoque: " + query.lastError().text());
      return false;
    }

    query.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'EM RECEBIMENTO', dataRealColeta = "
                  ":dataRealColeta, dataPrevReceb = :dataPrevReceb WHERE idCompra IN (SELECT idCompra FROM "
                  "estoque_has_compra WHERE idEstoque = :idEstoque) AND codComercial = :codComercial");
    query.bindValue(":dataRealColeta", dataColeta);
    query.bindValue(":dataPrevReceb", dataPrevReceb);
    query.bindValue(":idEstoque", model.data(item.row(), "idEstoque"));
    query.bindValue(":codComercial", model.data(item.row(), "codComercial"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando status no pedido_fornecedor: " + query.lastError().text());
      return false;
    }

    // salvar status na venda

    query.prepare("UPDATE venda_has_produto SET status = 'EM RECEBIMENTO', dataRealColeta = :dataRealColeta, "
                  "dataPrevReceb = :dataPrevReceb WHERE idCompra IN (SELECT idCompra FROM estoque_has_compra WHERE "
                  "idEstoque = :idEstoque) AND codComercial = :codComercial");
    query.bindValue(":dataRealColeta", dataColeta);
    query.bindValue(":dataPrevReceb", dataPrevReceb);
    query.bindValue(":idEstoque", model.data(item.row(), "idEstoque"));
    query.bindValue(":idVendaProduto", model.data(item.row(), "codComercial"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status da compra: " + query.lastError().text());
      return false;
    }

    if (not query.exec("CALL update_venda_status()")) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
      return false;
    }
    //
  }

  return true;
}

void WidgetLogisticaColeta::on_checkBoxMarcarTodos_clicked(const bool &) { ui->table->selectAll(); }

void WidgetLogisticaColeta::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetLogisticaColeta::on_lineEditBuscaColeta_textChanged(const QString &) {
  const QString textoBusca = ui->lineEditBuscaColeta->text();

  model.setFilter("(numeroNFe LIKE '%" + textoBusca + "%') or (produto LIKE '%" + textoBusca +
                  "%') or (idVenda LIKE '%" + textoBusca + "%') or (ordemCompra LIKE '%" + textoBusca +
                  "%')"); // nfe, produto, venda, oc

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
}

// TODO: verificar como lidar com os estoques que nao possuem venda atrelada, como marcar datas previstas?
// TODO: algumas caixas com valor negativo (verificar como elas estão entrando no estoque)
// TODO: busca: ao limpar o campo ele apaga a selecao de fornecedor
// TODO: adicionar OC, numero da nota e idVenda
// TODO: colocar selecao de multiplas linhas
