#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "checkboxdelegate.h"
#include "inputdialog.h"
#include "ui_inputdialog.h"

InputDialog::InputDialog(Type type, QWidget *parent) : QDialog(parent), type(type), ui(new Ui::InputDialog) {
  ui->setupUi(this);

  setupTables();

  ui->groupBoxData->hide();
  ui->groupBoxDataPreco->hide();

  ui->dateEditEvento->setDate(QDate::currentDate());
  ui->dateEditProximo->setDate(QDate::currentDate());

  if (type == Carrinho) {
    ui->groupBoxData->show();
    ui->labelEvento->hide();
    ui->dateEditEvento->hide();

    ui->labelProximoEvento->setText("Data prevista compra:");
  }

  if (type == GerarCompra) {
    ui->groupBoxData->show();
    ui->groupBoxDataPreco->show();

    ui->labelEvento->setText("Data compra:");
    ui->labelProximoEvento->setText("Data prevista confirmação:");
  }

  if (type == ConfirmarCompra) {
    ui->groupBoxData->show();
    ui->groupBoxDataPreco->show();

    ui->labelEvento->setText("Data confirmação:");
    ui->labelProximoEvento->setText("Data prevista faturamento:");

    ui->tableView->showColumn(model.fieldIndex("selecionado"));

    for (int i = 0; i < model.rowCount(); ++i) {
      ui->tableView->openPersistentEditor(model.index(i, model.fieldIndex("selecionado")));
    }
  }

  if (type == Faturamento) {
    ui->groupBoxData->show();

    ui->labelEvento->setText("Data faturamento:");
    ui->labelProximoEvento->setText("Data prevista coleta:");
  }

  if (type == Coleta) {
    ui->groupBoxData->show();

    ui->labelEvento->setText("Data coleta:");
    ui->labelProximoEvento->setText("Data prevista recebimento:");
  }

  if (type == Recebimento) {
    ui->groupBoxData->show();

    ui->labelEvento->setText("Data do recebimento:");
    ui->labelProximoEvento->setText("Data prevista entrega:");
  }

  if (type == Entrega) {
    ui->groupBoxData->show();
    ui->labelProximoEvento->hide();
    ui->dateEditProximo->hide();

    ui->labelEvento->setText("Data entrega:");
  }

  adjustSize();
  show();
}

InputDialog::~InputDialog() { delete ui; }

QDate InputDialog::getDate() { return ui->dateEditEvento->date(); }

QDate InputDialog::getNextDate() { return ui->dateEditProximo->date(); }

void InputDialog::on_pushButtonSalvar_clicked() {
  if (type == ConfirmarCompra) {
    // TODO: confirmar apenas os que estiverem marcados
  }

  if (not model.submitAll()) {
    qDebug() << "Erro salvando dados no model: " << model.lastError();
    return;
  }

  QDialog::accept();
  close();
}

void InputDialog::on_pushButtonCancelar_clicked() {
  QDialog::reject();
  close();
}

void InputDialog::on_dateEditEvento_dateChanged(const QDate &date) {
  if (ui->dateEditProximo->date() < date) {
    ui->dateEditProximo->setDate(date);
  }
}

void InputDialog::setFilter(QStringList ids) {
  if (ids.isEmpty() or ids.first().isEmpty()) {
    model.setFilter("idPedido = 0");
    QMessageBox::warning(this, "Aviso!", "ids vazio!");
    return;
  }

  QString filter;

  if (type == ConfirmarCompra) {
    filter = "idCompra = " + ids.first();
  } else {
    filter = "idPedido = " + ids.join(" OR idPedido = ");
  }

  model.setFilter(filter);

  if (not model.select()) {
    qDebug() << "Erro model: " << model.lastError();
    qDebug() << "query: " << model.query().executedQuery();
  }

  ui->tableView->resizeColumnsToContents();

  int nWidth = 800;
  int nHeight = height();

  setGeometry(parentWidget()->x() + parentWidget()->width() / 2 - nWidth / 2,
              parentWidget()->y() + parentWidget()->height() / 2 - nHeight / 2, nWidth, nHeight);

  for (int row = 0; row < model.rowCount(); ++row) {
    ui->tableView->openPersistentEditor(model.index(row, model.fieldIndex("selecionado")));
  }

  QMessageBox::information(this, "Aviso!", "Ajustar preço e quantidade se necessário.");
}

