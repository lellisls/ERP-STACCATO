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

  QString filter = model.filter();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return false;
  }

  model.setFilter(filter);

  for (int row = 0; row < model.rowCount(); ++row) {
    ui->table->openPersistentEditor(row, "selecionado");
  }

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

  for (int row = 0; row < model.rowCount(); ++row) {
    ui->table->openPersistentEditor(row, "selecionado");
  }

  ui->checkBoxMarcarTodos->setChecked(false);

  ui->table->resizeColumnsToContents();
}

void WidgetLogisticaColeta::setupTables() {
  model.setTable("view_coleta");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  model.setHeaderData("numeroNFe", "NFe");
  model.setHeaderData("produto", "Produto");
  model.setHeaderData("quant", "Quant.");
  model.setHeaderData("un", "Un.");
  model.setHeaderData("caixas", "Caixas");
  model.setHeaderData("idVenda", "Venda");
  model.setHeaderData("ordemCompra", "OC");
  model.setHeaderData("local", "Local");
  model.setHeaderData("dataRealCompra", "Data Comp.");
  model.setHeaderData("dataRealConf", "Data Conf.");
  model.setHeaderData("dataRealFat", "Data Fat.");
  model.setHeaderData("dataPrevColeta", "Data Prev. Col.");

  model.setFilter("0");

  ui->table->setModel(&model);

  ui->table->hideColumn("fornecedor");
  ui->table->hideColumn("idVendaProduto");
}

void WidgetLogisticaColeta::on_pushButtonMarcarColetado_clicked() {
  auto lista = ui->table->selectionModel()->selectedRows();

  if (lista.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  QString filtro = model.filter();

  InputDialog *inputDlg = new InputDialog(InputDialog::Coleta, this);

  if (inputDlg->exec() != InputDialog::Accepted) return;

  QString dataColeta = inputDlg->getDate().toString("yyyy-MM-dd");
  QString dataPrevista = inputDlg->getNextDate().toString("yyyy-MM-dd");

  model.setFilter(filtro);
  model.select();

  for (const auto item : lista) {
    QSqlQuery query;

    query.prepare("UPDATE estoque SET status = 'EM RECEBIMENTO' WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":idVendaProduto", model.data(item.row(), "idVendaProduto"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status do estoque: " + query.lastError().text());
      return;
    }

    // salvar status na venda

    query.prepare("UPDATE venda_has_produto SET dataRealColeta = :dataRealColeta, dataPrevReceb = :dataPrevReceb, "
                  "status = 'EM RECEBIMENTO' WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":dataRealColeta", dataColeta);
    query.bindValue(":dataPrevReceb", dataPrevista);
    query.bindValue(":idVendaProduto", model.data(item.row(), "idVendaProduto"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status da venda: " + query.lastError().text());
      return;
    }

    if (not query.exec("CALL update_venda_status()")) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
      return;
    }
    //
  }

  if (not updateTables()) return;

  QMessageBox::information(this, "Aviso!", "Confirmado coleta.");
}

void WidgetLogisticaColeta::on_checkBoxMarcarTodos_clicked(const bool &) { ui->table->selectAll(); }

void WidgetLogisticaColeta::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

// TODO: adicionar OC, numero da nota e idVenda
// TODO: colocar selecao de multiplas linhas

void WidgetLogisticaColeta::on_lineEditBuscaColeta_textChanged(const QString &) {
  const QString textoBusca = ui->lineEditBuscaColeta->text();
  model.setFilter("(numeroNFe LIKE '%" + textoBusca + "%') or (produto LIKE '%" + textoBusca +
                  "%') or (idVenda LIKE '%" + textoBusca + "%') or (ordemCompra LIKE '%" + textoBusca +
                  "%')"); // nfe, produto, venda, oc
}

// TODO: verificar como lidar com os estoques que nao possuem venda atrelada, como marcar datas previstas?
// TODO: algumas caixas com valor negativo (verificar como elas estão entrando no estoque)
// TODO: busca: ao limpar o campo ele apaga a selecao de fornecedor
