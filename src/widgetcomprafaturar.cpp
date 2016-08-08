#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "importarxml.h"
#include "inputdialog.h"
#include "reaisdelegate.h"
#include "ui_widgetcomprafaturar.h"
#include "widgetcomprafaturar.h"

WidgetCompraFaturar::WidgetCompraFaturar(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompraFaturar) {
  ui->setupUi(this);
}

WidgetCompraFaturar::~WidgetCompraFaturar() { delete ui; }

void WidgetCompraFaturar::setupTables() {
  model.setTable("view_faturamento");

  ui->table->setModel(&model);
  ui->table->setItemDelegateForColumn("Total", new ReaisDelegate(this));
  ui->table->hideColumn("representacao");
}

bool WidgetCompraFaturar::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  QString filter = model.filter();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela faturamento: " + model.lastError().text());
    return false;
  }

  model.setFilter(filter);

  ui->table->resizeColumnsToContents();

  return true;
}

bool WidgetCompraFaturar::faturarCompra() {
  auto list = ui->table->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Não selecionou nenhuma compra!");
    return false;
  }

  QSqlQuery query;
  query.prepare("SELECT representacao FROM fornecedor WHERE razaoSocial = :razaoSocial");
  query.bindValue(":razaoSocial", model.data(list.first().row(), "Fornecedor"));

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro verificando se fornecedor é representação: " + query.lastError().text());
    return false;
  }

  bool representacao = query.value("representacao").toBool();

  QStringList idsCompra;

  for (auto item : list) idsCompra << model.data(item.row(), "Compra").toString();

  InputDialog *inputDlg = new InputDialog(InputDialog::Faturamento, this);
  if (not inputDlg->setFilter(idsCompra)) return false;
  if (inputDlg->exec() != InputDialog::Accepted) return false;

  QString dataReal = inputDlg->getDate().toString("yyyy-MM-dd");

  if (representacao) {
    for (auto idCompra : idsCompra) {
      query.prepare("UPDATE pedido_fornecedor_has_produto SET dataRealFat = :dataRealFat, status = 'EM COLETA' WHERE "
                    "idCompra = :idCompra");
      query.bindValue(":dataRealFat", dataReal);
      query.bindValue(":idCompra", idCompra);

      if (not query.exec()) {
        QMessageBox::critical(this, "Erro!", "Erro atualizando status da compra: " + query.lastError().text());
        close();
        return false;
      }
    }
  } else {
    ImportarXML *import = new ImportarXML(idsCompra, dataReal, this);
    import->showMaximized();

    if (import->exec() != QDialog::Accepted) return false;
  }

  // salvar status na venda
  for (auto idCompra : idsCompra) {
    query.prepare("UPDATE venda_has_produto SET dataRealFat = :dataRealFat WHERE idCompra = :idCompra");
    query.bindValue(":dataRealFat", dataReal);
    query.bindValue(":idCompra", idCompra);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando status da venda: " + query.lastError().text());
      close();
      return false;
    }
  }

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    close();
    return false;
  }

  return true;
}

void WidgetCompraFaturar::on_pushButtonMarcarFaturado_clicked() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not faturarCompra()) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  if (not updateTables()) return;

  QMessageBox::information(this, "Aviso!", "Confirmado faturamento.");
}

void WidgetCompraFaturar::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetCompraFaturar::on_checkBoxRepresentacao_toggled(bool checked) {
  if (checked) model.setFilter("representacao = 1");
  if (not checked) model.setFilter("representacao = 0");
}
