#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "inputdialog.h"
#include "reaisdelegate.h"
#include "ui_widgetcompraconfirmar.h"
#include "widgetcompraconfirmar.h"

WidgetCompraConfirmar::WidgetCompraConfirmar(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompraConfirmar) {
  ui->setupUi(this);
}

WidgetCompraConfirmar::~WidgetCompraConfirmar() { delete ui; }

void WidgetCompraConfirmar::setupTables() {
  model.setTable("view_compras");

  ui->table->setModel(&model);
  ui->table->setItemDelegateForColumn("Total", new ReaisDelegate(this));
  ui->table->hideColumn("Compra");
}

bool WidgetCompraConfirmar::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela compras: " + model.lastError().text());
    return false;
  }

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetCompraConfirmar::on_pushButtonConfirmarCompra_clicked() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not confirmarCompra()) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  if (not updateTables()) return;

  QMessageBox::information(this, "Aviso!", "Confirmado compra.");
}

bool WidgetCompraConfirmar::confirmarCompra() {
  if (ui->table->selectionModel()->selectedRows().size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return false;
  }

  int row = ui->table->selectionModel()->selectedRows().first().row();
  QString idCompra = model.data(row, "Compra").toString();

  InputDialog *inputDlg = new InputDialog(InputDialog::ConfirmarCompra, this);
  if (not inputDlg->setFilter(idCompra)) return false;

  if (inputDlg->exec() != InputDialog::Accepted) return false;

  const QDate dataConf = inputDlg->getDate();
  const QDate dataPrevista = inputDlg->getNextDate();

  QSqlQuery query;
  query.prepare("UPDATE pedido_fornecedor_has_produto SET dataRealConf = :dataRealConf, dataPrevFat = :dataPrevFat, "
                "status = 'EM FATURAMENTO' WHERE idCompra = :idCompra AND selecionado = 1");
  query.bindValue(":dataRealConf", dataConf);
  query.bindValue(":dataPrevFat", dataPrevista);
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status da compra: " + query.lastError().text());
    return false;
  }

  // salvar status na venda
  query.prepare("UPDATE venda_has_produto SET dataRealConf = :dataRealConf, dataPrevFat = :dataPrevFat, "
                "status = 'EM FATURAMENTO' WHERE idProduto IN (SELECT idProduto FROM pedido_fornecedor_has_produto "
                "WHERE idCompra = :idCompra AND selecionado = 1) AND idCompra = :idCompra");
  query.bindValue(":dataRealConf", dataConf);
  query.bindValue(":dataPrevFat", dataPrevista);
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando status da venda: " + query.lastError().text());
    return false;
  }

  query.prepare(
      "UPDATE pedido_fornecedor_has_produto SET selecionado = 0 WHERE idCompra = :idCompra AND selecionado = 1");
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro setando selecionado 0: " + query.lastError().text());
    return false;
  }

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    return false;
  }
  //

  return true;
}

void WidgetCompraConfirmar::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

// NOTE: permitir na tela de compras alterar uma venda para quebrar um produto em dois para os casos de lotes
// diferentes: 50 -> 40+10
// TODO: representacao morre na confirmacao
// TODO: mostrar datas previstas
