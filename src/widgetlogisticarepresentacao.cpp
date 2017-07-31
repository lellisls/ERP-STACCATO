#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "estoqueprazoproxymodel.h"
#include "inputdialogconfirmacao.h"
#include "ui_widgetlogisticarepresentacao.h"
#include "widgetlogisticarepresentacao.h"

WidgetLogisticaRepresentacao::WidgetLogisticaRepresentacao(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetLogisticaRepresentacao) { ui->setupUi(this); }

WidgetLogisticaRepresentacao::~WidgetLogisticaRepresentacao() { delete ui; }

bool WidgetLogisticaRepresentacao::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return false;
  }

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetLogisticaRepresentacao::tableFornLogistica_activated(const QString &fornecedor) {
  this->fornecedor = fornecedor;

  ui->lineEditBusca->clear();

  model.setFilter("fornecedor = '" + fornecedor + "'");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return;
  }

  ui->table->sortByColumn("prazoEntrega", Qt::AscendingOrder);

  ui->table->resizeColumnsToContents();
}

void WidgetLogisticaRepresentacao::setupTables() {
  model.setTable("view_logistica_representacao");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  model.setHeaderData("idVenda", "Venda");
  model.setHeaderData("cliente", "Cliente");
  model.setHeaderData("descricao", "Produto");
  model.setHeaderData("codComercial", "Cód. Com.");
  model.setHeaderData("quant", "Quant.");
  model.setHeaderData("un", "Un.");
  model.setHeaderData("caixas", "Cx.");
  model.setHeaderData("kgcx", "Kg./Cx.");
  model.setHeaderData("ordemCompra", "OC");
  model.setHeaderData("prazoEntrega", "Prazo Limite");

  model.setFilter("0");

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());

  ui->table->setModel(new EstoquePrazoProxyModel(&model, this));
  ui->table->hideColumn("idPedido");
  ui->table->hideColumn("fornecedor");
  ui->table->hideColumn("status");
  ui->table->hideColumn("idVendaProduto");
}

void WidgetLogisticaRepresentacao::on_pushButtonMarcarEntregue_clicked() {
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  InputDialogConfirmacao input(InputDialogConfirmacao::Representacao);

  if (input.exec() != InputDialogConfirmacao::Accepted) return;

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not processRows(list, input.getDate())) {
    QSqlQuery("ROLLBACK").exec();
    if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
    return;
  }

  QSqlQuery("COMMIT").exec();

  updateTables();
  QMessageBox::information(this, "Aviso!", "Atualizado!");
}

bool WidgetLogisticaRepresentacao::processRows(const QModelIndexList &list, const QDateTime &dataEntrega) {
  QSqlQuery query1;
  query1.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'ENTREGUE', "
                 "dataRealEnt = :dataRealEnt WHERE idVendaProduto = :idVendaProduto");

  QSqlQuery query;
  query.prepare("UPDATE venda_has_produto SET status = 'ENTREGUE', dataRealEnt = :dataRealEnt "
                "WHERE idVendaProduto = :idVendaProduto");

  for (auto const &item : list) {
    query1.bindValue(":dataRealEnt", dataEntrega);
    query1.bindValue(":idVendaProduto", model.data(item.row(), "idVendaProduto"));

    if (not query1.exec()) {
      error = "Erro salvando status no pedido_fornecedor: " + query1.lastError().text();
      return false;
    }

    query.bindValue(":dataRealEnt", dataEntrega);
    query.bindValue(":idVendaProduto", model.data(item.row(), "idVendaProduto"));

    if (not query.exec()) {
      error = "Erro salvando status na venda_produto: " + query.lastError().text();
      return false;
    }
  }

  return true;
}

void WidgetLogisticaRepresentacao::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetLogisticaRepresentacao::on_lineEditBusca_textChanged(const QString &text) {
  model.setFilter("(idVenda LIKE '%" + text + "%' OR cliente LIKE '%" + text + "%')");

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
}

// TODO: 2palimanan precisa de coleta/recebimento (colocar flag no cadastro dizendo que entra no fluxo de logistica)
