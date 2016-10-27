#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "inputdialog.h"
#include "orcamentoproxymodel.h"
#include "ui_widgetlogisticarecebimento.h"
#include "widgetlogisticarecebimento.h"

WidgetLogisticaRecebimento::WidgetLogisticaRecebimento(QWidget *parent)
    : QWidget(parent), ui(new Ui::WidgetLogisticaRecebimento) {
  ui->setupUi(this);
}

WidgetLogisticaRecebimento::~WidgetLogisticaRecebimento() { delete ui; }

bool WidgetLogisticaRecebimento::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return false;
  }

  for (int row = 0; row < model.rowCount(); ++row) ui->table->openPersistentEditor(row, "selecionado");

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetLogisticaRecebimento::TableFornLogistica_activated(const QString &fornecedor) {
  this->fornecedor = fornecedor;

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

void WidgetLogisticaRecebimento::setupTables() {
  model.setTable("view_recebimento");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  model.setHeaderData("numeroNFe", "NFe");
  model.setHeaderData("produto", "Produto");
  model.setHeaderData("quant", "Quant.");
  model.setHeaderData("un", "Un.");
  model.setHeaderData("caixas", "Caixas");
  model.setHeaderData("idVenda", "Venda");
  model.setHeaderData("codComercial", "Cód. Com.");
  model.setHeaderData("ordemCompra", "OC");
  model.setHeaderData("local", "Local");
  model.setHeaderData("dataPrevReceb", "Data Prev. Rec.");

  model.setFilter("0");

  ui->table->setModel(new OrcamentoProxyModel(&model, this)); // TODO: make another proxyModel or refactor this one
  // ui->table->hideColumn("idEstoque");
  ui->table->hideColumn("fornecedor");
}

bool WidgetLogisticaRecebimento::processRows(QModelIndexList list) {
  const QString filtro = model.filter();

  InputDialog *inputDlg = new InputDialog(InputDialog::Recebimento, this);

  if (inputDlg->exec() != InputDialog::Accepted) return false;

  const QDate dataReceb = inputDlg->getDate();

  model.setFilter(filtro);
  model.select();

  QSqlQuery query;

  for (auto const &item : list) {
    query.prepare("UPDATE estoque SET status = 'ESTOQUE' WHERE idEstoque = :idEstoque");
    query.bindValue(":idEstoque", model.data(item.row(), "idEstoque"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status do estoque: " + query.lastError().text());
      return false;
    }

    query.prepare(
        "UPDATE estoque_has_consumo SET status = 'CONSUMO' WHERE status = 'PRÉ-CONSUMO' AND idEstoque = :idEstoque");
    query.bindValue(":idEstoque", model.data(item.row(), "idEstoque"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status da venda: " + query.lastError().text());
      return false;
    }

    query.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'EM RECEBIMENTO', dataRealReceb = :dataRealReceb "
                  "WHERE idCompra IN (SELECT idCompra FROM estoque_has_compra WHERE idEstoque = :idEstoque) AND "
                  "codComercial = :codComercial");
    query.bindValue(":dataRealReceb", dataReceb);
    query.bindValue(":idEstoque", model.data(item.row(), "idEstoque"));
    query.bindValue(":codComercial", model.data(item.row(), "codComercial"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status da compra: " + query.lastError().text());
      return false;
    }

    // salvar status na venda
    query.prepare(
        "UPDATE venda_has_produto SET status = 'ESTOQUE', dataRealReceb = :dataRealReceb WHERE idCompra IN "
        "(SELECT idCompra FROM estoque_has_compra WHERE idEstoque = :idEstoque) AND codComercial = :codComercial");
    query.bindValue(":dataRealReceb", dataReceb);
    query.bindValue(":idEstoque", model.data(item.row(), "idEstoque"));
    query.bindValue(":codComercial", model.data(item.row(), "codComercial"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando produtos venda: " + query.lastError().text());
      return false;
    }
  }

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    return false;
  }

  return true;
}

void WidgetLogisticaRecebimento::on_pushButtonMarcarRecebido_clicked() {
  auto list = ui->table->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not processRows(list)) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  QMessageBox::information(this, "Aviso!", "Confirmado recebimento!");
  updateTables();
}

void WidgetLogisticaRecebimento::on_checkBoxMarcarTodos_clicked(const bool &) { ui->table->selectAll(); }

void WidgetLogisticaRecebimento::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetLogisticaRecebimento::on_lineEditBuscaRecebimento_textChanged(const QString &text) {
  model.setFilter("fornecedor = '" + fornecedor + "' AND (numeroNFe LIKE '%" + text + "%' OR produto LIKE '%" + text +
                  "%' OR idVenda LIKE '%" + text + "%' OR ordemCompra LIKE '%" + text + "%')");

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
}

// TODO: verificar linhas com nfe vazio
