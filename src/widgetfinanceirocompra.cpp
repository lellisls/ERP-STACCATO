#include <QMessageBox>
#include <QSqlError>

#include "inputdialog.h"
#include "reaisdelegate.h"
#include "ui_widgetfinanceirocompra.h"
#include "widgetfinanceirocompra.h"

WidgetFinanceiroCompra::WidgetFinanceiroCompra(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetFinanceiroCompra) {
  ui->setupUi(this);
}

WidgetFinanceiroCompra::~WidgetFinanceiroCompra() { delete ui; }

bool WidgetFinanceiroCompra::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela de compras: " + model.lastError().text());
    return false;
  }

  return true;
}

void WidgetFinanceiroCompra::setupTables() {
  model.setTable("view_compras_financeiro");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela de compras: " + model.lastError().text());
  }

  ui->table->setModel(&model);
  ui->table->setItemDelegateForColumn("Total", new ReaisDelegate(this));
}

void WidgetFinanceiroCompra::on_table_activated(const QModelIndex &index) {
  InputDialog *input = new InputDialog(InputDialog::Financeiro, this);
  input->setFilter(model.data(index.row(), "Compra").toString());

  if (input->exec() != InputDialog::Accepted) return;
}

void WidgetFinanceiroCompra::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetFinanceiroCompra::on_lineEditBusca_textChanged(const QString &text) {
  // TODO: implement searching
}
