#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>

#include "widgetestoque.h"
#include "ui_widgetestoque.h"
#include "doubledelegate.h"
#include "estoque.h"
#include "xml.h"
#include "importarxml.h"

WidgetEstoque::WidgetEstoque(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetEstoque) {
  ui->setupUi(this);

  setupTables();
}

WidgetEstoque::~WidgetEstoque() { delete ui; }

void WidgetEstoque::setupTables() {
  DoubleDelegate *doubledelegate = new DoubleDelegate(this);

  // Estoque -----------------------------------------------------------------------------------------------------------
  modelEstoque.setTable("view_estoque");
  modelEstoque.setEditStrategy(QSqlTableModel::OnManualSubmit);

  ui->tableEstoque->setModel(&modelEstoque);
  ui->tableEstoque->setItemDelegate(doubledelegate);
}

bool WidgetEstoque::updateTables() {
  if (not modelEstoque.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + modelEstoque.lastError().text());
    return false;
  }

  return true;
}

void WidgetEstoque::on_tableEstoque_activated(const QModelIndex &index) {
  Estoque *estoque = new Estoque(this);
  estoque->viewRegisterById(modelEstoque.data(index.row(), "Cód Com").toString());
}

void WidgetEstoque::on_pushButtonEntradaEstoque_clicked() {
  QString filePath = QFileDialog::getOpenFileName(this, "Importar arquivo XML", QDir::currentPath(), ("XML (*.xml)"));

  if (filePath.isEmpty()) {
    return;
  }

  QFile file(filePath);

  if (not file.open(QFile::ReadOnly)) {
    QMessageBox::critical(this, "Erro!", "Erro lendo arquivo: " + file.errorString());
    return;
  }

  XML xml(file.readAll(), file.fileName());
  xml.cadastrarNFe("ENTRADA");

  updateTables();
}

void WidgetEstoque::on_pushButtonTesteFaturamento_clicked() {
  ImportarXML *import = new ImportarXML(this);
  import->show();
}

// TODO: gerenciar lugares de estoque (cadastro/permissoes)
