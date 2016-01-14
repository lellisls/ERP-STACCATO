#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "checkboxdelegate.h"
#include "inputdialog.h"
#include "ui_inputdialog.h"

InputDialog::InputDialog(const Type &type, QWidget *parent) : QDialog(parent), type(type), ui(new Ui::InputDialog) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

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

    // NOTE: readd this later
    //    ui->tableView->showColumn(model.fieldIndex("selecionado"));

    //    for (int row = 0; row < model.rowCount(); ++row) {
    //      ui->tableView->openPersistentEditor(model.index(row, model.fieldIndex("selecionado")));
    //    }
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
    // NOTE: confirmar apenas os que estiverem marcados
  }

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados na tabela: " + model.lastError().text());
    return;
  }

  QDialog::accept();
  close();
}

void InputDialog::on_dateEditEvento_dateChanged(const QDate &date) {
  if (ui->dateEditProximo->date() < date) ui->dateEditProximo->setDate(date);
}

void InputDialog::setFilter(const QStringList &ids) {
  if (ids.isEmpty() or ids.first().isEmpty()) {
    model.setFilter("idPedido = 0");
    QMessageBox::critical(this, "Erro!", "ids vazio!");
    return;
  }

  const QString filter =
      type == ConfirmarCompra ? "idCompra = " + ids.first() : "idPedido = " + ids.join(" OR idPedido = ");

  model.setFilter(filter);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return;
  }

  ui->table->resizeColumnsToContents();

  int nWidth = 800;
  int nHeight = height();

  setGeometry(parentWidget()->x() + parentWidget()->width() / 2 - nWidth / 2,
              parentWidget()->y() + parentWidget()->height() / 2 - nHeight / 2, nWidth, nHeight);

  for (int row = 0; row < model.rowCount(); ++row) {
    ui->table->openPersistentEditor(row, "selecionado");
  }

  QMessageBox::information(this, "Aviso!", "Ajustar preço e quantidade se necessário.");
}

void InputDialog::setFilter(const QString &id) {
  if (id.isEmpty()) {
    model.setFilter("idPedido = 0");
    QMessageBox::critical(this, "Erro!", "ids vazio!");
    return;
  }

  model.setFilter("idCompra = " + id);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return;
  }

  ui->table->resizeColumnsToContents();

  int nWidth = 800;
  int nHeight = height();

  setGeometry(parentWidget()->x() + parentWidget()->width() / 2 - nWidth / 2,
              parentWidget()->y() + parentWidget()->height() / 2 - nHeight / 2, nWidth, nHeight);

  for (int row = 0; row < model.rowCount(); ++row) {
    ui->table->openPersistentEditor(row, "selecionado");
  }

  QMessageBox::information(this, "Aviso!", "Ajustar preço e quantidade se necessário.");
}

void InputDialog::setupTables() {
  model.setTable("pedido_fornecedor_has_produto");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);
  model.setHeaderData("selecionado", "");
  model.setHeaderData("fornecedor", "Fornecedor");
  model.setHeaderData("descricao", "Produto");
  model.setHeaderData("colecao", "Coleção");
  model.setHeaderData("caixas", "Caixas");
  model.setHeaderData("quant", "Quant.");
  model.setHeaderData("un", "Un.");
  model.setHeaderData("un2", "Un.2");
  model.setHeaderData("kgcx", "Kg./Cx.");
  model.setHeaderData("formComercial", "Formato");
  model.setHeaderData("codComercial", "Código");
  model.setHeaderData("preco", "Preço");
  model.setHeaderData("obs", "Obs.");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return;
  }

  ui->table->setModel(&model);
  ui->table->hideColumn("idNfe");
  ui->table->hideColumn("idEstoque");
  ui->table->hideColumn("quantUpd");
  ui->table->hideColumn("selecionado");
  ui->table->hideColumn("idPedido");
  ui->table->hideColumn("idProduto");
  ui->table->hideColumn("codBarras");
  ui->table->hideColumn("idCompra");
  ui->table->hideColumn("status");
  ui->table->hideColumn("dataPrevCompra");
  ui->table->hideColumn("dataRealCompra");
  ui->table->hideColumn("dataPrevConf");
  ui->table->hideColumn("dataRealConf");
  ui->table->hideColumn("dataPrevFat");
  ui->table->hideColumn("dataRealFat");
  ui->table->hideColumn("dataPrevColeta");
  ui->table->hideColumn("dataRealColeta");
  ui->table->hideColumn("dataPrevReceb");
  ui->table->hideColumn("dataRealReceb");
  ui->table->hideColumn("dataPrevEnt");
  ui->table->hideColumn("dataRealEnt");
  ui->table->setItemDelegateForColumn("selecionado", new CheckBoxDelegate(this));
}
