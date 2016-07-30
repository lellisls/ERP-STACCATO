#include <QDate>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "ui_widgetlogisticarepresentacao.h"
#include "widgetlogisticarepresentacao.h"

WidgetLogisticaRepresentacao::WidgetLogisticaRepresentacao(QWidget *parent)
    : QWidget(parent), ui(new Ui::WidgetLogisticaRepresentacao) {
  ui->setupUi(this);
}

WidgetLogisticaRepresentacao::~WidgetLogisticaRepresentacao() { delete ui; }

QString WidgetLogisticaRepresentacao::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  model.setFilter("0");

  if (not model.select()) return "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text();

  //    for (int row = 0; row < model.rowCount(); ++row) {
  //      ui->table->openPersistentEditor(row, "selecionado");
  //    }

  ui->table->resizeColumnsToContents();

  return QString();
}

void WidgetLogisticaRepresentacao::TableFornLogistica_activated(const QString &fornecedor) {
  model.setFilter("fornecedor = '" + fornecedor + "' AND status = 'EM COLETA'");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return;
  }

  //    for (int row = 0; row < model.rowCount(); ++row) {
  //      ui->table->openPersistentEditor(row, "selecionado");
  //    }

  ui->table->resizeColumnsToContents();
}

void WidgetLogisticaRepresentacao::setupTables() {
  model.setTable("pedido_fornecedor_has_produto");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  model.setFilter("status = 'EM RECEBIMENTO'");

  ui->table->setModel(&model);
}

void WidgetLogisticaRepresentacao::on_pushButtonMarcarEntregue_clicked() {}

void WidgetLogisticaRepresentacao::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

// NOTE: botoes dinamicos de acordo com o status
// NOTE: botao para encerrar ciclo (entrega direta)
// NOTE: todos os itens na mesma tela
