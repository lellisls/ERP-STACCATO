#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "estoqueprazoproxymodel.h"
#include "inputdialog.h"
#include "inputdialogconfirmacao.h"
#include "ui_widgetlogisticarecebimento.h"
#include "venda.h"
#include "widgetlogisticarecebimento.h"

WidgetLogisticaRecebimento::WidgetLogisticaRecebimento(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetLogisticaRecebimento) { ui->setupUi(this); }

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

void WidgetLogisticaRecebimento::tableFornLogistica_activated(const QString &fornecedor) {
  this->fornecedor = fornecedor;

  ui->lineEditBusca->clear();

  model.setFilter("fornecedor = '" + fornecedor + "'");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return;
  }

  ui->checkBoxMarcarTodos->setChecked(false);

  ui->table->resizeColumnsToContents();
}

void WidgetLogisticaRecebimento::setupTables() {
  model.setTable("view_recebimento");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  model.setHeaderData("idEstoque", "Estoque");
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
  model.setHeaderData("prazoEntrega", "Prazo Limite");

  model.setFilter("0");

  ui->table->setModel(new EstoquePrazoProxyModel(&model, this));
  ui->table->hideColumn("fornecedor");
  ui->table->hideColumn("prazoEntrega");
  ui->table->hideColumn("ordemCompra");
  ui->table->hideColumn("local");
  ui->table->hideColumn("");
}

bool WidgetLogisticaRecebimento::processRows(const QModelIndexList &list, const QDateTime &dataReceb, const QString &recebidoPor) {
  QSqlQuery query;

  for (auto const &item : list) {
    query.prepare("UPDATE estoque SET status = 'ESTOQUE', recebidoPor = :recebidoPor WHERE idEstoque = :idEstoque");
    query.bindValue(":recebidoPor", recebidoPor);
    query.bindValue(":idEstoque", model.data(item.row(), "idEstoque"));

    if (not query.exec()) {
      error = "Erro atualizando status do estoque: " + query.lastError().text();
      return false;
    }

    query.prepare("UPDATE estoque_has_consumo SET status = 'CONSUMO' WHERE status = 'PRÉ-CONSUMO' AND idEstoque = :idEstoque");
    query.bindValue(":idEstoque", model.data(item.row(), "idEstoque"));

    if (not query.exec()) {
      error = "Erro atualizando status da venda: " + query.lastError().text();
      return false;
    }

    query.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'ESTOQUE', dataRealReceb = :dataRealReceb "
                  "WHERE idCompra IN (SELECT idCompra FROM estoque_has_compra WHERE idEstoque = :idEstoque) AND "
                  "codComercial = :codComercial");
    query.bindValue(":dataRealReceb", dataReceb);
    query.bindValue(":idEstoque", model.data(item.row(), "idEstoque"));
    query.bindValue(":codComercial", model.data(item.row(), "codComercial"));

    if (not query.exec()) {
      error = "Erro atualizando status da compra: " + query.lastError().text();
      return false;
    }

    // salvar status na venda
    query.prepare("UPDATE venda_has_produto SET status = 'ESTOQUE', dataRealReceb = :dataRealReceb WHERE idVendaProduto IN "
                  "(SELECT idVendaProduto FROM estoque_has_consumo WHERE idEstoque = :idEstoque)");
    query.bindValue(":dataRealReceb", dataReceb);
    query.bindValue(":idEstoque", model.data(item.row(), "idEstoque"));

    if (not query.exec()) {
      error = "Erro atualizando produtos venda: " + query.lastError().text();
      return false;
    }
  }

  return true;
}

void WidgetLogisticaRecebimento::on_pushButtonMarcarRecebido_clicked() {
  // TODO: 1gerar gare da nota de entrada

  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  QStringList ids;

  for (auto const &item : list) ids.append(model.data(item.row(), "idEstoque").toString());

  InputDialogConfirmacao inputDlg(InputDialogConfirmacao::Recebimento);
  inputDlg.setFilter(ids);

  if (inputDlg.exec() != InputDialogConfirmacao::Accepted) return;

  const QDateTime dataReceb = inputDlg.getDate();
  const QString recebidoPor = inputDlg.getRecebeu();

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not processRows(list, dataReceb, recebidoPor)) {
    QSqlQuery("ROLLBACK").exec();
    if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
    return;
  }

  QSqlQuery("COMMIT").exec();

  QSqlQuery query;

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    return;
  }

  updateTables();
  QMessageBox::information(this, "Aviso!", "Confirmado recebimento!");
}

