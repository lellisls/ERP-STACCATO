#include <QDate>
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
}

bool WidgetCompraConfirmar::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  QString filter = model.filter();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela compras: " + model.lastError().text());
    return false;
  }

  model.setFilter(filter);

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetCompraConfirmar::on_pushButtonConfirmarCompra_clicked() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not confirmarCompra()) QSqlQuery("ROLLBACK").exec();

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
  // TODO: atualizar apenas os que foram confirmados no InputDialog
  query.prepare("UPDATE pedido_fornecedor_has_produto SET dataRealConf = :dataRealConf, dataPrevFat = :dataPrevFat, "
                "status = 'EM FATURAMENTO' WHERE idCompra = :idCompra");
  query.bindValue(":dataRealConf", dataConf);
  query.bindValue(":dataPrevFat", dataPrevista);
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status da compra: " + query.lastError().text());
    return false;
  }

  // salvar status na venda
  query.prepare("UPDATE venda_has_produto SET dataRealConf = :dataRealConf, dataPrevFat = :dataPrevFat, status = 'EM "
                "FATURAMENTO' WHERE idCompra = :idCompra");
  query.bindValue(":dataRealConf", dataConf);
  query.bindValue(":dataPrevFat", dataPrevista);
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando status da venda: " + query.lastError().text());
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
// TODO: arrumar concatenacao (copiar do faturamento)
// TODO: representacao morre na confirmacao
// TODO: colocar frete
// TODO: confirmar produtos individualmente
