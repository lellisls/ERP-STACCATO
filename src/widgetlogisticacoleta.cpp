#include <QDate>
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

bool WidgetLogisticaColeta::updateTables(QString &error) {
  if (model.tableName().isEmpty()) setupTables();

  model.setFilter("0");

  if (not model.select()) {
    error = "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text();
    return false;
  }

  for (int row = 0; row < model.rowCount(); ++row) {
    ui->table->openPersistentEditor(row, "selecionado");
  }

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetLogisticaColeta::TableFornLogistica_activated(const QString &fornecedor) {
  model.setFilter("fornecedor = '" + fornecedor + "' AND status = 'EM COLETA'");

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
  model.setTable("estoque");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  model.setFilter("status = 'EM COLETA'");

  ui->table->setModel(&model);
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
    if (not model.setData(item.row(), "status", "EM RECEBIMENTO")) {
      QMessageBox::critical(this, "Erro!", "Erro marcando status EM RECEBIMENTO: " + model.lastError().text());
      return;
    }

    // salvar status na venda
    QSqlQuery query;

    if (not query.exec("UPDATE venda_has_produto SET dataRealColeta = '" + dataColeta + "', dataPrevReceb = '" +
                       dataPrevista + "', status = 'EM RECEBIMENTO' WHERE idCompra = " +
                       model.data(item.row(), "idCompra").toString().replace(",", " OR idCompra = "))) {
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

  QString error;

  if (not updateTables(error)) QMessageBox::critical(this, "Erro!", error);

  QMessageBox::information(this, "Aviso!", "Confirmado coleta.");
}

void WidgetLogisticaColeta::on_checkBoxMarcarTodos_clicked(const bool &) {
  for (int row = 0; row < model.rowCount(); ++row) {
    ui->table->selectRow(row);
  }
}

void WidgetLogisticaColeta::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }
