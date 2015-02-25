#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlRelationalDelegate>
#include <QTime>
#include <QXmlStreamWriter>

#include "mainwindow.h"
#include "nfe.h"
#include "orcamento.h"
#include "ui_venda.h"
#include "venda.h"

Venda::Venda(QWidget *parent) : QDialog(parent), ui(new Ui::Venda) {
  ui->setupUi(this);
  modelItem.setTable("Venda_has_Produto");
  //  modelItem.setRelation(modelItem.fieldIndex("idLoja"), QSqlRelation("loja", "idLoja", "descricao"));
  //  modelItem.setRelation(modelItem.fieldIndex("idProduto"), QSqlRelation("produto", "idProduto",
  //  "Fornecedor"));
  modelItem.setHeaderData(modelItem.fieldIndex("fornecedor"), Qt::Horizontal, "Fornecedor");
  modelItem.setHeaderData(modelItem.fieldIndex("produto"), Qt::Horizontal, "Produto");
  modelItem.setHeaderData(modelItem.fieldIndex("obs"), Qt::Horizontal, "Obs.");
  modelItem.setHeaderData(modelItem.fieldIndex("prcUnitario"), Qt::Horizontal, "Preço/Un");
  modelItem.setHeaderData(modelItem.fieldIndex("qte"), Qt::Horizontal, "Qte.");
  modelItem.setHeaderData(modelItem.fieldIndex("un"), Qt::Horizontal, "Un.");
  modelItem.setHeaderData(modelItem.fieldIndex("unCaixa"), Qt::Horizontal, "Un/Caixa");
  modelItem.setHeaderData(modelItem.fieldIndex("parcial"), Qt::Horizontal, "Parcial");
  modelItem.setHeaderData(modelItem.fieldIndex("desconto"), Qt::Horizontal, "Desconto");
  modelItem.setHeaderData(modelItem.fieldIndex("parcialDesc"), Qt::Horizontal, "Desc. Parc.");
  modelItem.setHeaderData(modelItem.fieldIndex("descGlobal"), Qt::Horizontal, "Desc. Glob.");
  modelItem.setHeaderData(modelItem.fieldIndex("total"), Qt::Horizontal, "Total");
  modelItem.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelItem.select();

  modelFluxoCaixa.setTable("Venda_has_Pagamento");
  modelFluxoCaixa.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelFluxoCaixa.setHeaderData(modelFluxoCaixa.fieldIndex("tipo"), Qt::Horizontal, "Tipo");
  modelFluxoCaixa.setHeaderData(modelFluxoCaixa.fieldIndex("parcela"), Qt::Horizontal, "Parcelas");
  modelFluxoCaixa.setHeaderData(modelFluxoCaixa.fieldIndex("data"), Qt::Horizontal, "Data");
  modelFluxoCaixa.select();

  modelFluxoCaixa.insertRow(modelFluxoCaixa.rowCount());
  modelFluxoCaixa.insertRow(modelFluxoCaixa.rowCount());
  modelFluxoCaixa.insertRow(modelFluxoCaixa.rowCount());

  ui->tableFluxoCaixa->setModel(&modelFluxoCaixa);
  ui->tableFluxoCaixa->setColumnHidden(modelFluxoCaixa.fieldIndex("idVenda"), true);
  ui->tableFluxoCaixa->setColumnHidden(modelFluxoCaixa.fieldIndex("idLoja"), true);
  ui->tableFluxoCaixa->setColumnHidden(modelFluxoCaixa.fieldIndex("idPagamento"), true);

  modelVenda.setTable("Venda");
  modelVenda.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelVenda.select();

  ui->tableVenda->setModel(&modelItem);
  ui->tableVenda->setColumnHidden(modelItem.fieldIndex("idVenda"), true);
  ui->tableVenda->setColumnHidden(modelItem.fieldIndex("idLoja"), true);
  ui->tableVenda->setColumnHidden(modelItem.fieldIndex("idProduto"), true);
  ui->tableVenda->setColumnHidden(modelItem.fieldIndex("item"), true);
  ui->tableVenda->setColumnHidden(modelItem.fieldIndex("status"), true);

  QStringList list;
  list << "Escolha uma opção!"
       << "Cartão de débito"
       << "Cartão de crédito"
       << "Cheque"
       << "Dinheiro";
  ui->comboBoxPgt1->insertItems(0, list);
  ui->comboBoxPgt2->insertItems(0, list);
  ui->comboBoxPgt3->insertItems(0, list);

  //  ui->tableVenda->resizeColumnsToContents();
  ui->tableVenda->setItemDelegate(new QSqlRelationalDelegate(ui->tableVenda));
  //  ui->tableVenda->setItemDelegateForColumn(11, );

  ui->pushButtonNFe->hide();

  show();
}

Venda::~Venda() {
  delete ui;
}

void Venda::fecharOrcamento(const QString &idOrcamento) {
  clearFields();
  this->idOrcamento = idOrcamento;
  QSqlQuery produtos("SELECT * FROM Orcamento_has_Produto WHERE idOrcamento = '" + idOrcamento + "'");
  if (!produtos.exec()) {
    qDebug() << "Erro buscando produtos: " << produtos.lastError();
  }

  modelItem.setFilter("idVenda = '" + idOrcamento + "'");

  while (produtos.next()) {
    //    qDebug() << "Adding to venda";
    int row = modelItem.rowCount();
    modelItem.insertRow(row);
    for (int field = 0; field < produtos.record().count(); field++) {
      modelItem.setData(modelItem.index(row, field), produtos.value(field));
    }
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("status")), "PENDENTE");
  }
  ui->tableVenda->resizeColumnsToContents();

  QSqlQuery qry("SELECT * FROM orcamento WHERE idOrcamento = '" + idOrcamento + "'");
  if (!qry.exec()) {
    qDebug() << "Erro buscando orcamento: " << qry.lastError();
  }

  if (!qry.first()) {
    qDebug() << "Erro selecionando primeiro resultado: " << qry.lastError();
  }

  int row = modelVenda.rowCount();
  modelVenda.insertRow(row);
  for (int field = 0; field < modelVenda.columnCount(); field++) {
    modelVenda.setData(modelVenda.index(row, field), qry.value(field));
  }

  ui->doubleSpinBoxPgt1->setValue(modelVenda.data(modelVenda.index(row, modelVenda.fieldIndex("total"))).toDouble());
  ui->doubleSpinBoxPgt1->setMaximum(ui->doubleSpinBoxPgt1->value());

  ui->doubleSpinBoxTotal->setValue(modelVenda.data(modelVenda.index(row, modelVenda.fieldIndex("total"))).toDouble());
  //  ui->doubleSpinBoxTotal->setValue(
  //    modelVenda.data(modelVenda.index(row, modelVenda.fieldIndex("total"))).toDouble());
  //  ui->doubleSpinBoxRestante->setValue(
  //    modelVenda.data(modelVenda.index(row, modelVenda.fieldIndex("total"))).toDouble());
  //  ui->doubleSpinBoxPgt1->setMaximum( ui->doubleSpinBoxRestante->value());
}

bool Venda::cadastrar() {
  return true;
}

bool Venda::verifyFields() {
  return true;
}

bool Venda::verifyRequiredField(QLineEdit *line) {
  line->objectName();
  return true;
}

QString Venda::requiredStyle() {
  return QString();
}

void Venda::calcPrecoGlobalTotal() {}

void Venda::calcPrecoItemTotal() {}

void Venda::clearFields() {
  idOrcamento = QString();
}

void Venda::setupMapper() {}

void Venda::updateId() {}

void Venda::on_pushButtonCancelar_clicked() {
  Orcamento *orc = new Orcamento(parentWidget());
//  orc->updateRegister(idOrcamento);
  orc->viewRegisterById(idOrcamento);
  qDebug() << "idOrcamento: " << idOrcamento;
  close();
}

void Venda::on_pushButtonFecharPedido_clicked() {
  //TODO: verificar se cadastro do cliente está incompleto

  QSqlQuery qry;

  qDebug() << "id: " << idOrcamento;

  qry.exec("START TRANSACTION");

  if (!qry.exec("INSERT INTO Venda SELECT idOrcamento, idLoja, idUsuario, idCadastroCliente, idEnderecoEntrega, idProfissional, data, total, desconto, frete, validade, status FROM orcamento WHERE idOrcamento = '" + idOrcamento + "'")) {
    qDebug() << "Error inserting into Venda " << qry.lastError();
    qry.exec("ROLLBACK");
    return;
  }

  if (!qry.exec("UPDATE Venda SET status = 'ABERTO' WHERE idVenda = '" + idOrcamento + "'")) {
    qDebug() << "Error updating status from Venda: " << qry.lastError();
    qry.exec("ROLLBACK");
    return;
  }

  if(!qry.exec("UPDATE Venda SET data = '"+ QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +"'"))

    if (!modelItem.submitAll()) {
      qDebug() << "Error submitting itemModel" << modelItem.lastError();
      //    qDebug() << "query: " << modelItem.query().lastQuery();
      qry.exec("ROLLBACK");
      return;
    }

  QSqlQuery qryEstoque("SELECT produto.descricao, produto.estoque, venda_has_produto.idVenda FROM "
                       "venda_has_produto INNER JOIN produto ON produto.idProduto = "
                       "venda_has_produto.idProduto WHERE estoque = 0 AND venda_has_produto.idVenda = '" +
                       idOrcamento + "';");
  if (!qryEstoque.exec()) {
    qDebug() << qryEstoque.lastError();
  }
  if (qryEstoque.size() > 0) {
    if (!qry.exec(
          "INSERT INTO pedidofornecedor (idPedido, idLoja, idUsuario, idCadastroCliente, "
          "idEnderecoEntrega, idProfissional, data, total, desconto, frete, validade, status) SELECT * "
          "FROM venda WHERE idVenda = '" +
          idOrcamento + "'")) {
      qDebug() << "Erro na criação do pedido fornecedor: " << qry.lastError();
      qry.exec("ROLLBACK");
      return;
    }
    if (!qry.exec("INSERT INTO pedidofornecedor_has_produto SELECT venda_has_produto.* FROM "
                  "venda_has_produto INNER JOIN produto ON produto.idProduto = "
                  "venda_has_produto.idProduto WHERE estoque = 0 AND venda_has_produto.idVenda = '" +
                  idOrcamento + "';")) {
      qDebug() << "Erro na inserção dos produtos em pedidofornecedor_has_produto: " << qry.lastError();
      qry.exec("ROLLBACK");
      return;
    }
  }

  if (!qry.exec("DELETE FROM Orcamento_has_Produto WHERE idOrcamento = '" + idOrcamento + "'")) {
    qDebug() << "Error deleting items from Orcamento_has_Produto: " << qry.lastError();
    qry.exec("ROLLBACK");
  }

  if (!qry.exec("DELETE FROM orcamento WHERE idOrcamento = '" + idOrcamento + "'")) {
    qDebug() << "Error deleting row from Orcamento: " << qry.lastError();
    qry.exec("ROLLBACK");
  }

  if (!qry.exec("INSERT INTO contaareceber (dataEmissao, idVenda) VALUES ('" +
                QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "', '" + idOrcamento + "')")) {
    qDebug() << "Error inserting contaareceber: " << qry.lastError();
    qry.exec("ROLLBACK");
  }

  if (qry.exec("COMMIT")) {
    QMessageBox::information(this, "Atenção!", "Venda cadastrada com sucesso!", QMessageBox::Ok,
                             QMessageBox::NoButton);
  }

  if (MainWindow *window = qobject_cast<MainWindow *>(parentWidget())) {
    window->updateTables();
  } else {
    qDebug() << "something went wrong";
  }

  close();
}

void Venda::on_pushButtonNFe_clicked() {
  NFe nota(idOrcamento, this);
  qDebug() << nota.XML();
}

void Venda::viewVenda(QString idVenda) {
  clearFields();
  idOrcamento = idVenda;

  setWindowTitle("Venda: " + idVenda);
  ui->pushButtonCancelar->hide();
  ui->pushButtonFecharPedido->hide();

  modelItem.setFilter("idVenda = '" + idVenda + "'");
  ui->tableVenda->resizeColumnsToContents();
}

void Venda::on_doubleSpinBoxPgt1_valueChanged(double) {

}

void Venda::on_doubleSpinBoxPgt2_valueChanged(double) {

}

void Venda::on_doubleSpinBoxPgt3_valueChanged(double) {

}

//void Venda::on_doubleSpinBoxRestante_valueChanged(double value) {
//  Q_UNUSED(value);
//}

void Venda::on_doubleSpinBoxPgt1_editingFinished() {
  double pgt1 = ui->doubleSpinBoxPgt1->value();
  double pgt2 = ui->doubleSpinBoxPgt2->value();
  double pgt3 = ui->doubleSpinBoxPgt3->value();
  double total = ui->doubleSpinBoxTotal->value();
  double restante = total - (pgt1 + pgt2 + pgt3);
  ui->doubleSpinBoxPgt2->setValue(pgt2 + restante);
  ui->doubleSpinBoxPgt1->setMaximum(pgt1);
  ui->doubleSpinBoxPgt2->setMaximum(pgt2 + restante);
  ui->doubleSpinBoxPgt3->setMaximum(pgt3);
  if(pgt2 == 0.0 ) {
    ui->doubleSpinBoxPgt2->setMaximum(restante);
    ui->doubleSpinBoxPgt2->setValue(restante);
    ui->doubleSpinBoxPgt3->setMaximum(pgt3);
    restante = 0;
  } else if(pgt3 == 0.0) {
    ui->doubleSpinBoxPgt3->setMaximum(restante);
    ui->doubleSpinBoxPgt3->setValue(restante);
    ui->doubleSpinBoxPgt2->setMaximum(pgt2);
    restante = 0;
  }else{
    ui->doubleSpinBoxPgt2->setMaximum(pgt2 + restante);
    ui->doubleSpinBoxPgt3->setMaximum(pgt3 + restante);
  }
  ui->doubleSpinBoxPgt1->setMaximum(pgt1 + restante);
//  ui->doubleSpinBoxRestante->setValue(restante);
}

void Venda::on_doubleSpinBoxPgt2_editingFinished() {
  double pgt1 = ui->doubleSpinBoxPgt1->value();
  double pgt2 = ui->doubleSpinBoxPgt2->value();
  double pgt3 = ui->doubleSpinBoxPgt3->value();
  double total = ui->doubleSpinBoxTotal->value();
  double restante = total - (pgt1 + pgt2 + pgt3);
  if(pgt3 == 0.0) {
    ui->doubleSpinBoxPgt3->setMaximum(restante);
    ui->doubleSpinBoxPgt3->setValue(restante);
    restante = 0;
  }
  ui->doubleSpinBoxPgt1->setMaximum(pgt1 + restante);
  ui->doubleSpinBoxPgt2->setMaximum(pgt2 + restante);
//  ui->doubleSpinBoxRestante->setValue(restante);
}

void Venda::on_doubleSpinBoxPgt3_editingFinished() {
  double pgt1 = ui->doubleSpinBoxPgt1->value();
  double pgt2 = ui->doubleSpinBoxPgt2->value();
  double pgt3 = ui->doubleSpinBoxPgt3->value();
  double total = ui->doubleSpinBoxTotal->value();
  double restante = total - (pgt1 + pgt2 + pgt3);
//  ui->doubleSpinBoxRestante->setValue(restante);
  ui->doubleSpinBoxPgt1->setMaximum(pgt1 + restante);
  ui->doubleSpinBoxPgt2->setMaximum(pgt2 + restante);
}

void Venda::on_comboBoxPgt1_currentTextChanged(const QString &text)
{
  if(text == "Escolha uma opção!"){
    return;
  }
  if(text == "Cartão de crédito" or text == "Cheque"){
    ui->comboBoxPgt1Parc->setEnabled(true);
  }else{
    ui->comboBoxPgt1Parc->setDisabled(true);
  }

  modelFluxoCaixa.setData(modelFluxoCaixa.index(0, modelFluxoCaixa.fieldIndex("tipo")), text);
  modelFluxoCaixa.setData(modelFluxoCaixa.index(0, modelFluxoCaixa.fieldIndex("parcela")), ui->comboBoxPgt1Parc->currentIndex() + 1);
    modelFluxoCaixa.setData(modelFluxoCaixa.index(0, modelFluxoCaixa.fieldIndex("valor")), ui->doubleSpinBoxPgt1->value());
  modelFluxoCaixa.setData(modelFluxoCaixa.index(0, modelFluxoCaixa.fieldIndex("data")), QDateTime::currentDateTime());
}

void Venda::on_comboBoxPgt2_currentTextChanged(const QString &text)
{
  if(text == "Escolha uma opção!"){
    return;
  }
  if(text == "Cartão de crédito" or text == "Cheque"){
    ui->comboBoxPgt2Parc->setEnabled(true);
  }else{
    ui->comboBoxPgt2Parc->setDisabled(true);
  }

  modelFluxoCaixa.setData(modelFluxoCaixa.index(1, modelFluxoCaixa.fieldIndex("tipo")), text);
  modelFluxoCaixa.setData(modelFluxoCaixa.index(1, modelFluxoCaixa.fieldIndex("parcela")), ui->comboBoxPgt1Parc->currentIndex() + 1);
  modelFluxoCaixa.setData(modelFluxoCaixa.index(1, modelFluxoCaixa.fieldIndex("valor")), ui->doubleSpinBoxPgt2->value());
  modelFluxoCaixa.setData(modelFluxoCaixa.index(1, modelFluxoCaixa.fieldIndex("data")), QDateTime::currentDateTime());
}

void Venda::on_comboBoxPgt3_currentTextChanged(const QString &text)
{
  if(text == "Escolha uma opção!"){
    return;
  }
  if(text == "Cartão de crédito" or text == "Cheque"){
    ui->comboBoxPgt3Parc->setEnabled(true);
  }else{
    ui->comboBoxPgt3Parc->setDisabled(true);
  }

  modelFluxoCaixa.setData(modelFluxoCaixa.index(2, modelFluxoCaixa.fieldIndex("tipo")), text);
  modelFluxoCaixa.setData(modelFluxoCaixa.index(2, modelFluxoCaixa.fieldIndex("parcela")), ui->comboBoxPgt1Parc->currentIndex() + 1);
    modelFluxoCaixa.setData(modelFluxoCaixa.index(2, modelFluxoCaixa.fieldIndex("valor")), ui->doubleSpinBoxPgt3->value());
  modelFluxoCaixa.setData(modelFluxoCaixa.index(2, modelFluxoCaixa.fieldIndex("data")), QDateTime::currentDateTime());
}
