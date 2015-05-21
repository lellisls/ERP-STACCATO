#include "cadastrarnfe.h"
#include "cadastrocliente.h"
#include "endereco.hpp"
#include "nfe.h"
#include "ui_cadastrarnfe.h"
#include "usersession.h"

#include <QFile>
#include <QDate>
#include <QFileInfo>

CadastrarNFE::CadastrarNFE(QString idOrcamento, QWidget *parent) : QDialog(parent), ui(new Ui::CadastrarNFE), idOrcamento(idOrcamento){
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  SearchDialog *sdCliente = SearchDialog::cliente(ui->itemBoxCliente);
  ui->itemBoxCliente->setSearchDialog(sdCliente);

  SearchDialog *sdEndereco = SearchDialog::endereco(ui->itemBoxEndereco);
  ui->itemBoxEndereco->setSearchDialog(sdEndereco);

  RegisterDialog *cadCliente = new CadastroCliente(this);
  ui->itemBoxCliente->setRegisterDialog(cadCliente);

  modelNFe.setTable("NFe");
  modelNFe.setEditStrategy(EditableSqlModel::OnManualSubmit);
  modelNFe.select();

  modelItem.setTable("NFe_has_Itens");
  modelItem.setEditStrategy(EditableSqlModel::OnManualSubmit);
  modelItem.select();
  ui->tableView->setModel(&modelItem);
  ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableView->setColumnHidden(modelNFe.fieldIndex("idNFe"), true);

  mapper.setModel(&modelNFe);
  mapper.addMapping(ui->itemBoxCliente, modelNFe.fieldIndex("idCliente"));
  mapper.addMapping(ui->itemBoxEndereco, modelNFe.fieldIndex("idEnderecoFaturamento"));
  mapper.addMapping(ui->doubleSpinBoxFrete, modelNFe.fieldIndex("frete"));
  mapper.addMapping(ui->doubleSpinBoxFinal, modelNFe.fieldIndex("total"));
  mapper.addMapping(ui->textEdit, modelNFe.fieldIndex("obs"));
  mapper.toLast();

  connect(ui->tableView->model(), &QAbstractItemModel::dataChanged, this, &CadastrarNFE::onDataChanged);

  modelLoja.setTable("Loja");
  modelLoja.setEditStrategy(QSqlTableModel::OnManualSubmit);
  QString idLoja = QString::number(UserSession::getLoja());
  modelLoja.setFilter("idLoja = '" + idLoja + "'");
  modelLoja.select();

  modelVenda.setTable("Venda");
  modelVenda.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelVenda.setFilter("idVenda = '" + idOrcamento + "'");
  modelVenda.select();
  qDebug() << "idOrcamento: " << idOrcamento;

  modelProd.setTable("Venda_has_Produto");
  modelProd.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelProd.setFilter("idVenda = '" + idOrcamento + "'");
  modelProd.select();
}

CadastrarNFE::~CadastrarNFE() { delete ui; }

void CadastrarNFE::on_pushButtonGerarNFE_clicked() {
  qDebug() << ui->tableView->selectionModel()->selectedRows();

  //  NFe nota(idOrcamento, this);

  //  if (nota.TXT()) {
  //    QMessageBox::information(this, "Aviso!", "NFe gerada com sucesso!");
  //  } else {
  //    QMessageBox::warning(this, "Aviso!", "Ocorreu algum erro, NFe não foi gerada.");
  //  }

  if(generateNFE()){
    QMessageBox::information(this, "Aviso!", "NFe gerada com sucesso!");
  } else {
    QMessageBox::warning(this, "Aviso!", "Ocorreu algum erro, NFe não foi gerada.");
  }
}

void CadastrarNFE::on_pushButtonCancelar_clicked() { close(); }

void CadastrarNFE::updateImpostos() {
  double icms = 0;
  for (int row = 0; row < modelItem.rowCount(); ++row) {
    icms += getItemData(row, "valorICMS").toDouble();
  }
  ui->doubleSpinBoxVlICMS->setValue(icms);
  double imposto = 0.593 * ui->doubleSpinBoxFinal->value() + icms;
  Endereco end(ui->itemBoxEndereco->value().toInt());
  QString texto =
      "Venda de código " +
      modelNFe.data(modelNFe.index(mapper.currentIndex(), modelNFe.fieldIndex("idVenda"))).toString() + "\n" +
      "END. ENTREGA: " + end.umaLinha() + "\n" +
      "Informações Adicionais de Interesse do Fisco: ICMS RECOLHIDO ANTECIPADAMENTE CONFORME ARTIGO 3113Y\n" +
      "Total aproximado de tributos federais, estaduais e municipais: " + QString::number(imposto);
  ui->textEdit->setPlainText(texto);
  modelNFe.setData(modelNFe.index(mapper.currentIndex(), modelNFe.fieldIndex("obs")), texto);
}

void CadastrarNFE::setItemData(int row, const QString &key, const QVariant &value) {
  if (modelItem.fieldIndex(key) == -1) {
    qDebug() << "A chave '" + key + "' não existe na tabela de Itens da NFe";
  }
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex(key)), value);
}

QVariant CadastrarNFE::getItemData(int row, const QString &key) {
  if (modelItem.fieldIndex(key) == -1) {
    qDebug() << "A chave '" + key + "' não existe na tabela de Itens da NFe";
  }
  return (modelItem.data(modelItem.index(row, modelItem.fieldIndex(key))));
}

void CadastrarNFE::gerarNFe(QList<int> items) {
  QSqlQuery qryVenda("SELECT * FROM Venda WHERE idVenda = '" + idOrcamento + "'");
  if (not qryVenda.first()) {
    qDebug() << "Erro lendo itens da venda. ERRO: " << qryVenda.lastError();
  }
  qDebug() << idOrcamento << ", " << items;

  // new register
  modelNFe.select();
  int row = modelNFe.rowCount();
  modelNFe.insertRow(row);
  mapper.toLast();
  //
  modelNFe.setData(modelNFe.index(mapper.currentIndex(), modelNFe.fieldIndex("idVenda")), idOrcamento);
  modelNFe.setData(modelNFe.index(mapper.currentIndex(), modelNFe.fieldIndex("frete")), qryVenda.value("frete"));
  modelNFe.setData(modelNFe.index(mapper.currentIndex(), modelNFe.fieldIndex("total")), qryVenda.value("total"));
  modelNFe.setData(modelNFe.index(mapper.currentIndex(), modelNFe.fieldIndex("idLoja")), qryVenda.value("idLoja"));
  modelNFe.setData(modelNFe.index(mapper.currentIndex(), modelNFe.fieldIndex("idCliente")), qryVenda.value("idCliente"));
  modelNFe.setData(modelNFe.index(mapper.currentIndex(), modelNFe.fieldIndex("idEnderecoFaturamento")), qryVenda.value("idEnderecoFaturamento"));

  ui->doubleSpinBoxFinal->setValue(qryVenda.value("total").toDouble());
  ui->doubleSpinBoxFrete->setValue(qryVenda.value("frete").toDouble());
  ui->itemBoxCliente->setValue(qryVenda.value("idCliente"));
  ui->itemBoxEndereco->searchDialog()->setFilter(
        "idCliente = " + QString::number(ui->itemBoxCliente->value().toInt()) + " AND ativo = 1");
  ;
  ui->itemBoxEndereco->setValue(qryVenda.value("idEnderecoFaturamento"));
  double descontoGlobal = qryVenda.value("descontoPorc").toDouble();
  for (auto item: items) {
    QSqlQuery qryItens("SELECT * FROM Venda_has_Produto NATURAL LEFT JOIN Produto WHERE idVenda = '" +
                       idOrcamento + "' AND item = '" + QString::number(item) + "'");

    if (not qryItens.exec() or not qryItens.first()) {
      qDebug() << "Erro buscando produto. ERRO: " << qryItens.lastError();
      qDebug() << "Last query: " << qryItens.lastQuery();
      continue;
    }
    int row = modelItem.rowCount();
    modelItem.insertRow(row);
    setItemData(row, "idNFe", 0); // FIXME idNFE
    setItemData(row, "item", row+1);
    setItemData(row, "cst", "060");
    setItemData(row, "cfop", "5405");
    setItemData(row, "codComercial", qryItens.value("codComercial"));
    setItemData(row, "descricao", qryItens.value("produto"));
    setItemData(row, "ncm", qryItens.value("ncm"));
    setItemData(row, "un", qryItens.value("un"));
    setItemData(row, "qte", qryItens.value("qte"));
    setItemData(row, "valorUnitario", qryItens.value("prcUnitario"));

    double total = qryItens.value("total").toDouble() * (1.0 - (descontoGlobal / 100.0));
    setItemData(row, "valorTotal", total);
  }
  ui->tableView->resizeColumnsToContents();
  updateImpostos();
}

void CadastrarNFE::on_tableView_activated(const QModelIndex &index) {
  Q_UNUSED(index);
  updateImpostos(); }

void CadastrarNFE::on_tableView_pressed(const QModelIndex &index) {
  Q_UNUSED(index);
  updateImpostos(); }

void CadastrarNFE::writeTXT() {}

bool CadastrarNFE::generateNFE() {
  QString chave = criarChaveAcesso();

  chave += calculaDigitoVerificador(chave);

  return writeTXT(chave);
}

QString CadastrarNFE::criarChaveAcesso() {
  QStringList listChave;
  listChave.push_back("35");                                  // cUF - código da UF
  listChave.push_back(QDate::currentDate().toString("yyMM")); // Ano/Mês
  QString cnpj = clearStr(getFromLoja("cnpj").toString());
  listChave.push_back(cnpj);        // CNPJ
  listChave.push_back("55");        // modelo
  listChave.push_back("001");       // série
  listChave.push_back("123456789"); // número nf-e
  listChave.push_back("1");         // tpEmis - forma de emissão
  listChave.push_back("76543210");  // código númerico
  //  qDebug() << "chave: " << vetorChave;
  QString chave = listChave.join("");

  return chave;
}

QString CadastrarNFE::clearStr(QString str)
{
  return str.remove(".").remove("/").remove("-").remove(" ").remove("(").remove(")");
}

void CadastrarNFE::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) {
  if (topLeft == bottomRight) { // only one cell changed
    //    qDebug() << "cell changed: " << topLeft;

    if (topLeft.column() == 11) { // baseICMS (valor produto + frete)
    }
    if (topLeft.column() == 12) { // valorICMS
    }
    if (topLeft.column() == 13) { // valorIPI
    }
    if (topLeft.column() == 14) { // aliquotaICMS
      if (topLeft.data().toInt() > 100) {
        //        qDebug() << "bigger than 100! limit.";
        modelItem.setData(topLeft, 100);
      }
      double icms =
          modelItem.data(modelItem.index(topLeft.row(), modelItem.fieldIndex("valorTotal"))).toDouble() *
          (topLeft.data().toDouble() / 100);
      qDebug() << "setting icms: "
               << modelItem.setData(modelItem.index(topLeft.row(), modelItem.fieldIndex("valorICMS")), icms);
    }
    if (topLeft.column() == 15) { // aliquotaIPI
      if (topLeft.data().toInt() > 100) {
        qDebug() << "bigger than 100! limit.";
        modelItem.setData(topLeft, 100);
      }
      double icms =
          modelItem.data(modelItem.index(topLeft.row(), modelItem.fieldIndex("valorTotal"))).toDouble() *
          (topLeft.data().toDouble() / 100);
      qDebug() << "setting icms: "
               << modelItem.setData(modelItem.index(topLeft.row(), modelItem.fieldIndex("valorIPI")), icms);
    }
  }
}

QVariant CadastrarNFE::getFromVenda(QString column) {
  return (modelVenda.data(modelVenda.index(0, modelVenda.fieldIndex(column))));
}

QVariant CadastrarNFE::getFromLoja(QString column) {
  return (modelLoja.data(modelLoja.index(0, modelLoja.fieldIndex(column))));
}

QVariant CadastrarNFE::getFromItemModel(int row, QString column) {
  return (modelItem.data(modelItem.index(row, modelItem.fieldIndex(column))));
}

QVariant CadastrarNFE::getFromProdModel(int row, QString column) {
  return (modelProd.data(modelProd.index(row, modelProd.fieldIndex(column))));
}

QString CadastrarNFE::calculaDigitoVerificador(QString chave) {
  if (chave.size() != 43) {
    qDebug() << "Erro na chave!";
    return QString();
  }

  QVector<int> chave2;
  for (int i = 0; i < chave.size(); ++i) {
    chave2.push_back(chave.at(i).digitValue());
  }

  QVector<int> multiplicadores = {4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2, 9, 8, 7,
                                  6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2};
  int soma = 0;
  //  qDebug() << "codigo size: " << codigo.size();
  //  qDebug() << "chave size: " << chave2.size();
  for (int i = 0; i < 43; ++i) {
    soma += chave2.at(i) * multiplicadores.at(i);
  }
  int resto = soma % 11;
  int cDV;

  if (resto == 1) {
    cDV = 0;
  } else {
    cDV = 11 - resto;
  }

  //  qDebug() << "soma: " << soma;
  //  qDebug() << "resto: " << resto;

  return QString::number(cDV);
}

bool CadastrarNFE::writeTXT(QString chave) {
  //  QFile file(idVenda + ".txt");
  QFile file("C:/ACBrNFeMonitor/ENTNFE.TXT/" + idOrcamento + ".txt");
  //  qDebug() << QDir::current().absoluteFilePath(idVenda + ".txt");
  if (not file.open(QFile::WriteOnly)) {
    QMessageBox::warning(0, "Aviso!", "Não foi possível criar a nota na pasta do ACBr, favor verificar se as "
                                      "pastas estão corretamente configuradas.");
    return false;
  }
  QFileInfo fileInfo(file);
  qDebug() << "path: " << fileInfo.absoluteFilePath();
  arquivo = fileInfo.absoluteFilePath().replace("/", "\\\\");

  chaveAcesso = "NFe" + chave;
  //  qDebug() << chaveAcesso;
  //  qDebug() << chaveAcesso.mid(38, 8);
  //  qDebug() << chaveAcesso.mid(25, 3);
  //  qDebug() << chaveAcesso.mid(28, 9);

  QTextStream stream(&file);

  stream << "NFe.CriarEnviarNFe(\"" << endl;

  qDebug() << "[Identificacao]";
  stream << "[Identificacao]" << endl;
  stream << "NaturezaOperacao = 5405-VENDA PROD/SERV D.ESTADO" << endl;
  stream << "Modelo = 55" << endl;
  stream << "Serie = " + chaveAcesso.mid(25, 3) << endl;
  stream << "Codigo = " + chaveAcesso.mid(25, 3) << endl;
  stream << "Numero = " + chaveAcesso.mid(28, 9) << endl;
  stream << "Emissao = " + QDate::currentDate().toString("dd/MM/yyyy") << endl;
  stream << "Saida = " + QDate::currentDate().toString("dd/MM/yyyy") << endl;
  stream << "Tipo = 1" << endl;
  stream << "FormaPag = 0" << endl;

  qDebug() << "[Emitente]";
  stream << "[Emitente]" << endl;

  if (clearStr(getFromLoja("cnpj").toString()).isEmpty()) {
    qDebug() << "cnpj vazio";
    return false;
  }
  stream << "CNPJ = " + clearStr(getFromLoja("cnpj").toString()) << endl;

  stream << "IE = " + getFromLoja("inscEstadual").toString() << endl;

  if (getFromLoja("razaoSocial").toString().isEmpty()) {
    qDebug() << "razaoSocial vazio";
    return false;
  }
  stream << "Razao = " + getFromLoja("razaoSocial").toString() << endl;

  if (getFromLoja("nomeFantasia").toString().isEmpty()) {
    qDebug() << "nomeFantasia vazio";
    return false;
  }
  stream << "Fantasia = " + getFromLoja("nomeFantasia").toString() << endl;

  if (getFromLoja("tel").toString().isEmpty()) {
    qDebug() << "tel vazio";
    return false;
  }
  stream << "Fone = " + getFromLoja("tel").toString() << endl;

  QString idEndLoja = getFromLoja("idEndereco").toString();
  if (idEndLoja.isEmpty()) {
    qDebug() << "idEndereco Loja vazio";
    return false;
  }
  QSqlQuery endLoja("SELECT * FROM Endereco WHERE idEndereco = '" + idEndLoja + "'");
  if (not endLoja.exec()) {
    qDebug() << "End. loja failed! : " << endLoja.lastError();
    return false;
  }
  endLoja.first();

  if (clearStr(endLoja.value("CEP").toString()).isEmpty()) {
    qDebug() << "CEP vazio";
    return false;
  }
  stream << "CEP = " + clearStr(endLoja.value("CEP").toString()) << endl;

  if (endLoja.value("logradouro").toString().isEmpty()) {
    qDebug() << "logradouro vazio";
    return false;
  }
  stream << "Logradouro = " + endLoja.value("logradouro").toString() << endl;

  if (endLoja.value("numero").toString().isEmpty()) {
    qDebug() << "numero vazio";
    return false;
  }
  stream << "Numero = " + endLoja.value("numero").toString() << endl;

  //  if(endLoja.value("complemento").toString().isEmpty()){
  //    qDebug() << "complemento vazio";
  //    return false;
  //  }
  stream << "Complemento = " + endLoja.value("complemento").toString() << endl;

  if (endLoja.value("bairro").toString().isEmpty()) {
    qDebug() << "bairro vazio";
    return false;
  }
  stream << "Bairro = " + endLoja.value("bairro").toString() << endl;

  if (endLoja.value("cidade").toString().isEmpty()) {
    qDebug() << "cidade vazio";
    return false;
  }
  stream << "Cidade = " + endLoja.value("cidade").toString() << endl;

  if (endLoja.value("uf").toString().isEmpty()) {
    qDebug() << "uf vazio";
    return false;
  }
  stream << "UF = " + endLoja.value("uf").toString() << endl;

  qDebug() << "[Destinatario]";
  stream << "[Destinatario]" << endl;

  QString idCliente = getFromVenda("idCliente").toString();
  if (idCliente.isEmpty()) {
    qDebug() << "idCliente vazio";
    return false;
  }

  QSqlQuery cliente;

  if (not cliente.exec("SELECT * FROM Cliente LEFT JOIN Endereco ON Cliente.idCliente = Endereco.idCliente "
                    "WHERE Endereco.idCliente = " +
                    idCliente + "")) {
    qDebug() << "Cliente query failed! : " << cliente.lastError();
    return false;
  }
  qDebug() << "cliente size: " << cliente.size();
  if (cliente.next()) {
    qDebug() << "Cliente next";

    if (cliente.value("nome_razao").toString().isEmpty()) {
      qDebug() << "nome_razao vazio";
      return false;
    }
    stream << "NomeRazao = " + cliente.value("nome_razao").toString() << endl;

    if (cliente.value("pfpj").toString() == "PF") {
      if (clearStr(cliente.value("cpf").toString()).isEmpty()) {
        qDebug() << "cpf vazio";
        return false;
      }
      stream << "CPF = " + clearStr(cliente.value("cpf").toString()) << endl;

      stream << "indIEDest = 9" << endl;
    }
    if (cliente.value("pfpj").toString() == "PJ") {
      if (clearStr(cliente.value("cnpj").toString()).isEmpty()) {
        qDebug() << "cnpj dest vazio";
        return false;
      }
      stream << "CNPJ = " + clearStr(cliente.value("cnpj").toString()) << endl;
      stream << "IE = " + clearStr(cliente.value("inscEstadual").toString()) << endl;
      if (cliente.value("inscEstadual").toString() == "ISENTO") {
        stream << "indIEDest = 2" << endl;
      }
    }

    if (cliente.value("tel").toString().isEmpty()) {
      qDebug() << "tel vazio";
      return false;
    }
    stream << "Fone = " + cliente.value("tel").toString() << endl;

    if (cliente.value("CEP").toString().isEmpty()) {
      qDebug() << "CEP vazio";
      return false;
    }
    stream << "CEP = " + clearStr(cliente.value("CEP").toString()) << endl;

    if (cliente.value("logradouro").toString().isEmpty()) {
      qDebug() << "logradouro vazio";
      return false;
    }
    stream << "Logradouro = " + cliente.value("logradouro").toString() << endl;

    if (cliente.value("numero").toString().isEmpty()) {
      qDebug() << "numero vazio";
      return false;
    }
    stream << "Numero = " + cliente.value("numero").toString() << endl;

    //    if(cliente.value("complemento").toString().isEmpty()){
    //      qDebug() << "complemento vazio";
    //      return false;
    //    }
    stream << "Complemento = " + cliente.value("complemento").toString() << endl;

    if (cliente.value("bairro").toString().isEmpty()) {
      qDebug() << "bairro vazio";
      return false;
    }
    stream << "Bairro = " + cliente.value("bairro").toString() << endl;

    if (cliente.value("cidade").toString().isEmpty()) {
      qDebug() << "cidade vazio";
      return false;
    }
    stream << "Cidade = " + cliente.value("cidade").toString() << endl;

    if (cliente.value("uf").toString().isEmpty()) {
      qDebug() << "uf vazio";
      return false;
    }
    stream << "UF = " + cliente.value("uf").toString() << endl;
  } else {
    qDebug() << "Erro buscando endereço do destinatário: " << cliente.lastError();
  }

  qDebug() << "[Produto]";
  double total = 0;
  double icmsTotal = 0;
  qDebug() << "idProduto: " << getFromProdModel(0, "idProduto").toString();
  for (int row = 0; row < modelItem.rowCount(); ++row) {
    QSqlQuery prod("SELECT * FROM Produto WHERE idProduto = '" +
                   getFromProdModel(row, "idProduto").toString() + "'");
    if (not prod.exec()) {
      qDebug() << "Erro buscando produtos: " << prod.lastError();
      return false;
    }

    prod.first();
    QString number = QString("%1").arg(row + 1, 3, 10, QChar('0')); // padding row with zeros
    stream << "[Produto" + number + "]" << endl;
    stream << "CFOP = " + modelItem.data(modelItem.index(row, modelItem.fieldIndex("cfop"))).toString() << endl;
    stream << "NCM = " + clearStr(modelItem.data(modelItem.index(row, modelItem.fieldIndex("ncm"))).toString()) << endl;

    if (prod.value("codBarras").toString().isEmpty()) {
      qDebug() << "codBarras vazio";
      return false;
    }
    stream << "Codigo = " + prod.value("codBarras").toString() << endl;

    if (prod.value("descricao").toString().isEmpty()) {
      qDebug() << "descricao vazio";
      return false;
    }
    stream << "Descricao = " + prod.value("descricao").toString() << endl;

    if (prod.value("un").toString().isEmpty()) {
      qDebug() << "un vazio";
      return false;
    }
    stream << "Unidade = " + prod.value("un").toString() << endl;

    if (getFromProdModel(row, "qte").toString().isEmpty()) {
      qDebug() << "qte vazio";
      return false;
    }
    stream << "Quantidade = " + getFromProdModel(row, "qte").toString() << endl;

    if (prod.value("precoVenda").toDouble() == 0) {
      qDebug() << "precoVenda = 0";
      return false;
    }
    double preco = prod.value("precoVenda").toDouble();
    double rounded_number = static_cast<double>(static_cast<int>(preco * 100 + 0.5)) / 100.0;
    stream << "ValorUnitario = " + QString::number(rounded_number) << endl;

    if (getFromProdModel(row, "parcial").toString().isEmpty()) {
      qDebug() << "parcial vazio";
      return false;
    }
    stream << "ValorTotal = " + getFromProdModel(row, "parcial").toString() << endl;

    total += getFromProdModel(row, "parcial").toDouble();

    //    if(cfop.endsWith("405")){
    //      stream << "[ICMS" + number + "]" << endl;
    //      stream << "CST = 60";
    //      stream << "ValorBase = 0";
    //      stream << "Aliquota = 0";
    //    }else{
    stream << "[ICMS" + number + "]" << endl;
    stream << "CST = 650" << endl;
    stream << "ValorBase = " + getFromProdModel(row, "parcial").toString() << endl;
    stream << "Aliquota = 18.00" << endl;
    //    }

    double icms = getFromProdModel(row, "parcial").toDouble() * 0.18;
    icmsTotal += icms;
    stream << "Valor = " + QString::number(icms) << endl;
  }

  qDebug() << "[Total]";
  stream << "[Total]" << endl;
  stream << "BaseICMS = " + QString::number(total) << endl;
  stream << "ValorICMS = " + QString::number(icmsTotal) << endl;
  stream << "ValorProduto = " + QString::number(total) << endl;
  stream << "ValorNota = " + QString::number(total) << endl;

  stream << "\"), [1]";

  stream.flush();
  file.close();
  return true;
}
