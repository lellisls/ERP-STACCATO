#include "cadastrarnfe.h"
#include "cadastrocliente.h"
#include "endereco.hpp"
#include "ui_cadastrarnfe.h"

#include <QFile>

CadastrarNFE::CadastrarNFE(QWidget *parent) :
  RegisterDialog("NFe","idNFe",parent),
  ui(new Ui::CadastrarNFE) {
  ui->setupUi(this);
  SearchDialog * sdCliente = SearchDialog::cliente(ui->itemBoxCliente);
  ui->itemBoxCliente->setSearchDialog(sdCliente);

  RegisterDialog * cadCliente = new CadastroCliente(this);
  ui->itemBoxCliente->setRegisterDialog(cadCliente);

  SearchDialog * sdEndereco = SearchDialog::endereco(ui->itemBoxEndereco);
  ui->itemBoxEndereco->setSearchDialog(sdEndereco);

  modelItem.setTable("NFe_has_Itens");
  modelItem.select();
  ui->tableView->setModel(&modelItem);


  ui->tableView->setColumnHidden(model.fieldIndex("idNFe"),true);
}

CadastrarNFE::~CadastrarNFE() {
  delete ui;
}

void CadastrarNFE::on_pushButtonGerarNFE_clicked() {
  save();
}

void CadastrarNFE::on_pushButtonCancelar_clicked() {
  close();
}

void CadastrarNFE::updateImpostos() {
  double icms = 0;
  for(int row = 0; row < modelItem.rowCount(); ++row) {
    icms += getItemData(row,"valorICMS").toDouble();
  }
  ui->doubleSpinBoxVlICMS->setValue(icms);
  double imposto = 0.593 * ui->doubleSpinBoxFinal->value() + icms;
  Endereco end(ui->itemBoxEndereco->value().toInt());
  QString texto = "Venda de código " + data("idVenda").toString() + "\n"
                  + "END. ENTREGA: " +  end.umaLinha() + "\n"
                  + "Informações Adicionais de Interesse do Fisco: ICMS RECOLHIDO ANTECIPADAMENTE CONFORME ARTIGO 3113Y\n"
                  + "Total aproximado de tributos federais, estaduais e municipais: " + QString::number(imposto);
  ui->textEdit->setPlainText(texto);
  setData("obs",texto);
}

void CadastrarNFE::getItemData(int row, const QString &key, const QVariant & value) {
  if(modelItem.fieldIndex(key) == -1) {
    qDebug() << "A chave '" + key + "' não existe na tabela de Itens da NFe";
  }
  modelItem.setData(modelItem.index(row,modelItem.fieldIndex(key)),value);
}

QVariant CadastrarNFE::getItemData(int row, const QString & key) {
  if(modelItem.fieldIndex(key) == -1) {
    qDebug() << "A chave '" + key + "' não existe na tabela de Itens da NFe";
  }
  return(modelItem.data(modelItem.index(row,modelItem.fieldIndex(key))));
}

void CadastrarNFE::gerarNFe(QString idVenda, QList<int> items) {
  QSqlQuery qryVenda("SELECT * FROM Venda WHERE idVenda = '" + idVenda +  "'");
  if(!qryVenda.first()) {
    qDebug() << "Erro lendo itens da venda. ERRO: " << qryVenda.lastError();
  }
  qDebug() << idVenda << ", " << items;
  newRegister();
  setData("idVenda",idVenda);
  setData("frete",qryVenda.value("frete"));
  setData("total",qryVenda.value("total"));
  setData("idLoja",qryVenda.value("idLoja"));
  setData("idCliente",qryVenda.value("idCliente"));
  setData("idEnderecoFaturamento",qryVenda.value("idEnderecoFaturamento"));
  ui->doubleSpinBoxFinal->setValue(qryVenda.value("total").toDouble());
  ui->doubleSpinBoxFrete->setValue(qryVenda.value("frete").toDouble());
  ui->itemBoxCliente->setValue(qryVenda.value("idCliente"));
  ui->itemBoxEndereco->searchDialog()->setFilter("idCliente = " + QString::number(ui->itemBoxCliente->value().toInt()) +
      " AND ativo = 1");;
  ui->itemBoxEndereco->setValue(qryVenda.value("idEnderecoFaturamento"));
  double descontoGlobal = qryVenda.value("descontoPorc").toDouble();
  foreach (int item, items) {
    QSqlQuery qryItens("SELECT * FROM Venda_has_Produto NATURAL LEFT JOIN Produto WHERE idVenda = '" + idVenda +  "' AND item = '" + QString::number(item) + "'");

    if(!qryItens.exec() || !qryItens.first()) {
      qDebug() << "Erro buscando produto. ERRO: " << qryItens.lastError();
      qDebug() << "Last query: " << qryItens.lastQuery();
      continue;
    }
    int row = modelItem.rowCount();
    modelItem.insertRow(row);
    getItemData(row, primaryKey, 0 ); //FIXME idNFE
    getItemData(row, "item", row);
    getItemData(row, "cst", "060");
    getItemData(row, "cfop", "5405");
//    setItemData(row, "baseICMS", qryItens.value(""));
//    setItemData(row, "valorICMS", qryItens.value(""));
//    setItemData(row, "valorIPI", qryItens.value(""));
//    setItemData(row, "aliquotaICMS", qryItens.value(""));
//    setItemData(row, "aliquotaIPI", qryItens.value(""));

    getItemData(row, "codComercial", qryItens.value("codComercial"));
    getItemData(row, "descricao", qryItens.value("produto"));
    getItemData(row, "ncm", qryItens.value("ncm"));
    getItemData(row, "un", qryItens.value("un"));
    getItemData(row, "qte", qryItens.value("qte"));
    getItemData(row, "valorUnitario", qryItens.value("prcUnitario"));

    double total = qryItens.value("total").toDouble() * (1.0 - (descontoGlobal/100.0));
    getItemData(row, "valorTotal", total);
  }
  ui->tableView->resizeColumnsToContents();
  updateImpostos();
}

// RegisterDialog interface

bool CadastrarNFE::verifyFields(int row) {
  return true;
}

bool CadastrarNFE::savingProcedures(int row) {

  return true;
}

void CadastrarNFE::clearFields() {
}

void CadastrarNFE::setupMapper() {
  addMapping(ui->itemBoxCliente,"idCliente");
  addMapping(ui->itemBoxEndereco,"idEnderecoFaturamento");
  addMapping(ui->doubleSpinBoxFrete,"frete");
  addMapping(ui->doubleSpinBoxFinal,"total");
  addMapping(ui->textEdit,"obs");
}

void CadastrarNFE::registerMode() {
}

void CadastrarNFE::updateMode() {

}

// End of RegisterDialog interface

void CadastrarNFE::on_tableView_activated(const QModelIndex &index) {
  updateImpostos();
}

void CadastrarNFE::on_tableView_pressed(const QModelIndex &index) {
  updateImpostos();
}

void CadastrarNFE::writeTXT() {

}

void CadastrarNFE::generateNFE(QTextStream & stream) {

}

QString CadastrarNFE::criarChaveAcesso() {

}
