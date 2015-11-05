#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

#include "cadastrarnfe.h"
#include "entregascliente.h"
#include "ui_entregascliente.h"
#include "checkboxdelegate.h"

EntregasCliente::EntregasCliente(QWidget *parent) : QDialog(parent), ui(new Ui::EntregasCliente) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables();

  show();
}

EntregasCliente::~EntregasCliente() { delete ui; }

void EntregasCliente::setupTables() {
  modelProdutos.setTable("venda_has_produto");
  modelProdutos.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelProdutos.setHeaderData(modelProdutos.fieldIndex("selecionado"), Qt::Horizontal, "");

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela venda_has_produto: " + modelProdutos.lastError().text());
    return;
  }

  ui->tableProdutos->setModel(&modelProdutos);
  ui->tableProdutos->setItemDelegateForColumn(modelProdutos.fieldIndex("selecionado"), new CheckBoxDelegate(this));
  ui->tableProdutos->setColumnHidden(modelProdutos.fieldIndex("idVendaProduto"), true);
  ui->tableProdutos->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableProdutos->horizontalHeader()->setResizeContentsPrecision(0);

  modelEntregas.setTable("pedido_transportadora");
  modelEntregas.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelEntregas.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_transportadora: " + modelEntregas.lastError().text());
    return;
  }

  ui->tableEntregas->setModel(&modelEntregas);
  ui->tableEntregas->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableEntregas->horizontalHeader()->setResizeContentsPrecision(0);
}

void EntregasCliente::on_pushButtonNFe_clicked() {
  QList<int> lista;

  for (const auto index :
       modelProdutos.match(modelProdutos.index(0, modelProdutos.fieldIndex("selecionado")), Qt::DisplayRole, true, -1,
                           Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap))) {
    lista.append(modelProdutos.data(index.row(), "idVendaProduto").toInt());

    if (modelProdutos.data(index.row(), "idNfeSaida").toInt() != 0) {
      QMessageBox::warning(this, "Aviso!", "Produto já possui nota emitida!");
      return;
    }

    if (modelProdutos.data(index.row(), "status").toString() != "ESTOQUE") {
      QMessageBox::warning(this, "Aviso!", "Produto não está em estoque!");
      return;
    }
  }

  if (lista.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  CadastrarNFe *nfe = new CadastrarNFe(idVenda, this);
  nfe->prepararNFe(lista);
  nfe->show();
}

void EntregasCliente::on_pushButtonCancelar_clicked() { close(); }

void EntregasCliente::viewEntrega(const QString idVenda) {
  this->idVenda = idVenda;
  modelProdutos.setFilter("idVenda = '" + idVenda + "'");

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela venda_has_produto: " + modelProdutos.lastError().text());
    return;
  }

  for (int row = 0; row < modelProdutos.rowCount(); ++row) {
    ui->tableProdutos->openPersistentEditor(modelProdutos.index(row, modelProdutos.fieldIndex("selecionado")));
  }

  ui->tableProdutos->resizeColumnsToContents();
  ui->tableEntregas->resizeColumnsToContents();
}
