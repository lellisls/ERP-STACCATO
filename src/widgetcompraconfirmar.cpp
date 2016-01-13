#include <QDate>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "inputdialog.h"
#include "ui_widgetcompraconfirmar.h"
#include "widgetcompraconfirmar.h"

WidgetCompraConfirmar::WidgetCompraConfirmar(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompraConfirmar) {
  ui->setupUi(this);

  setupTables();
}

WidgetCompraConfirmar::~WidgetCompraConfirmar() { delete ui; }

void WidgetCompraConfirmar::setupTables() {
  model.setTable("view_compras");

  model.setHeaderData("fornecedor", "Fornecedor");
  model.setHeaderData("idCompra", "Compra");
  model.setHeaderData("COUNT(idProduto)", "Itens");
  model.setHeaderData("SUM(preco)", "Preço");
  model.setHeaderData("status", "Status");

  ui->table->setModel(&model);
}

QString WidgetCompraConfirmar::updateTables() {
  if (not model.select()) return "Erro lendo tabela compras: " + model.lastError().text();

  ui->table->resizeColumnsToContents();

  return QString();
}

void WidgetCompraConfirmar::on_pushButtonConfirmarCompra_clicked() {
  QString idCompra;

  if (ui->table->selectionModel()->selectedRows().size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  int row = ui->table->selectionModel()->selectedRows().first().row();
  idCompra = model.data(row, "idCompra").toString();

  InputDialog *inputDlg = new InputDialog(InputDialog::ConfirmarCompra, this);
  inputDlg->setFilter(idCompra);

  if (inputDlg->exec() != InputDialog::Accepted) return;

  const QDate dataConf = inputDlg->getDate();
  const QDate dataPrevista = inputDlg->getNextDate();

  QSqlQuery query;
  query.prepare("UPDATE pedido_fornecedor_has_produto SET dataRealConf = :dataRealConf, dataPrevFat = :dataPrevFat, "
                "status = 'EM FATURAMENTO' WHERE idCompra = :idCompra");
  query.bindValue(":dataRealConf", dataConf);
  query.bindValue(":dataPrevFat", dataPrevista);
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status da compra: " + query.lastError().text());
    return;
  }

  // salvar status na venda
  query.prepare("UPDATE venda_has_produto SET dataRealConf = :dataRealConf, dataPrevFat = :dataPrevFat, status = 'EM "
                "FATURAMENTO' WHERE idCompra = :idCompra");
  query.bindValue(":dataRealConf", dataConf);
  query.bindValue(":dataPrevFat", dataPrevista);
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando status da venda: " + query.lastError().text());
    return;
  }

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    return;
  }
  //

  updateTables();

  QMessageBox::information(this, "Aviso!", "Confirmado compra.");
}