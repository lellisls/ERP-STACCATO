#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "estoqueprazoproxymodel.h"
#include "inputdialog.h"
#include "ui_widgetlogisticacoleta.h"
#include "venda.h"
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

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetLogisticaColeta::tableFornLogistica_activated(const QString &fornecedor) {
  this->fornecedor = fornecedor;

  ui->lineEditBusca->clear();

  model.setFilter("fornecedor = '" + fornecedor + "'");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return;
  }

  ui->checkBoxMarcarTodos->setChecked(false);

  ui->table->resizeColumnsToContents();
}

void WidgetLogisticaColeta::setupTables() {
  model.setTable("view_coleta");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  model.setHeaderData("idEstoque", "Estoque");
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
  model.setHeaderData("prazoEntrega", "Prazo Limite");

  model.setFilter("0");

  ui->table->setModel(new EstoquePrazoProxyModel(&model, this));
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

  updateTables();
  QMessageBox::information(this, "Aviso!", "Confirmado coleta.");
}

bool WidgetLogisticaColeta::cadastrar() {
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return false;
  }

  const QString filtro = model.filter();

  InputDialog inputDlg(InputDialog::Coleta);

  if (inputDlg.exec() != InputDialog::Accepted) return false;

  const QDateTime dataColeta = inputDlg.getDate();
  const QDateTime dataPrevReceb = inputDlg.getNextDate();

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
    query.bindValue(":codComercial", model.data(item.row(), "codComercial"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status da compra: " + query.lastError().text());
      return false;
    }

    //
  }

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    return false;
  }

  return true;
}

void WidgetLogisticaColeta::on_checkBoxMarcarTodos_clicked(const bool) { ui->table->selectAll(); }

void WidgetLogisticaColeta::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetLogisticaColeta::on_lineEditBusca_textChanged(const QString &) {
  const QString textoBusca = ui->lineEditBusca->text();

  model.setFilter("fornecedor = '" + fornecedor + "' AND (numeroNFe LIKE '%" + textoBusca + "%' OR produto LIKE '%" +
                  textoBusca + "%' OR idVenda LIKE '%" + textoBusca + "%' OR ordemCompra LIKE '%" + textoBusca + "%')");

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
}

void WidgetLogisticaColeta::on_pushButtonReagendar_clicked() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not reagendar()) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  updateTables();
  QMessageBox::information(this, "Aviso!", "Reagendado com sucesso!");
}

bool WidgetLogisticaColeta::reagendar() {
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return false;
  }

  InputDialog input(InputDialog::AgendarColeta);

  if (input.exec() != InputDialog::Accepted) return false;

  const QDateTime dataPrevColeta = input.getNextDate();

  for (const auto &item : list) {
    const int idEstoque = model.data(item.row(), "idEstoque").toInt();
    const QString codComercial = model.data(item.row(), "codComercial").toString();

    QSqlQuery query;
    query.prepare("UPDATE pedido_fornecedor_has_produto SET dataPrevColeta = :dataPrevColeta WHERE idCompra IN (SELECT "
                  "idCompra FROM estoque_has_compra WHERE idEstoque = :idEstoque) AND codComercial = :codComercial");
    query.bindValue(":dataPrevColeta", dataPrevColeta);
    query.bindValue(":idEstoque", idEstoque);
    query.bindValue(":codComercial", codComercial);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando status no pedido_fornecedor: " + query.lastError().text());
      return false;
    }

    query.prepare(
        "UPDATE venda_has_produto SET dataPrevColeta = :dataPrevColeta WHERE idCompra IN (SELECT idCompra FROM "
        "estoque_has_compra WHERE idEstoque = :idEstoque) AND codComercial = :codComercial");
    query.bindValue(":dataPrevColeta", dataPrevColeta);
    query.bindValue(":idEstoque", idEstoque);
    query.bindValue(":codComercial", codComercial);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando status na venda_produto: " + query.lastError().text());
      return false;
    }
  }

  return true;
}

void WidgetLogisticaColeta::on_table_doubleClicked(const QModelIndex &index) {
  const QString idVenda = model.data(index.row(), "idVenda").toString();

  if (idVenda.isEmpty()) return;

  Venda *venda = new Venda(this);
  venda->viewRegisterById(idVenda);
}
