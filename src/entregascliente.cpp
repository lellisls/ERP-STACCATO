#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "cadastrarnfe.h"
#include "checkboxdelegate.h"
#include "entregascliente.h"
#include "ui_entregascliente.h"

EntregasCliente::EntregasCliente(QWidget *parent) : QDialog(parent), ui(new Ui::EntregasCliente) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);
  setAttribute(Qt::WA_DeleteOnClose);

  setupTables();

  show();
}

EntregasCliente::~EntregasCliente() { delete ui; }

void EntregasCliente::setupTables() {
  modelProdutos.setTable("venda_has_produto");
  modelProdutos.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelProdutos.setHeaderData("selecionado", "");

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela venda_has_produto: " + modelProdutos.lastError().text());
  }

  ui->tableProdutos->setModel(&modelProdutos);
  ui->tableProdutos->setItemDelegateForColumn("selecionado", new CheckBoxDelegate(this));
  ui->tableProdutos->hideColumn("idVendaProduto");

  modelEntregas.setTable("pedido_transportadora");
  modelEntregas.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelEntregas.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_transportadora: " + modelEntregas.lastError().text());
  }

  ui->tableEntregas->setModel(&modelEntregas);
}

void EntregasCliente::on_pushButtonNFe_clicked() {
  QList<int> lista;

  for (const auto index :
       modelProdutos.match(modelProdutos.index(0, modelProdutos.fieldIndex("selecionado")), Qt::DisplayRole, true, -1,
                           Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap))) {
    // NOTE: readd these
    //    if (modelProdutos.data(index.row(), "idNfeSaida").toInt() != 0) {
    //      QMessageBox::critical(this, "Erro!", "Produto já possui nota emitida!");
    //      return;
    //    }

    //    if (modelProdutos.data(index.row(), "status").toString() != "ESTOQUE") {
    //      QMessageBox::critical(this, "Erro!", "Produto não está em estoque!");
    //      return;
    //    }

    lista.append(modelProdutos.data(index.row(), "idVendaProduto").toInt());
  }

  if (lista.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  CadastrarNFe *nfe = new CadastrarNFe(idVenda, this);
  nfe->prepararNFe(lista);
  nfe->show();
}

void EntregasCliente::viewEntrega(const QString &idVenda) {
  this->idVenda = idVenda;

  modelProdutos.setFilter("idVenda = '" + idVenda + "'");

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela venda_has_produto: " + modelProdutos.lastError().text());
    return;
  }

  for (int row = 0; row < modelProdutos.rowCount(); ++row) {
    ui->tableProdutos->openPersistentEditor(row, "selecionado");
  }

  ui->tableProdutos->resizeColumnsToContents();
  ui->tableEntregas->resizeColumnsToContents();
}
