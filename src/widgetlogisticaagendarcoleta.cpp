#include <QDate>
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardItemModel>

#include "doubledelegate.h"
#include "inputdialog.h"
#include "ui_widgetlogisticaagendarcoleta.h"
#include "widgetlogisticaagendarcoleta.h"

WidgetLogisticaAgendarColeta::WidgetLogisticaAgendarColeta(QWidget *parent)
    : QWidget(parent), ui(new Ui::WidgetLogisticaAgendarColeta) {
  ui->setupUi(this);

  ui->frameCaminhao->hide();
}

WidgetLogisticaAgendarColeta::~WidgetLogisticaAgendarColeta() { delete ui; }

void WidgetLogisticaAgendarColeta::setupTables() {
  modelEstoque.setTable("view_agendar_coleta");
  modelEstoque.setFilter("0");

  if (not modelEstoque.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + modelEstoque.lastError().text());
    return;
  }

  modelEstoque.setHeaderData("codComercial", "Cód. Com.");
  modelEstoque.setHeaderData("fornecedor", "Fornecedor");
  modelEstoque.setHeaderData("numeroNFe", "NFe");
  modelEstoque.setHeaderData("produto", "Produto");
  modelEstoque.setHeaderData("quant", "Quant.");
  modelEstoque.setHeaderData("un", "Un.");
  modelEstoque.setHeaderData("caixas", "Cx.");
  modelEstoque.setHeaderData("kgcx", "Kg./Cx.");
  modelEstoque.setHeaderData("idVenda", "Venda");
  modelEstoque.setHeaderData("ordemCompra", "OC");
  modelEstoque.setHeaderData("local", "Local");

  ui->tableEstoque->setModel(&modelEstoque);
  ui->tableEstoque->setItemDelegate(new DoubleDelegate(this));
  //  ui->tableEstoque->hideColumn("idEstoque");
  ui->tableEstoque->hideColumn("fornecedor");

  //

  QSqlQuery query;

  if (not query.exec("SELECT modelo FROM transportadora_has_veiculo")) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados da transportadora: " + query.lastError().text());
    return;
  }

  //  while (query.next()) ui->comboBox->addItem(query.value("modelo").toString());

  connect(ui->tableEstoque->selectionModel(), &QItemSelectionModel::selectionChanged, this,
          &WidgetLogisticaAgendarColeta::calcularPeso);

  //

  modelTransp.setTable("transportadora_has_veiculo");

  if (not modelTransp.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela transportadora: " + modelTransp.lastError().text());
    return;
  }

  //  ui->tableTransp->setModel(&modelTransp);

  treeModel = new QStandardItemModel(this);
  treeModel->setColumnCount(5);

  query.exec("SELECT DISTINCT(razaoSocial) FROM transportadora_has_veiculo tv LEFT JOIN transportadora t ON "
             "t.idTransportadora = tv.idTransportadora");

  while (query.next()) {
    QString razaoSocial = query.value("razaoSocial").toString();

    QSqlQuery query2;
    query2.prepare("SELECT * FROM transportadora_has_veiculo tv LEFT JOIN transportadora t ON t.idTransportadora = "
                   "tv.idTransportadora WHERE razaoSocial = :razaoSocial");
    query2.bindValue(":razaoSocial", razaoSocial);

    if (not query2.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando dados da transportadora: " + query2.lastError().text());
      return;
    }

    QStandardItem *parent = new QStandardItem(razaoSocial);
    treeModel->appendRow(parent);

    while (query2.next()) {
      QString modelo = query2.value("modelo").toString();
      QString carga = query2.value("carga").toString() + " Kg";
      QStandardItem *child = new QStandardItem(modelo + " - " + carga);
      parent->setChild(parent->rowCount(), child);
    }
  }

  //  for (int row = 0; row < modelTransp.rowCount(); ++row) {
  //    QString modelo = modelTransp.data(row, "modelo").toString();
  //    QString carga = modelTransp.data(row, "carga").toString() + " Kg";
  //    QStandardItem *parent = new QStandardItem(modelo + " - " + carga);
  //    treeModel->appendRow(parent);
  //  }

  ui->tableTransp->setModel(treeModel);
  //  ui->tableTransp->setItemDelegate(new DoubleDelegate(this));
}

void WidgetLogisticaAgendarColeta::calcularPeso() {
  double peso = 0;

  const auto list = ui->tableEstoque->selectionModel()->selectedRows();

  for (auto const &item : list) {
    const double kg = modelEstoque.data(item.row(), "kgcx").toDouble();
    const double caixas = modelEstoque.data(item.row(), "caixas").toDouble();
    peso += kg * caixas;
  }

  ui->doubleSpinBox->setValue(peso);
}

bool WidgetLogisticaAgendarColeta::updateTables() {
  if (modelEstoque.tableName().isEmpty()) setupTables();

  const QString filter = modelEstoque.filter();

  if (not modelEstoque.select()) {
    emit errorSignal("Erro lendo tabela estoque: " + modelEstoque.lastError().text());
    return false;
  }

  modelEstoque.setFilter(filter);

  //  ui->tableEstoque->resizeColumnsToContents();

  return true;
}

