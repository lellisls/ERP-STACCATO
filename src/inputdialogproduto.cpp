#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "inputdialogproduto.h"
#include "noeditdelegate.h"
#include "porcentagemdelegate.h"
#include "reaisdelegate.h"
#include "singleeditdelegate.h"
#include "ui_inputdialogproduto.h"

InputDialogProduto::InputDialogProduto(const Type &type, QWidget *parent) : QDialog(parent), type(type), ui(new Ui::InputDialogProduto) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables();

  ui->labelAliquota->hide();
  ui->doubleSpinBoxAliquota->hide();
  ui->labelST->hide();
  ui->doubleSpinBoxST->hide();

  ui->dateEditEvento->setDate(QDate::currentDate());
  ui->dateEditProximo->setDate(QDate::currentDate());

  if (type == GerarCompra) {
    ui->labelEvento->setText("Data compra:");
    ui->labelProximoEvento->setText("Data prevista confirmação:");

    connect(&model, &SqlTableModel::dataChanged, this, &InputDialogProduto::updateTableData);
  }

  if (type == Faturamento) {
    ui->labelProximoEvento->hide();
    ui->dateEditProximo->hide();
    ui->comboBoxST->hide();
    ui->labelAliquota->hide();
    ui->doubleSpinBoxAliquota->hide();
    ui->labelST->hide();
    ui->doubleSpinBoxST->hide();

    ui->labelEvento->setText("Data faturamento:");
  }

  adjustSize();

  showMaximized();
}

InputDialogProduto::~InputDialogProduto() { delete ui; }

void InputDialogProduto::setupTables() {
  model.setTable("pedido_fornecedor_has_produto");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);
  model.setHeaderData("idVenda", "Código");
  model.setHeaderData("fornecedor", "Fornecedor");
  model.setHeaderData("descricao", "Produto");
  model.setHeaderData("colecao", "Coleção");
  model.setHeaderData("caixas", "Caixas");
  model.setHeaderData("prcUnitario", "$ Unit.");
  model.setHeaderData("quant", "Quant.");
  model.setHeaderData("preco", "Total");
  model.setHeaderData("un", "Un.");
  model.setHeaderData("un2", "Un.2");
  model.setHeaderData("kgcx", "Kg./Cx.");
  model.setHeaderData("formComercial", "Formato");
  model.setHeaderData("codComercial", "Cód. Com.");
  model.setHeaderData("obs", "Obs.");
  model.setHeaderData("aliquotaSt", "Alíquota ST");
  model.setHeaderData("st", "ST");

  ui->table->setModel(&model);
  ui->table->hideColumn("idVendaProduto");
  ui->table->hideColumn("statusFinanceiro");
  ui->table->hideColumn("ordemCompra");
  ui->table->hideColumn("quantConsumida");
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
  ui->table->setItemDelegate(new NoEditDelegate(this));
  ui->table->setItemDelegateForColumn("prcUnitario", new ReaisDelegate(this));
  ui->table->setItemDelegateForColumn("preco", new ReaisDelegate(this));
  ui->table->setItemDelegateForColumn("aliquotaSt", new PorcentagemDelegate(this));

  if (type == GerarCompra) {
    ui->table->setItemDelegateForColumn("quant", new SingleEditDelegate(this));
  }
}

bool InputDialogProduto::setFilter(const QStringList &ids) {
  if (ids.isEmpty()) {
    model.setFilter("0");
    QMessageBox::critical(this, "Erro!", "Ids vazio!");
    return false;
  }

  QString filter;

  if (type == GerarCompra) filter = "(idPedido = " + ids.join(" OR idPedido = ") + ") AND status = 'PENDENTE'";
  if (type == Faturamento) filter = "(idCompra = " + ids.join(" OR idCompra = ") + ") AND status = 'EM FATURAMENTO'";

  if (filter.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Filtro vazio!");
    return false;
  }

  model.setFilter(filter);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return false;
  }

  ui->table->resizeColumnsToContents();

  calcularTotal();

  QSqlQuery query;
  query.prepare("SELECT aliquotaSt, st FROM fornecedor WHERE razaoSocial = :razaoSocial");
  query.bindValue(":razaoSocial", model.data(0, "fornecedor"));

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando substituicao tributaria do fornecedor: " + query.lastError().text());
    return false;
  }

  ui->comboBoxST->setCurrentText(query.value("st").toString());
  ui->doubleSpinBoxAliquota->setValue(query.value("aliquotaSt").toDouble());

  if (type == GerarCompra) QMessageBox::information(this, "Aviso!", "Ajustar preço e quantidade se necessário.");

  return true;
}

QDateTime InputDialogProduto::getDate() const { return ui->dateEditEvento->dateTime(); }

QDateTime InputDialogProduto::getNextDate() const { return ui->dateEditProximo->dateTime(); }

void InputDialogProduto::updateTableData(const QModelIndex &topLeft) {
  const QString header = model.headerData(topLeft.column(), Qt::Horizontal).toString();
  const int row = topLeft.row();

  if (header == "Quant." or header == "$ Unit.") {
    const double preco = model.data(row, "quant").toDouble() * model.data(row, "prcUnitario").toDouble();
    model.setData(row, "preco", preco);
  }

  if (header == "Total") {
    const double preco = model.data(row, "preco").toDouble() / model.data(row, "quant").toDouble();
    model.setData(row, "prcUnitario", preco);
  }

  processST();
  //  calcularTotal();
}

void InputDialogProduto::calcularTotal() {
  double total = 0;

  for (int row = 0; row < model.rowCount(); ++row) total += model.data(row, "preco").toDouble();

  ui->doubleSpinBoxTotal->setValue(total);
}

void InputDialogProduto::on_pushButtonSalvar_clicked() {
  if (not cadastrar()) return;

  QDialog::accept();
  close();
}

bool InputDialogProduto::cadastrar() {
  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados na tabela: " + model.lastError().text());
    return false;
  }

  return true;
}

void InputDialogProduto::on_dateEditEvento_dateChanged(const QDate &date) {
  if (ui->dateEditProximo->date() < date) ui->dateEditProximo->setDate(date);
}

void InputDialogProduto::on_doubleSpinBoxAliquota_valueChanged(double) { processST(); }

void InputDialogProduto::on_doubleSpinBoxST_valueChanged(double value) {
  // calcular a aliquota e jogar no spinbox para recalcular os totais
  isBlockedAliquota = true;

  double total = 0;

  for (int row = 0; row < model.rowCount(); ++row) total += model.data(row, "preco").toDouble();

  const double aliquota = value / total * 100;

  ui->doubleSpinBoxAliquota->setValue(aliquota);

  isBlockedAliquota = false;
}

void InputDialogProduto::processST() {
  calcularTotal();

  const QString text = ui->comboBoxST->currentText();

  const double aliquota = ui->doubleSpinBoxAliquota->value();
  double total = ui->doubleSpinBoxTotal->value();

  if (text == "Sem ST") {
    ui->doubleSpinBoxST->setValue(0);

    ui->labelAliquota->hide();
    ui->doubleSpinBoxAliquota->hide();
    ui->labelST->hide();
    ui->doubleSpinBoxST->hide();

    for (int row = 0; row < model.rowCount(); ++row) {
      model.setData(row, "aliquotaSt", aliquota);
      model.setData(row, "st", text);
    }
  }

  if (text == "ST Fornecedor") {
    ui->labelAliquota->show();
    ui->doubleSpinBoxAliquota->show();
    ui->labelST->show();
    ui->doubleSpinBoxST->show();

    const double st = total * aliquota / 100;

    if (not isBlockedAliquota) ui->doubleSpinBoxST->setValue(st);

    for (int row = 0; row < model.rowCount(); ++row) {
      model.setData(row, "aliquotaSt", aliquota);
      model.setData(row, "st", text);
    }

    total += total * aliquota / 100;
    ui->doubleSpinBoxTotal->setValue(total);
  }

  if (text == "ST Loja") {
    ui->labelAliquota->show();
    ui->doubleSpinBoxAliquota->show();
    ui->labelST->show();
    ui->doubleSpinBoxST->show();

    const double st = total * aliquota / 100;

    if (not isBlockedAliquota) ui->doubleSpinBoxST->setValue(st);

    for (int row = 0; row < model.rowCount(); ++row) {
      model.setData(row, "aliquotaSt", aliquota);
      model.setData(row, "st", text);
    }

    total += total * aliquota / 100;
    ui->doubleSpinBoxTotal->setValue(total);
  }

  ui->table->resizeColumnsToContents();
}

void InputDialogProduto::on_comboBoxST_currentTextChanged(const QString &) { processST(); }

void InputDialogProduto::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }
