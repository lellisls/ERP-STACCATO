#include "widgetestoque.h"
#include "ui_widgetestoque.h"
#include "doubledelegate.h"
#include "estoque.h"
#include "xml.h"
#include "importarxml.h"

#include <QMessageBox>
#include <QSqlError>

WidgetEstoque::WidgetEstoque(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetEstoque) {
  ui->setupUi(this);

  setupTables();
}

WidgetEstoque::~WidgetEstoque() { delete ui; }

void WidgetEstoque::setupTables() {
  DoubleDelegate *doubledelegate = new DoubleDelegate(this);

  // Estoque -----------------------------------------------------------------------------------------------------------
  modelEstoque = new SqlTableModel(this);
  modelEstoque->setTable("view_estoque");
  modelEstoque->setEditStrategy(QSqlTableModel::OnManualSubmit);

  ui->tableEstoque->setModel(modelEstoque);
  ui->tableEstoque->setItemDelegate(doubledelegate);
}

bool WidgetEstoque::updateTables() {
  if (not modelEstoque->select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + modelEstoque->lastError().text());
    return false;
  }

  return true;
}

void WidgetEstoque::on_tableEstoque_activated(const QModelIndex &index) {
  Estoque *estoque = new Estoque(this);
  estoque->viewRegisterById(modelEstoque->data(index.row(), "Cód Com").toString());
}

void WidgetEstoque::on_pushButtonEntradaEstoque_clicked() {
  XML xml;
  xml.importarXML();
  updateTables();
}

void WidgetEstoque::on_pushButtonTesteFaturamento_clicked() {
  QList<int> temp;
  ImportarXML *import = new ImportarXML(temp, this);
  import->show();
}