void WidgetLogisticaAgendarColeta::on_pushButtonSendLeft_clicked() {
  // TODO: fix
  auto list = ui->tableTransp->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  for (auto const &item : list) {
    if (item.parent().row() == -1) {
      QMessageBox::critical(this, "Erro!", "Não pode remover veículo!");
      return;
    }

    treeModel->item(item.parent().row())->removeRow(item.row());
  }
}

void WidgetLogisticaAgendarColeta::on_pushButtonSendRight_clicked() {
  auto list = ui->tableEstoque->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  qDebug() << "rows: " << ui->tableTransp->selectionModel()->selectedRows();

  const auto list2 = ui->tableTransp->selectionModel()->selectedRows();

  if (list2.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum veículo selecionado!");
    return;
  }

  if (list2.first().parent().row() == -1) {
    QMessageBox::critical(this, "Erro!", "Selecione um veículo!");
    return;
  }

  qDebug() << "list2.first: " << list2.first();

  auto selectedTransp = treeModel->itemFromIndex(list2.first());
  //  auto selectedTransp = treeModel->item(list2.first().row());

  qDebug() << "selectedT: " << selectedTransp;

  for (auto const &item : list) {
    const QString produto = modelEstoque.data(item.row(), "produto").toString();
    const double kg = modelEstoque.data(item.row(), "kgcx").toDouble();
    const double quant = modelEstoque.data(item.row(), "quant").toDouble();
    const QString peso = QString::number(kg * quant) + " Kg";

    selectedTransp->insertColumns(item.row(), 2);

    QStandardItem *child = new QStandardItem(produto + " - " + peso);
    QStandardItem *pesoItem = new QStandardItem(peso);
    QList<QStandardItem *> listItems;
    listItems << pesoItem;
    //    child->appendColumn(listItems);
    //    treeModel->setItem(0, 0, child);
    //    treeModel->setItem(0, 1, pesoItem);
    selectedTransp->setColumnCount(2);
    int row = selectedTransp->rowCount();
    selectedTransp->setChild(row, 0, child);
    selectedTransp->setChild(row, 1, pesoItem);
    //    selectedTransp->insertColumn(item.row(), listItems);
    //    selectedTransp->insertColumn(item.row(), listItems);
  }

  ui->tableTransp->expand(list2.first());

  ui->tableEstoque->clearSelection();
}

void WidgetLogisticaAgendarColeta::TableFornLogistica_activated(const QString &fornecedor) {
  modelEstoque.setFilter("fornecedor = '" + fornecedor + "'");

  if (not modelEstoque.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + modelEstoque.lastError().text());
    return;
  }

  ui->tableEstoque->resizeColumnsToContents();
}

// QTreeView::item { border: 0.5px ; border-style: solid ; border-color: lightgray ;} table stylesheet

void WidgetLogisticaAgendarColeta::on_pushButtonMontarCarga_clicked() {
  // TODO: data prevista +8 dias
  ui->frameCaminhao->show();

  ui->tableTransp->expandAll();
  ui->tableTransp->resizeColumnToContents(0);

  return;

  if (not ui->frameCaminhao->isVisible()) return;

  ui->frameCaminhao->setVisible(true);

  const auto list = ui->tableEstoque->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }
}

void WidgetLogisticaAgendarColeta::on_pushButtonAgendarColeta_clicked() {
  const auto list = ui->tableEstoque->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not processRows(list)) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  QMessageBox::information(this, "Aviso!", "Marcado com sucesso!");
  updateTables();
}

bool WidgetLogisticaAgendarColeta::processRows(const QModelIndexList &list) {
  InputDialog *input = new InputDialog(InputDialog::AgendarColeta, this);

  if (input->exec() != InputDialog::Accepted) return false;

  const QDate dataPrevColeta = input->getNextDate();

  for (auto const &item : list) {
    QSqlQuery query;
    query.prepare("UPDATE estoque SET status = 'EM COLETA' WHERE idEstoque = :idEstoque");
    query.bindValue(":idEstoque", modelEstoque.data(item.row(), "idEstoque"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando status no estoque: " + query.lastError().text());
      return false;
    }

    query.prepare("UPDATE pedido_fornecedor_has_produto SET dataPrevColeta = :dataPrevColeta WHERE idCompra IN (SELECT "
                  "idCompra FROM estoque_has_compra WHERE idEstoque = :idEstoque) AND codComercial = :codComercial");
    query.bindValue(":dataPrevColeta", dataPrevColeta);
    query.bindValue(":idEstoque", modelEstoque.data(item.row(), "idEstoque"));
    query.bindValue(":codComercial", modelEstoque.data(item.row(), "codComercial"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando status no pedido_fornecedor: " + query.lastError().text());
      return false;
    }

    query.prepare(
        "UPDATE venda_has_produto SET dataPrevColeta = :dataPrevColeta WHERE idCompra IN (SELECT idCompra FROM "
        "estoque_has_compra WHERE idEstoque = :idEstoque) AND codComercial = :codComercial");
    query.bindValue(":dataPrevColeta", dataPrevColeta);
    query.bindValue(":idEstoque", modelEstoque.data(item.row(), "idEstoque"));
    query.bindValue(":codComercial", modelEstoque.data(item.row(), "codComercial"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando status na venda_produto: " + query.lastError().text());
      return false;
    }
  }

  return true;
}

void WidgetLogisticaAgendarColeta::on_tableEstoque_entered(const QModelIndex &) {
  ui->tableEstoque->resizeColumnsToContents();
}
