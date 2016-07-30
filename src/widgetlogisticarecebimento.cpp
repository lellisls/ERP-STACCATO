#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "inputdialog.h"
#include "ui_widgetlogisticarecebimento.h"
#include "widgetlogisticarecebimento.h"

WidgetLogisticaRecebimento::WidgetLogisticaRecebimento(QWidget *parent)
    : QWidget(parent), ui(new Ui::WidgetLogisticaRecebimento) {
  ui->setupUi(this);
}

WidgetLogisticaRecebimento::~WidgetLogisticaRecebimento() { delete ui; }

bool WidgetLogisticaRecebimento::updateTables() {
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

void WidgetLogisticaRecebimento::TableFornLogistica_activated(const QString &fornecedor) {
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

void WidgetLogisticaRecebimento::setupTables() {
  //  model.setTable("estoque");
  model.setTable("view_recebimento");
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

void WidgetLogisticaRecebimento::on_pushButtonMarcarRecebido_clicked() {
  auto lista = ui->table->selectionModel()->selectedRows();

  if (lista.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  QString filtro = model.filter();

  InputDialog *inputDlg = new InputDialog(InputDialog::Recebimento, this);

  if (inputDlg->exec() != InputDialog::Accepted) return;

  const QString dataReceb = inputDlg->getDate().toString("yyyy-MM-dd");

  model.setFilter(filtro);
  model.select();

  for (const auto item : lista) {
    if (not model.setData(item.row(), "status", "ESTOQUE")) {
      QMessageBox::critical(this, "Erro!", "Erro marcando status ESTOQUE: " + model.lastError().text());
      return;
    }

    qDebug() << model.data(item.row(), "idCompra").toString();
    return;

    // salvar status na venda
    QSqlQuery query;

    // TODO: finish
    //        query.prepare("UPDATE venda_has_produto SET dataRealReceb = :dataRealReceb, status = 'ESTOQUE' WHERE
    //        idCompra
    //    IN (SELECT idCompra FROM )")

    //    if (not query.exec("UPDATE venda_has_produto SET dataRealReceb = '" + dataReceb +
    //                       "', status = 'ESTOQUE' WHERE idCompra = " +
    //                       model.data(item.row(), "idCompra").toString().replace(",", " OR idCompra = "))) {
    //      QMessageBox::critical(this, "Erro!", "Erro atualizando status da venda: " + query.lastError().text());
    //      return;
    //    }

    query.prepare("UPDATE estoque SET status = 'CONSUMO' WHERE status = 'PRÉ-CONSUMO' AND idEstoque = :idEstoque");
    query.bindValue(":idEstoque", model.data(item.row(), "idEstoque"));

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

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro salvando dados da tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return;
  }

  if (not updateTables()) return;

  QMessageBox::information(this, "Aviso!", "Confirmado recebimento.");
}

void WidgetLogisticaRecebimento::on_checkBoxMarcarTodos_clicked(const bool &) { ui->table->selectAll(); }

void WidgetLogisticaRecebimento::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetLogisticaRecebimento::on_lineEditBuscaRecebimento_textChanged(const QString &text) {
  model.setFilter("(numeroNFe LIKE '%" + text + "%') or (produto LIKE '%" + text + "%') or (idVenda LIKE '%" + text +
                  "%') or (ordemCompra LIKE '%" + text + "%')"); // nfe, produto, venda, oc
}
