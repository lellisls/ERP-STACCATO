#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

#include "src/estoque.h"
#include "ui_estoque.h"
#include "xml_viewer.h"

Estoque::Estoque(QWidget *parent) : QDialog(parent), ui(new Ui::Estoque) {
  ui->setupUi(this);

  modelEstoque.setTable("produto_has_estoque");
  modelEstoque.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelEstoque.select()) {
    qDebug() << "Erro carregando estoque: " << modelEstoque.lastError();
  }

  ui->tableEstoque->setModel(&modelEstoque);
}

Estoque::~Estoque() { delete ui; }

void Estoque::on_tableEstoque_activated(const QModelIndex &index) {
  //  qDebug() << "xml: " << modelEstoque.data(modelEstoque.index(index.row(),
  //  modelEstoque.fieldIndex("xml"))).toString();
  XML_Viewer *xml = new XML_Viewer(this);
  xml->exibirXML(modelEstoque.data(modelEstoque.index(index.row(), modelEstoque.fieldIndex("xml"))).toString());
  xml->show();
}

void Estoque::viewRegisterById(const QVariant id) {
  //  qDebug() << "id: " << id;

  if (not modelEstoque.select()) {
    qDebug() << "erro model: " << modelEstoque.lastError();
  }

  const QModelIndex index =
      modelEstoque.match(modelEstoque.index(0, modelEstoque.fieldIndex("idProduto")), Qt::DisplayRole, id).first();

  if (not index.isValid()) {
    QMessageBox::warning(this, "Atenção!", "Item não encontrado.", QMessageBox::Ok, QMessageBox::NoButton);
    close();
  }

  QString idProduto =
      modelEstoque.data(modelEstoque.index(index.row(), modelEstoque.fieldIndex("idProduto"))).toString();
  //  qDebug() << "idEstoque: " << idProduto;

  modelEstoque.setFilter("idProduto = '" + idProduto + "'");

  if (not modelEstoque.select()) {
    qDebug() << "erro modelEstoque: " << modelEstoque.lastError();
  }

  for (int column = 0; column < modelEstoque.columnCount(); ++column) {
    if (modelEstoque.fieldIndex("xml") == column) {
      continue;
    }

    ui->tableEstoque->resizeColumnToContents(column);
  }

  show();
}
