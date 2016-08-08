#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "cadastrarnfe.h"
#include "checkboxdelegate.h"
#include "entregascliente.h"
#include "reaisdelegate.h"
#include "ui_entregascliente.h"
#include "usersession.h"

EntregasCliente::EntregasCliente(QWidget *parent) : QDialog(parent), ui(new Ui::EntregasCliente) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);
  setAttribute(Qt::WA_DeleteOnClose);

  setupTables();

  showMaximized();
}

EntregasCliente::~EntregasCliente() { delete ui; }

void EntregasCliente::setupTables() {
  modelProdutos.setTable("venda_has_produto");
  modelProdutos.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelProdutos.setHeaderData("selecionado", "");
  modelProdutos.setHeaderData("status", "Status");
  modelProdutos.setHeaderData("idNfeSaida", "NFe Saída");
  modelProdutos.setHeaderData("fornecedor", "Forn.");
  modelProdutos.setHeaderData("produto", "Produto");
  modelProdutos.setHeaderData("obs", "Obs.");
  modelProdutos.setHeaderData("prcUnitario", "R$ Unit.");
  modelProdutos.setHeaderData("caixas", "Cx.");
  modelProdutos.setHeaderData("quant", "Quant.");
  modelProdutos.setHeaderData("un", "Un.");
  modelProdutos.setHeaderData("unCaixa", "Un./Cx.");
  modelProdutos.setHeaderData("codComercial", "Cód. Com.");
  modelProdutos.setHeaderData("formComercial", "Form. Com.");
  modelProdutos.setHeaderData("total", "Total");
  modelProdutos.setHeaderData("dataPrevEnt", "Prev. Ent.");
  modelProdutos.setHeaderData("dataRealEnt", "Entrega");

  modelProdutos.setFilter("idNfeSaida IS NULL");

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela venda_has_produto: " + modelProdutos.lastError().text());
  }

  ui->tableProdutos->setModel(&modelProdutos);
  ui->tableProdutos->setItemDelegateForColumn("selecionado", new CheckBoxDelegate(this));
  ui->tableProdutos->setItemDelegateForColumn("total", new ReaisDelegate(this));
  ui->tableProdutos->hideColumn("obs");
  ui->tableProdutos->hideColumn("idVendaProduto");
  ui->tableProdutos->hideColumn("idVenda");
  ui->tableProdutos->hideColumn("idLoja");
  ui->tableProdutos->hideColumn("item");
  ui->tableProdutos->hideColumn("idProduto");
  ui->tableProdutos->hideColumn("parcial");
  ui->tableProdutos->hideColumn("desconto");
  ui->tableProdutos->hideColumn("parcialDesc");
  ui->tableProdutos->hideColumn("descGlobal");
  ui->tableProdutos->hideColumn("idCompra");
  ui->tableProdutos->hideColumn("dataPrevCompra");
  ui->tableProdutos->hideColumn("dataRealCompra");
  ui->tableProdutos->hideColumn("dataPrevConf");
  ui->tableProdutos->hideColumn("dataRealConf");
  ui->tableProdutos->hideColumn("dataPrevFat");
  ui->tableProdutos->hideColumn("dataRealFat");
  ui->tableProdutos->hideColumn("dataPrevColeta");
  ui->tableProdutos->hideColumn("dataRealColeta");
  ui->tableProdutos->hideColumn("dataPrevReceb");
  ui->tableProdutos->hideColumn("dataRealReceb");
  ui->tableProdutos->hideColumn("descUnitario");
  ui->tableProdutos->hideColumn("estoque_promocao");

  modelEntregas.setTable("venda_has_produto");
  modelEntregas.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelEntregas.setHeaderData("status", "Status");
  modelEntregas.setHeaderData("idNfeSaida", "NFe Saída");
  modelEntregas.setHeaderData("fornecedor", "Forn.");
  modelEntregas.setHeaderData("produto", "Produto");
  modelEntregas.setHeaderData("obs", "Obs.");
  modelEntregas.setHeaderData("prcUnitario", "R$ Unit.");
  modelEntregas.setHeaderData("caixas", "Cx.");
  modelEntregas.setHeaderData("quant", "Quant.");
  modelEntregas.setHeaderData("un", "Un.");
  modelEntregas.setHeaderData("unCaixa", "Un./Cx.");
  modelEntregas.setHeaderData("codComercial", "Cód. Com.");
  modelEntregas.setHeaderData("formComercial", "Form. Cod.");
  modelEntregas.setHeaderData("total", "Total");
  modelEntregas.setHeaderData("dataPrevEnt", "Prev. Ent.");
  modelEntregas.setHeaderData("dataRealEnt", "Entrega");

  modelEntregas.setFilter("idNfeSaida IS NOT NULL");

  if (not modelEntregas.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_transportadora: " + modelEntregas.lastError().text());
  }

  ui->tableEntregas->setModel(&modelEntregas);
  ui->tableEntregas->setItemDelegateForColumn("total", new ReaisDelegate(this));
  ui->tableEntregas->hideColumn("selecionado");
  ui->tableEntregas->hideColumn("idVendaProduto");
  ui->tableEntregas->hideColumn("idVenda");
  ui->tableEntregas->hideColumn("idLoja");
  ui->tableEntregas->hideColumn("item");
  ui->tableEntregas->hideColumn("idProduto");
  ui->tableEntregas->hideColumn("parcial");
  ui->tableEntregas->hideColumn("desconto");
  ui->tableEntregas->hideColumn("parcialDesc");
  ui->tableEntregas->hideColumn("descGlobal");
  ui->tableEntregas->hideColumn("idCompra");
  ui->tableEntregas->hideColumn("dataPrevCompra");
  ui->tableEntregas->hideColumn("dataRealCompra");
  ui->tableEntregas->hideColumn("dataPrevConf");
  ui->tableEntregas->hideColumn("dataRealConf");
  ui->tableEntregas->hideColumn("dataPrevFat");
  ui->tableEntregas->hideColumn("dataRealFat");
  ui->tableEntregas->hideColumn("dataPrevColeta");
  ui->tableEntregas->hideColumn("dataRealColeta");
  ui->tableEntregas->hideColumn("dataPrevReceb");
  ui->tableEntregas->hideColumn("dataRealReceb");
  ui->tableEntregas->hideColumn("descUnitario");
  ui->tableEntregas->hideColumn("estoque_promocao");
}

void EntregasCliente::on_pushButtonNFe_clicked() {
  // TODO: escolher transportadora para envio dos produtos apos gerar nfe
  // TODO: colocar InputDlg prev/Real
  QList<int> lista;

  for (const auto index :
       modelProdutos.match(modelProdutos.index(0, modelProdutos.fieldIndex("selecionado")), Qt::DisplayRole, true, -1,
                           Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap))) {
    // NOTE: readd these (these wont be necessary after filtering the table to show only the ready to send
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

  const QString dirEntrada = UserSession::settings("User/pastaEntACBr").toString();
  const QString dirSaida = UserSession::settings("User/pastaSaiACBr").toString();
  const QString dirXml = UserSession::settings("User/pastaXmlACBr").toString();
  const QVariant loja = UserSession::settings("User/lojaACBr");

  if (dirEntrada.isEmpty() or dirSaida.isEmpty() or dirXml.isEmpty() or loja.isNull()) {
    QMessageBox::critical(this, "Erro!", "Por favor definir os parâmetros do ACBr nas configurações do usuário!");
    return;
  }

  CadastrarNFe *nfe = new CadastrarNFe(idVenda, this);
  nfe->prepararNFe(lista);
  //  nfe->show();
  nfe->exec();

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela venda_has_produto: " + modelProdutos.lastError().text());
    return;
  }

  for (int row = 0; row < modelProdutos.rowCount(); ++row) {
    ui->tableProdutos->openPersistentEditor(row, "selecionado");
  }

  ui->tableProdutos->resizeColumnsToContents();
}

void EntregasCliente::viewEntrega(const QString &idVenda) {
  this->idVenda = idVenda;

  // TODO: add filter status = 'ESTOQUE' (por hora desativado para testar)
  modelProdutos.setFilter("idVenda = '" + idVenda + "' AND idNfeSaida IS NULL");

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela venda_has_produto: " + modelProdutos.lastError().text());
    return;
  }

  modelEntregas.setFilter("idVenda = '" + idVenda + "' AND idNfeSaida IS NOT NULL");

  if (not modelEntregas.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela venda_has_produto: " + modelProdutos.lastError().text());
    return;
  }

  for (int row = 0; row < modelProdutos.rowCount(); ++row) {
    ui->tableProdutos->openPersistentEditor(row, "selecionado");
  }

  ui->tableProdutos->resizeColumnsToContents();
  ui->tableEntregas->resizeColumnsToContents();
}

void EntregasCliente::on_tableProdutos_entered(const QModelIndex &) { ui->tableProdutos->resizeColumnsToContents(); }

void EntregasCliente::on_tableEntregas_entered(const QModelIndex &) { ui->tableEntregas->resizeColumnsToContents(); }

void EntregasCliente::on_checkBoxMarcarTodos_clicked(bool checked) {
  for (int row = 0, rowCount = modelProdutos.rowCount(); row < rowCount; ++row) {
    modelProdutos.setData(row, "selecionado", checked);
  }
}

void EntregasCliente::on_pushButtonImprimir_clicked() {
  // TODO: botao para imprimir danfe
}

void EntregasCliente::on_pushButtonReagendar_clicked() {
  // TODO: implement
}