void WidgetLogisticaRecebimento::on_checkBoxMarcarTodos_clicked(const bool) { ui->table->selectAll(); }

void WidgetLogisticaRecebimento::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetLogisticaRecebimento::on_lineEditBusca_textChanged(const QString &text) {
  model.setFilter("(numeroNFe LIKE '%" + text + "%' OR produto LIKE '%" + text + "%' OR idVenda LIKE '%" + text + "%' OR ordemCompra LIKE '%" + text + "%')");

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
}

void WidgetLogisticaRecebimento::on_pushButtonReagendar_clicked() {
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  InputDialog input(InputDialog::AgendarRecebimento);

  if (input.exec() != InputDialog::Accepted) return;

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not reagendar(list, input.getNextDate())) {
    QSqlQuery("ROLLBACK").exec();
    if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
    return;
  }

  QSqlQuery("COMMIT").exec();

  updateTables();
  QMessageBox::information(this, "Aviso!", "Reagendado com sucesso!");
}

bool WidgetLogisticaRecebimento::reagendar(const QModelIndexList &list, const QDate &dataPrevReceb) {
  for (const auto &item : list) {
    const int idEstoque = model.data(item.row(), "idEstoque").toInt();
    const QString codComercial = model.data(item.row(), "codComercial").toString();

    QSqlQuery query;
    query.prepare("UPDATE pedido_fornecedor_has_produto SET dataPrevReceb = :dataPrevReceb WHERE idCompra IN (SELECT "
                  "idCompra FROM estoque_has_compra WHERE idEstoque = :idEstoque) AND codComercial = :codComercial");
    query.bindValue(":dataPrevReceb", dataPrevReceb);
    query.bindValue(":idEstoque", idEstoque);
    query.bindValue(":codComercial", codComercial);

    if (not query.exec()) {
      error = "Erro salvando status no pedido_fornecedor: " + query.lastError().text();
      return false;
    }

    query.prepare("UPDATE venda_has_produto SET dataPrevReceb = :dataPrevReceb WHERE idCompra IN (SELECT idCompra FROM "
                  "estoque_has_compra WHERE idEstoque = :idEstoque) AND codComercial = :codComercial");
    query.bindValue(":dataPrevReceb", dataPrevReceb);
    query.bindValue(":idEstoque", idEstoque);
    query.bindValue(":codComercial", codComercial);

    if (not query.exec()) {
      error = "Erro salvando status na venda_produto: " + query.lastError().text();
      return false;
    }
  }

  return true;
}

void WidgetLogisticaRecebimento::on_pushButtonVenda_clicked() {
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  for (auto const &item : list) {
    const QString idVenda = model.data(item.row(), "idVenda").toString();
    const QStringList ids = idVenda.split(", ");

    if (ids.isEmpty()) return;

    for (auto const &id : ids) {
      auto *venda = new Venda(this);
      venda->setAttribute(Qt::WA_DeleteOnClose);
      venda->viewRegisterById(id);
    }
  }
}

void WidgetLogisticaRecebimento::on_pushButtonCancelar_clicked() {
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
  QMessageBox::information(this, "Aviso!", "Cancelado com sucesso!");
}

bool WidgetLogisticaRecebimento::cancelar(const QModelIndexList &list) {
  for (const auto &item : list) {
    const int idEstoque = model.data(item.row(), "idEstoque").toInt();
    const QString codComercial = model.data(item.row(), "codComercial").toString();

    QSqlQuery query;
    query.prepare("UPDATE pedido_fornecedor_has_produto SET dataPrevReceb = NULL WHERE idCompra IN (SELECT "
                  "idCompra FROM estoque_has_compra WHERE idEstoque = :idEstoque) AND codComercial = :codComercial");
    query.bindValue(":idEstoque", idEstoque);
    query.bindValue(":codComercial", codComercial);

    if (not query.exec()) {
      error = "Erro salvando status no pedido_fornecedor: " + query.lastError().text();
      return false;
    }

    query.prepare("UPDATE venda_has_produto SET dataPrevReceb = NULL WHERE idCompra IN (SELECT idCompra FROM "
                  "estoque_has_compra WHERE idEstoque = :idEstoque) AND codComercial = :codComercial");
    query.bindValue(":idEstoque", idEstoque);
    query.bindValue(":codComercial", codComercial);

    if (not query.exec()) {
      error = "Erro salvando status na venda_produto: " + query.lastError().text();
      return false;
    }
  }

  return true;
}

// TODO: 0marcar em qual bloco foi guardado (opcional?)