void InputDialog::setFilter(QString ids) {
  if (ids.isEmpty()) {
    model.setFilter("idPedido = 0");
    QMessageBox::warning(this, "Aviso!", "ids vazio!");
    return;
  }

  model.setFilter("idCompra = " + ids);

  if (not model.select()) {
    qDebug() << "Erro model: " << model.lastError();
    qDebug() << "query: " << model.query().executedQuery();
  }

  ui->tableView->resizeColumnsToContents();

  int nWidth = 800;
  int nHeight = height();

  setGeometry(parentWidget()->x() + parentWidget()->width() / 2 - nWidth / 2,
              parentWidget()->y() + parentWidget()->height() / 2 - nHeight / 2, nWidth, nHeight);

  for (int row = 0; row < model.rowCount(); ++row) {
    ui->tableView->openPersistentEditor(model.index(row, model.fieldIndex("selecionado")));
  }

  QMessageBox::information(this, "Aviso!", "Ajustar preço e quantidade se necessário.");
}

void InputDialog::setupTables() {
  model.setTable("pedido_fornecedor_has_produto");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);
  model.setHeaderData(model.fieldIndex("selecionado"), Qt::Horizontal, "");
  model.setHeaderData(model.fieldIndex("fornecedor"), Qt::Horizontal, "Fornecedor");
  model.setHeaderData(model.fieldIndex("descricao"), Qt::Horizontal, "Produto");
  model.setHeaderData(model.fieldIndex("colecao"), Qt::Horizontal, "Coleção");
  model.setHeaderData(model.fieldIndex("quant"), Qt::Horizontal, "Quant.");
  model.setHeaderData(model.fieldIndex("un"), Qt::Horizontal, "Un.");
  model.setHeaderData(model.fieldIndex("formComercial"), Qt::Horizontal, "Formato");
  model.setHeaderData(model.fieldIndex("codComercial"), Qt::Horizontal, "Código");
  model.setHeaderData(model.fieldIndex("preco"), Qt::Horizontal, "Preço");
  model.setHeaderData(model.fieldIndex("obs"), Qt::Horizontal, "Obs.");

  if (not model.select()) {
    qDebug() << "Erro model: " << model.lastError();
    return;
  }

  ui->tableView->setModel(&model);
  ui->tableView->hideColumn(model.fieldIndex("quantUpd"));
  ui->tableView->hideColumn(model.fieldIndex("selecionado"));
  ui->tableView->hideColumn(model.fieldIndex("idPedido"));
  ui->tableView->hideColumn(model.fieldIndex("idProduto"));
  ui->tableView->hideColumn(model.fieldIndex("codBarras"));
  ui->tableView->hideColumn(model.fieldIndex("idCompra"));
  ui->tableView->hideColumn(model.fieldIndex("status"));
  ui->tableView->hideColumn(model.fieldIndex("dataPrevCompra"));
  ui->tableView->hideColumn(model.fieldIndex("dataRealCompra"));
  ui->tableView->hideColumn(model.fieldIndex("dataPrevConf"));
  ui->tableView->hideColumn(model.fieldIndex("dataRealConf"));
  ui->tableView->hideColumn(model.fieldIndex("dataPrevFat"));
  ui->tableView->hideColumn(model.fieldIndex("dataRealFat"));
  ui->tableView->hideColumn(model.fieldIndex("dataPrevColeta"));
  ui->tableView->hideColumn(model.fieldIndex("dataRealColeta"));
  ui->tableView->hideColumn(model.fieldIndex("dataPrevReceb"));
  ui->tableView->hideColumn(model.fieldIndex("dataRealReceb"));
  ui->tableView->hideColumn(model.fieldIndex("dataPrevEnt"));
  ui->tableView->hideColumn(model.fieldIndex("dataRealEnt"));
  ui->tableView->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableView->horizontalHeader()->setResizeContentsPrecision(0);

  ui->tableView->setItemDelegateForColumn(model.fieldIndex("selecionado"), new CheckBoxDelegate(this));
}
