#include "cadastrarnfe.h"
#include "cadastrocliente.h"
#include "ui_cadastrarnfe.h"

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

}

CadastrarNFE::~CadastrarNFE() {
  delete ui;
}

void CadastrarNFE::on_pushButtonGerarNFE_clicked() {

}

void CadastrarNFE::on_pushButtonCancelar_clicked() {
  close();
}

void CadastrarNFE::setItemData(int row, const QString &key, const QVariant & value) {
  if(modelItem.fieldIndex(key) == -1){
    qDebug() << "A chave '" + key + "' não existe na tabela de Itens da NFe";
  }
  modelItem.setData(modelItem.index(row,modelItem.fieldIndex(key)),value);
}

void CadastrarNFE::gerarNFe(QString idVenda, QList<int> items) {
  QSqlQuery qryVenda("SELECT * FROM Venda WHERE idVenda = '" + idVenda +  "'");
  if(!qryVenda.first()){
    qDebug() << "Erro lendo itens da venda. ERRO: " << qryVenda.lastError();
  }
  qDebug() << idVenda << ", " << items;
  ui->doubleSpinBoxFinal->setValue(qryVenda.value("total").toDouble());
  ui->doubleSpinBoxFrete->setValue(qryVenda.value("frete").toDouble());
  ui->itemBoxCliente->setValue(qryVenda.value("idCliente"));
  ui->itemBoxEndereco->searchDialog()->setFilter("idCliente = " + QString::number(ui->itemBoxCliente->value().toInt()) +
                                                    " AND ativo = 1");;
  ui->itemBoxEndereco->setValue(qryVenda.value("idEnderecoFaturamento"));
  double descontoGlobal = qryVenda.value("descontoPorc").toDouble();
  foreach (int item, items) {
    QSqlQuery qryItens("SELECT * FROM Venda_has_Produto NATURAL LEFT JOIN Produto WHERE idVenda = '" + idVenda +  "' AND item = '" + QString::number(item) + "'");

    if(!qryItens.exec() || !qryItens.first()){
      qDebug() << "Erro buscando produto. ERRO: " << qryItens.lastError();
      qDebug() << "Last query: " << qryItens.lastQuery();
      continue;
    }
    int row = modelItem.rowCount();
    modelItem.insertRow(row);
    setItemData(row, primaryKey, 0 ); //FIXME idNFE
    setItemData(row, "item", row);
    setItemData(row, "cst", "060");
    setItemData(row, "cfop", "5405");
//    setItemData(row, "baseICMS", qryItens.value(""));
//    setItemData(row, "valorICMS", qryItens.value(""));
//    setItemData(row, "valorIPI", qryItens.value(""));
//    setItemData(row, "aliquotaICMS", qryItens.value(""));
//    setItemData(row, "aliquotaIPI", qryItens.value(""));

    setItemData(row, "codComercial", qryItens.value("codComercial"));
    setItemData(row, "descricao", qryItens.value("produto"));
    setItemData(row, "ncm", qryItens.value("ncm"));
    setItemData(row, "un", qryItens.value("un"));
    setItemData(row, "qte", qryItens.value("qte"));
    setItemData(row, "valorUnitario", qryItens.value("prcUnitario"));

    double total = qryItens.value("total").toDouble() * (1.0 - (descontoGlobal/100.0));
    setItemData(row, "valorTotal", total);
  }
  ui->tableView->resizeColumnsToContents();
}

// RegisterDialog interface

bool CadastrarNFE::verifyFields(int row) {
}

bool CadastrarNFE::savingProcedures(int row) {
}

void CadastrarNFE::clearFields() {
}

void CadastrarNFE::setupMapper() {
}

void CadastrarNFE::registerMode() {
}

void CadastrarNFE::updateMode() {
}

// End of RegisterDialog interface
