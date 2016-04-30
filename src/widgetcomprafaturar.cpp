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

void WidgetCompraFaturar::setupTables() {
  model.setTable("view_faturamento");

  ui->table->setModel(&model);
}

bool WidgetCompraFaturar::updateTables(QString &error) {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) {
    error = "Erro lendo tabela faturamento: " + model.lastError().text();
    return false;
  }

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetCompraFaturar::on_pushButtonMarcarFaturado_clicked() {
  if (ui->table->selectionModel()->selectedRows().size() == 0) {
    QMessageBox::critical(this, "Erro!", "Não selecionou nenhuma compra!");
    return;
  }

  auto list = ui->table->selectionModel()->selectedRows();

  QSqlQuery query;
  query.prepare("SELECT representacao FROM fornecedor WHERE razaoSocial = :razaoSocial");
  query.bindValue(":razaoSocial", model.data(list.first().row(), "Fornecedor"));

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro verificando se fornecedor é representação: " + query.lastError().text());
    return;
  }

  QString idCompra;

  for (auto item : list) {
    QString id = model.data(item.row(), "idCompra").toString();
    idCompra += idCompra.isEmpty() ? id : " OR idCompra = " + id;
  }

  InputDialog *inputDlg = new InputDialog(InputDialog::Faturamento, this);
  inputDlg->setFilter(idCompra);

  if (inputDlg->exec() != InputDialog::Accepted) return;

  QString dataReal = inputDlg->getDate().toString("yyyy-MM-dd");
  QString dataPrevista = inputDlg->getNextDate().toString("yyyy-MM-dd");

  if (query.value("representacao").toBool()) {
    if (not query.exec("UPDATE pedido_fornecedor_has_produto SET dataRealFat = '" + dataReal + "', dataPrevColeta = '" +
                       dataPrevista + "', status = 'EM COLETA' WHERE idCompra = " + idCompra)) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status da compra: " + query.lastError().text());
      close();
      return;
    }
  } else {
    ImportarXML *import = new ImportarXML(idCompra, dataReal, dataPrevista, this);
    import->showMaximized();

    if (import->exec() != QDialog::Accepted) return;
  }

  // salvar status na venda
  if (not query.exec("UPDATE venda_has_produto SET dataRealFat = '" + dataReal + "', dataPrevColeta = '" +
                     dataPrevista + "' WHERE idCompra = " + idCompra)) {
    QMessageBox::critical(this, "Erro!", "Erro salvando status da venda: " + query.lastError().text());
    close();
    return;
  }

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    close();
    return;
  }

  QString error;

  if (not updateTables(error)) {
    QMessageBox::critical(this, "Erro!", error);
    return;
  }

  QMessageBox::information(this, "Aviso!", "Confirmado faturamento.");
}

void WidgetCompraFaturar::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }
