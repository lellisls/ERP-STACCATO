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

QString WidgetCompraFaturar::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) return "Erro lendo tabela faturamento: " + model.lastError().text();

  ui->table->resizeColumnsToContents();

  return QString();
}

void WidgetCompraFaturar::on_pushButtonMarcarFaturado_clicked() {
  if (ui->table->selectionModel()->selectedRows().size() == 0) {
    QMessageBox::critical(this, "Erro!", "Não selecionou nenhuma compra!");
    return;
  }

  auto list = ui->table->selectionModel()->selectedRows();

  QString idCompra;

  for (auto item : list) {
    QString id = model.data(item.row(), "idCompra").toString();
    idCompra += idCompra.isEmpty() ? id : " OR idCompra = " + id;
  }

  //-----------------------
  InputDialog *inputDlg = new InputDialog(InputDialog::Faturamento, this);

  if (inputDlg->exec() != InputDialog::Accepted) return;

  const QString dataFat = inputDlg->getDate().toString("yyyy-MM-dd");
  const QString dataPrevista = inputDlg->getNextDate().toString("yyyy-MM-dd");
  //-----------------------

  ImportarXML *import = new ImportarXML(idCompra, this);
  import->setData(dataFat, dataPrevista);
  import->showMaximized();

  if (import->exec() != QDialog::Accepted) return;

  //----------------------------------------------------------//

  updateTables();

  QMessageBox::information(this, "Aviso!", "Confirmado faturamento.");
}

void WidgetCompraFaturar::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }
