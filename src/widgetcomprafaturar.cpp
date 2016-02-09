#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "importarxml.h"
#include "inputdialog.h"
#include "ui_widgetcomprafaturar.h"
#include "widgetcomprafaturar.h"

WidgetCompraFaturar::WidgetCompraFaturar(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompraFaturar) {
  ui->setupUi(this);
}

WidgetCompraFaturar::~WidgetCompraFaturar() { delete ui; }

void WidgetCompraFaturar::tableFornCompras_activated(const QString &fornecedor) {
  model.setFilter("fornecedor = '" + fornecedor + "' AND status = 'EM FATURAMENTO'");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return;
  }
}

void WidgetCompraFaturar::setupTables() {
  model.setTable("view_faturamento");

  ui->table->setModel(&model);
}

QString WidgetCompraFaturar::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) {
    return "Erro lendo tabela faturamento: " + model.lastError().text();
  }

  ui->table->resizeColumnsToContents();

  return QString();
}

void WidgetCompraFaturar::on_pushButtonMarcarFaturado_clicked() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (ui->table->selectionModel()->selectedRows().size() == 0) {
    QMessageBox::critical(this, "Erro!", "Não selecionou nenhuma compra!");
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  auto list = ui->table->selectionModel()->selectedRows();

  QString idCompra;

  for (auto item : list) {
    QString id = model.data(item.row(), "idCompra").toString();
    idCompra += idCompra.isEmpty() ? id : " OR idCompra = " + id;
  }

  ImportarXML *import = new ImportarXML(idCompra, this);
  import->showMaximized();

  if (import->exec() != QDialog::Accepted) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  //----------------------------------------------------------//

  InputDialog *inputDlg = new InputDialog(InputDialog::Faturamento, this);

  if (inputDlg->exec() != InputDialog::Accepted) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  const QDate dataFat = inputDlg->getDate();
  const QDate dataPrevista = inputDlg->getNextDate();

  QSqlQuery query;

  if (not query.exec("UPDATE pedido_fornecedor_has_produto SET dataRealFat = '" + dataFat.toString("yyyy-MM-dd") +
                     "', dataPrevColeta = '" + dataPrevista.toString("yyyy-MM-dd") +
                     "', status = 'EM COLETA' WHERE idCompra = " + idCompra + "")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status da compra: " + query.lastError().text());
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  // salvar status na venda
  if (not query.exec("UPDATE venda_has_produto SET dataRealFat = '" + dataFat.toString("yyyy-MM-dd") +
                     "', dataPrevColeta = '" + dataPrevista.toString("yyyy-MM-dd") +
                     "', status = 'EM COLETA' WHERE idCompra = " + idCompra + "")) {
    QMessageBox::critical(this, "Erro!", "Erro salvando status da venda: " + query.lastError().text());
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  updateTables();

  QSqlQuery("COMMIT").exec();

  QMessageBox::information(this, "Aviso!", "Confirmado faturamento.");
}

void WidgetCompraFaturar::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }
