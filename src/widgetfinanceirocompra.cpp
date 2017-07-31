#include <QMessageBox>
#include <QSqlError>

#include "inputdialogfinanceiro.h"
#include "reaisdelegate.h"
#include "ui_widgetfinanceirocompra.h"
#include "widgetfinanceirocompra.h"

WidgetFinanceiroCompra::WidgetFinanceiroCompra(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetFinanceiroCompra) { ui->setupUi(this); }

WidgetFinanceiroCompra::~WidgetFinanceiroCompra() { delete ui; }

bool WidgetFinanceiroCompra::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela de compras: " + model.lastError().text());
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
  ui->table->hideColumn("desativado");
}

void WidgetFinanceiroCompra::on_table_activated(const QModelIndex &index) {
  InputDialogFinanceiro input(InputDialogFinanceiro::Financeiro);
  input.setFilter(model.data(index.row(), "Compra").toString());

  if (input.exec() != InputDialogFinanceiro::Accepted) return;
}

void WidgetFinanceiroCompra::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetFinanceiroCompra::on_lineEditBusca_textChanged(const QString &text) {
  const QString filtroBusca = text.isEmpty() ? "" : "OC LIKE '%" + text + "%' OR Código LIKE '%" + text + "%'";

  model.setFilter(filtroBusca);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
  }
}

// TODO: quando recalcula fluxo deve ter um campo para digitar/calcular ST pois o antigo é substituido e não é criado um
// novo
// TODO: associar notas com cada produto? e verificar se dá para refazer/ajustar o fluxo de pagamento de acordo com as
// duplicatas da nota
