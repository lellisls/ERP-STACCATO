#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "estoqueprazoproxymodel.h"
#include "inputdialogconfirmacao.h"
#include "ui_widgetlogisticarepresentacao.h"
#include "widgetlogisticarepresentacao.h"

WidgetLogisticaRepresentacao::WidgetLogisticaRepresentacao(QWidget *parent)
    : QWidget(parent), ui(new Ui::WidgetLogisticaRepresentacao) {
  ui->setupUi(this);
}

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

  model.setFilter("fornecedor = '" + fornecedor + "'");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return;
  }

  ui->table->sortByColumn("prazoEntrega", Qt::AscendingOrder);

  ui->table->resizeColumnsToContents();
}

void WidgetLogisticaRepresentacao::setupTables() {
  model.setTable("view_logistica_representacao");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  model.setHeaderData("idVenda", "Venda");
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
}

void WidgetLogisticaRepresentacao::on_pushButtonMarcarEntregue_clicked() {
  const auto list = ui->table->selectionModel()->selectedRows();

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

  updateTables();
  QMessageBox::information(this, "Aviso!", "Atualizado!");
}

bool WidgetLogisticaRepresentacao::processRows(const QModelIndexList &list) {
  InputDialogConfirmacao *input = new InputDialogConfirmacao(InputDialogConfirmacao::Entrega, this);

  if (input->exec() != InputDialogConfirmacao::Accepted) return false;

  const QDateTime dataEntrega = input->getDate();

  for (auto const &item : list) {
    QSqlQuery query;
    query.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'ENTREGUE', dataRealEnt = :dataRealEnt WHERE "
                  "idPedido = :idPedido");
    query.bindValue(":dataRealEnt", dataEntrega);
    query.bindValue(":idPedido", model.data(item.row(), "idPedido"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando status no pedido_fornecedor: " + query.lastError().text());
      return false;
    }

    query.prepare("UPDATE venda_has_produto SET status = 'ENTREGUE', dataRealEnt = :dataRealEnt WHERE idVenda = "
                  ":idVenda AND codComercial = :codComercial");
    query.bindValue(":dataRealEnt", dataEntrega);
    query.bindValue(":idVenda", model.data(item.row(), "idVenda"));
    query.bindValue(":codComercial", model.data(item.row(), "codComercial"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando status na venda_produto: " + query.lastError().text());
      return false;
    }
  }

  return true;
}

void WidgetLogisticaRepresentacao::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetLogisticaRepresentacao::on_lineEditBusca_textChanged(const QString &text) {
  // TODO: add cliente
  model.setFilter("fornecedor = '" + fornecedor + "' AND (idVenda LIKE '%" + text + "%')");

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
}
