#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "doubledelegate.h"
#include "ui_widgetnfeentrada.h"
#include "widgetnfeentrada.h"
#include "xml_viewer.h"

WidgetNfeEntrada::WidgetNfeEntrada(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetNfeEntrada) { ui->setupUi(this); }

WidgetNfeEntrada::~WidgetNfeEntrada() { delete ui; }

bool WidgetNfeEntrada::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela NFe: " + model.lastError().text());
    return false;
  }

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetNfeEntrada::setupTables() {
  model.setTable("view_nfe_entrada");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());

  ui->table->setModel(&model);
  ui->table->hideColumn("idNFe");
  ui->table->setItemDelegate(new DoubleDelegate(this));
}

void WidgetNfeEntrada::on_table_activated(const QModelIndex &index) {
  QSqlQuery query;
  query.prepare("SELECT xml FROM nfe WHERE idNFe = :idNFe");
  query.bindValue(":idNFe", model.data(index.row(), "idNFe"));

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando xml da nota: " + query.lastError().text());
    return;
  }

  auto *viewer = new XML_Viewer(this);
  viewer->setAttribute(Qt::WA_DeleteOnClose);
  viewer->exibirXML(query.value("xml").toByteArray());
}

void WidgetNfeEntrada::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetNfeEntrada::on_lineEditBusca_textChanged(const QString &text) {
  model.setFilter("NFe LIKE '%" + text + "%' OR OC LIKE '%" + text + "%' OR Venda LIKE '%" + text + "%'");

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
}

void WidgetNfeEntrada::on_pushButtonCancelarNFe_clicked() {
  // TODO: 1quando cancelar nota pegar os estoques e cancelar/remover da logistica (exceto quando estiverem entregues?)
  // TODO: confirmar antes de cancelar
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhuma linha selecionada!");
    return;
  }

  QMessageBox msgBox(QMessageBox::Question, "Cancelar?", "Tem certeza que deseja cancelar?", QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Cancelar");
  msgBox.setButtonText(QMessageBox::No, "Voltar");

  if (msgBox.exec() == QMessageBox::No) return;

  const int row = list.first().row();

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not cancelar(row)) {
    QSqlQuery("ROLLBACK").exec();
    if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
    return;
  }

  QSqlQuery("COMMIT").exec();

  updateTables();
  QMessageBox::information(this, "Aviso!", "Cancelado com sucesso!");
}

bool WidgetNfeEntrada::cancelar(const int row) {
  // marcar nota como cancelada
  QSqlQuery query;
  query.prepare("UPDATE nfe SET status = 'CANCELADO' WHERE idNFe = :idNFe");
  query.bindValue(":idNFe", model.data(row, "idNFe"));

  if (not query.exec()) {
    error = "Erro cancelando nota: " + query.lastError().text();
    return false;
  }

  query.prepare("UPDATE estoque SET status = 'CANCELADO' WHERE idEstoque IN (SELECT idEstoque FROM estoque_has_nfe WHERE idNFe = :idNFe)");
  query.bindValue(":idNFe", model.data(row, "idNFe"));

  if (not query.exec()) {
    error = "Erro marcando estoque cancelado: " + query.lastError().text();
    return false;
  }

  // voltar compra para faturamento
  query.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'EM FATURAMENTO', dataRealFat = NULL, "
                "dataPrevColeta = NULL, dataRealColeta = NULL, dataPrevReceb = NULL, dataRealReceb = NULL, "
                "dataPrevEnt = NULL, dataRealEnt = NULL WHERE idCompra IN (SELECT idCompra FROM estoque_has_compra "
                "WHERE idEstoque IN (SELECT idEstoque FROM estoque_has_nfe WHERE idNFe = :idNFe))");
  query.bindValue(":idNFe", model.data(row, "idNFe"));

  if (not query.exec()) {
    error = "Erro voltando compra para faturamento: " + query.lastError().text();
    return false;
  }

  // desvincular produtos associados (se houver)
  query.prepare("SELECT idVendaProduto FROM estoque_has_consumo WHERE idEstoque IN (SELECT idEstoque FROM estoque_has_nfe WHERE idNFe = :idNFe)");
  query.bindValue(":idNFe", model.data(row, "idNFe"));

  if (not query.exec()) {
    error = "Erro buscando consumos: " + query.lastError().text();
    return false;
  }

  while (query.next()) {
    QSqlQuery query2;
    query2.prepare("DELETE FROM estoque_has_consumo WHERE idVendaProduto = :idVendaProduto");
    query2.bindValue(":idVendaProduto", query.value("idVendaProduto"));

    if (not query2.exec()) {
      error = "Erro removendo consumos: " + query.lastError().text();
      return false;
    }

    // voltar status para pendente
    query2.prepare("UPDATE venda_has_produto SET status = 'PENDENTE', dataPrevCompra = NULL, dataRealCompra = NULL, "
                   "dataPrevConf = NULL, dataRealConf = NULL, dataPrevFat = NULL, dataRealFat = NULL, dataPrevColeta = NULL, "
                   "dataRealColeta = NULL, dataPrevReceb = NULL, dataRealReceb = NULL, dataPrevEnt = NULL, "
                   "dataRealEnt = NULL WHERE idVendaProduto = :idVendaProduto");
    query2.bindValue(":idVendaProduto", query.value("idVendaProduto"));

    if (not query2.exec()) {
      error = "Erro voltando produtos para pendente: " + query2.lastError().text();
      return false;
    }
  }

  return true;
}

// TODO: copiar filtros do widgetnfesaida
