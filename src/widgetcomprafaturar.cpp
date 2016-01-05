#include <QDate>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "importarxml.h"
#include "inputdialog.h"
#include "ui_widgetcomprafaturar.h"
#include "widgetcomprafaturar.h"

WidgetCompraFaturar::WidgetCompraFaturar(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompraFaturar) {
  ui->setupUi(this);

  setupTables();
}

WidgetCompraFaturar::~WidgetCompraFaturar() { delete ui; }

void WidgetCompraFaturar::tableFornCompras_activated(const QString &fornecedor) {
  modelFat.setFilter("fornecedor = '" + fornecedor + "' AND status = 'EM FATURAMENTO'");

  if (not modelFat.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + modelFat.lastError().text());
    return;
  }
}

void WidgetCompraFaturar::setupTables() {
  modelFat.setTable("view_faturamento");

  modelFat.setHeaderData("fornecedor", "Fornecedor");
  modelFat.setHeaderData("idCompra", "Compra");
  modelFat.setHeaderData("COUNT(idProduto)", "Itens");
  modelFat.setHeaderData("SUM(preco)", "Preço");
  modelFat.setHeaderData("status", "Status");

  ui->tableFaturamento->setModel(&modelFat);
}

QString WidgetCompraFaturar::updateTables() {
  if (not modelFat.select()) {
    return "Erro lendo tabela faturamento: " + modelFat.lastError().text();
  }

  ui->tableFaturamento->resizeColumnsToContents();

  return QString();
}

void WidgetCompraFaturar::on_pushButtonMarcarFaturado_clicked() {
  if (ui->tableFaturamento->selectionModel()->selectedRows().size() == 0) {
    QMessageBox::critical(this, "Erro!", "Não selecionou nenhuma compra!");
    return;
  }

  int row = ui->tableFaturamento->selectionModel()->selectedRows().first().row();

  ImportarXML *import = new ImportarXML(this);
  import->filtrar(modelFat.data(row, "Fornecedor").toString());
  import->showMaximized();

  if (import->exec() != QDialog::Accepted) {
    return;
  }

  QString const idCompra = import->getIdCompra();
  //----------------------------------------------------------//

  InputDialog *inputDlg = new InputDialog(InputDialog::Faturamento, this);
  //  inputDlg->setFilter(idCompra);

  if (inputDlg->exec() != InputDialog::Accepted) {
    return;
  }

  const QDate dataFat = inputDlg->getDate();
  const QDate dataPrevista = inputDlg->getNextDate();

  QSqlQuery query;

  if (not query.exec("UPDATE pedido_fornecedor_has_produto SET dataRealFat = '" + dataFat.toString("yyyy-MM-dd") +
                     "', dataPrevColeta = '" + dataPrevista.toString("yyyy-MM-dd") +
                     "', status = 'EM COLETA' WHERE idCompra = " + idCompra + "")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status da compra: " + query.lastError().text());
    return;
  }

  // salvar status na venda
  if (not query.exec("UPDATE venda_has_produto SET dataRealFat = '" + dataFat.toString("yyyy-MM-dd") +
                     "', dataPrevColeta = '" + dataPrevista.toString("yyyy-MM-dd") +
                     "', status = 'EM COLETA' WHERE idCompra = " + idCompra + "")) {
    QMessageBox::critical(this, "Erro!", "Erro salvando status da venda: " + query.lastError().text());
    return;
  }

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    return;
  }

  updateTables();

  QMessageBox::information(this, "Aviso!", "Confirmado faturamento.");
}
