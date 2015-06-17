#include "cadastrarnfe.h"
#include "cadastrocliente.h"
#include "endereco.h"
#include "mainwindow.h"
#include "ui_cadastrarnfe.h"
#include "usersession.h"

#include <QFile>
#include <QDate>
#include <QFileInfo>
#include <QInputDialog>

CadastrarNFE::CadastrarNFE(QString idVenda, QWidget *parent)
  : QDialog(parent), ui(new Ui::CadastrarNFE), idVenda(idVenda) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  SearchDialog *sdCliente = SearchDialog::cliente(ui->itemBoxCliente);
  ui->itemBoxCliente->setSearchDialog(sdCliente);

  SearchDialog *sdEndereco = SearchDialog::enderecoCliente(ui->itemBoxEndereco);
  ui->itemBoxEndereco->setSearchDialog(sdEndereco);

  RegisterDialog *cadCliente = new CadastroCliente(this);
  ui->itemBoxCliente->setRegisterDialog(cadCliente);

  modelNFe.setTable("NFe");
  modelNFe.setEditStrategy(EditableSqlModel::OnManualSubmit);

  if (not modelNFe.select()) {
    qDebug() << "erro modelNFe: " << modelNFe.lastError();
    return;
  }

  modelNFeItem.setTable("NFe_has_Itens");
  modelNFeItem.setEditStrategy(EditableSqlModel::OnManualSubmit);

  if (not modelNFeItem.select()) {
    qDebug() << "erro modelNFeItem: " << modelNFeItem.lastError();
    return;
  }

  ui->tableItens->setModel(&modelNFeItem);

  mapperNFe.setModel(&modelNFe);
  mapperNFe.addMapping(ui->itemBoxCliente, modelNFe.fieldIndex("idCliente"));
  mapperNFe.addMapping(ui->itemBoxEndereco, modelNFe.fieldIndex("idEnderecoFaturamento"));
  mapperNFe.addMapping(ui->doubleSpinBoxFrete, modelNFe.fieldIndex("frete"));
  mapperNFe.addMapping(ui->doubleSpinBoxFinal, modelNFe.fieldIndex("total"));
  mapperNFe.addMapping(ui->textEdit, modelNFe.fieldIndex("obs"));
  mapperNFe.toLast();

  connect(ui->tableItens->model(), &QAbstractItemModel::dataChanged, this, &CadastrarNFE::onDataChanged);

  modelLoja.setTable("Loja");
  modelLoja.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelLoja.setFilter("idLoja = " + QString::number(UserSession::getLoja()) + "");

  if (not modelLoja.select()) {
    qDebug() << "erro modelLoja: " << modelLoja.lastError();
    return;
  }

  modelVenda.setTable("Venda");
  modelVenda.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelVenda.setFilter("idVenda = '" + idVenda + "'");

  if (not modelVenda.select()) {
    qDebug() << "erro modelVenda: " << modelVenda.lastError();
    return;
  }

  modelProd.setTable("Venda_has_Produto");
  modelProd.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelProd.setFilter("idVenda = '" + idVenda + "'");

  if (not modelProd.select()) {
    qDebug() << "erro modelProd: " << modelProd.lastError();
    return;
  }
}

CadastrarNFE::~CadastrarNFE() { delete ui; }

void CadastrarNFE::guardarNotaBD() {
  QString arquivo = getFromLoja("pastaXmlACBr").toString() + chaveNum + "-nfe.xml";
  qDebug() << "arquivo: " << arquivo;
  QFile file(arquivo);

  bool espera = true;

  while (espera) {
    if (file.exists()) {
      qDebug() << "file found!";

      QSqlQuery qryNota;
      // TODO: frete e total devem ser substituidos pelos valores na nota
      if (not qryNota.exec(
            "INSERT INTO NFe (idVenda, idLoja, idCliente, "
            "idEnderecoFaturamento, NFe, status, chaveAcesso, "
            "obs, frete, total) VALUES ('" +
            idVenda + "', " + getFromLoja("idLoja").toString() + ", " + getFromVenda("idCliente").toString() + ", " +
            getFromVenda("idEnderecoFaturamento").toString() + ", load_file('" + arquivo + "'), '', '" + chaveNum +
            "', '', " + getFromVenda("frete").toString() + ", " + getFromVenda("total").toString() + ")")) {
        qDebug() << "Erro guardando nota: " << qryNota.lastError();
      } else {
        qDebug() << "Nota guardada com sucesso!";
        QMessageBox::information(this, "Aviso!", "Nota guardada com sucesso.");
      }

      espera = false;
    } else {
      qDebug() << ":`(";
      // TODO: caso não encontre o arquivo, guarde os outros dados e posteriormente na tela de nfe pesquisa pelo arquivo
      QMessageBox msgBox(QMessageBox::Warning, "Atenção!", "Esperar ACBr?", QMessageBox::Yes | QMessageBox::No, this);
      msgBox.setButtonText(QMessageBox::Yes, "Sim");
      msgBox.setButtonText(QMessageBox::No, "Não");

      if (msgBox.exec() == QMessageBox::Yes) {
        espera = true;
      } else {
        espera = false;
      }
    }
  }
}

void CadastrarNFE::on_pushButtonGerarNFE_clicked() {
  if (writeTXT()) {
    QMessageBox::information(this, "Aviso!", "NFe gerada com sucesso!");

    guardarNotaBD();

  } else {
    QMessageBox::warning(this, "Aviso!", "Ocorreu algum erro, NFe não foi gerada.");
  }
}

void CadastrarNFE::on_pushButtonCancelar_clicked() { close(); }

void CadastrarNFE::updateImpostos() {
  double icms = 0;

  for (int row = 0; row < modelNFeItem.rowCount(); ++row) {
    icms += getItemData(row, "valorICMS").toDouble();
  }

  ui->doubleSpinBoxVlICMS->setValue(icms);
  double imposto = 0.593 * ui->doubleSpinBoxFinal->value() + icms;
  Endereco end(ui->itemBoxEndereco->value().toInt(), "Cliente_has_Endereco");
  QString texto = "Venda de código " +
                  modelNFe.data(modelNFe.index(mapperNFe.currentIndex(), modelNFe.fieldIndex("idVenda"))).toString() +
                  "\n" + "END. ENTREGA: " + end.umaLinha() + "\n" +
                  "Informações Adicionais de Interesse do Fisco: ICMS RECOLHIDO "
                  "ANTECIPADAMENTE CONFORME ARTIGO 3113Y\n" +
                  "Total aproximado de tributos federais, estaduais e municipais: " + QString::number(imposto);
  ui->textEdit->setPlainText(texto);
  modelNFe.setData(modelNFe.index(mapperNFe.currentIndex(), modelNFe.fieldIndex("obs")), texto);
}

void CadastrarNFE::setItemData(int row, const QString &key, const QVariant &value) {
  if (modelNFeItem.fieldIndex(key) == -1) {
    qDebug() << "A chave '" + key + "' não existe na tabela de Itens da NFe";
  }

  modelNFeItem.setData(modelNFeItem.index(row, modelNFeItem.fieldIndex(key)), value);
}

QVariant CadastrarNFE::getItemData(int row, const QString &key) {
  if (modelNFeItem.fieldIndex(key) == -1) {
    qDebug() << "A chave '" + key + "' não existe na tabela de Itens da NFe";
  }

  return (modelNFeItem.data(modelNFeItem.index(row, modelNFeItem.fieldIndex(key))));
}

void CadastrarNFE::prepararNFe(QList<int> items) {
  QSqlQuery qryVenda("SELECT * FROM Venda WHERE idVenda = '" + idVenda + "'");

  if (not qryVenda.first()) {
    qDebug() << "Erro lendo itens da venda. ERRO: " << qryVenda.lastError();
  }

  if (not modelNFe.select()) {
    qDebug() << "erro modelNFe: " << modelNFe.lastError();
    return;
  }

  int row = modelNFe.rowCount();
  modelNFe.insertRow(row);
  mapperNFe.toLast();

  modelNFe.setData(modelNFe.index(mapperNFe.currentIndex(), modelNFe.fieldIndex("idVenda")), idVenda);
  modelNFe.setData(modelNFe.index(mapperNFe.currentIndex(), modelNFe.fieldIndex("frete")), qryVenda.value("frete"));
  modelNFe.setData(modelNFe.index(mapperNFe.currentIndex(), modelNFe.fieldIndex("total")), qryVenda.value("total"));
  modelNFe.setData(modelNFe.index(mapperNFe.currentIndex(), modelNFe.fieldIndex("idLoja")), qryVenda.value("idLoja"));
  modelNFe.setData(modelNFe.index(mapperNFe.currentIndex(), modelNFe.fieldIndex("idCliente")),
                   qryVenda.value("idCliente"));
  modelNFe.setData(modelNFe.index(mapperNFe.currentIndex(), modelNFe.fieldIndex("idEnderecoFaturamento")),
                   qryVenda.value("idEnderecoFaturamento"));

  ui->doubleSpinBoxFinal->setValue(qryVenda.value("total").toDouble());
  ui->doubleSpinBoxFrete->setValue(qryVenda.value("frete").toDouble());
  ui->itemBoxCliente->setValue(qryVenda.value("idCliente"));
  ui->itemBoxEndereco->searchDialog()->setFilter("idCliente = " + QString::number(ui->itemBoxCliente->value().toInt()) +
                                                 " AND desativado = false");
  ui->itemBoxEndereco->setValue(qryVenda.value("idEnderecoFaturamento"));

  double descontoGlobal = qryVenda.value("descontoPorc").toDouble();

  for (auto item : items) {
    QSqlQuery qryItens("SELECT * FROM Venda_has_Produto NATURAL LEFT JOIN "
                       "Produto WHERE idVenda = '" +
                       idVenda + "' AND item = '" + QString::number(item) + "'");

    if (not qryItens.exec() or not qryItens.first()) {
      qDebug() << "Erro buscando produto: " << qryItens.lastError();
      qDebug() << "Last query: " << qryItens.lastQuery();
      continue;
    }

    int row = modelNFeItem.rowCount();
    modelNFeItem.insertRow(row);

    setItemData(row, "item", row + 1);
    setItemData(row, "cst", "060"); // FIXME: query this from produto
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

  ui->tableItens->resizeColumnsToContents();
  updateImpostos();
}

void CadastrarNFE::on_tableItens_activated(const QModelIndex &index) {
  Q_UNUSED(index);

  updateImpostos();
}

void CadastrarNFE::on_tableItens_pressed(const QModelIndex &index) {
  Q_UNUSED(index);

  updateImpostos();
}

QString CadastrarNFE::criarChaveAcesso() {
  QSqlQuery query;

  if (not query.exec("SELECT idNFe FROM NFe ORDER BY idNFe DESC LIMIT 1")) {
    qDebug() << "Erro buscando idNFe: " << query.lastError();
  }

  int id;

  if (query.first()) {
    id = query.value("idNFe").toInt() + 1;
  } else {
    id = 1;
  }

  if (id > 999999999) {
    id = 1;
  }

  QStringList listChave;

  QSqlQuery endLoja("SELECT * FROM Loja_has_Endereco WHERE idLoja = " + QString::number(UserSession::getLoja()) + "");

  if (not endLoja.exec()) {
    qDebug() << "End. loja failed! : " << endLoja.lastError();
    return QString();
  }

  endLoja.first();
  listChave.push_back(endLoja.value("uf").toString()); // cUF - código da UF

  listChave.push_back(QDate::currentDate().toString("yyMM")); // Ano/Mês

  QString cnpj = clearStr(getFromLoja("cnpj").toString());
  listChave.push_back(cnpj); // CNPJ

  listChave.push_back("55"); // modelo

  QString serie = QString::number(QInputDialog::getInt(this, "Série", "Digite a série da nota: ", 1));
  serie = QString("%1").arg(serie.toInt(), 3, 10, QChar('0'));

  listChave.push_back(serie);                                    // série
  listChave.push_back(QString("%1").arg(id, 9, 10, QChar('0'))); // número nf-e (id interno)
  listChave.push_back("1");                                      // tpEmis - forma de emissão
  listChave.push_back("00000001");                               // código númerico aleatorio

  QString chave = listChave.join("");

  return chave;
}

QString CadastrarNFE::clearStr(QString str) {
  return str.remove(".").remove("/").remove("-").remove(" ").remove("(").remove(")");
}

void CadastrarNFE::onDataChanged(const QModelIndex &topLeft,
                                 const QModelIndex &bottomRight) { // TODO: finish implementation
  if (topLeft == bottomRight) {                                    // only one cell changed
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
        modelNFeItem.setData(topLeft, 100);
      }

      double icms =
          modelNFeItem.data(modelNFeItem.index(topLeft.row(), modelNFeItem.fieldIndex("valorTotal"))).toDouble() *
          (topLeft.data().toDouble() / 100);
      qDebug() << "setting icms: "
               << modelNFeItem.setData(modelNFeItem.index(topLeft.row(), modelNFeItem.fieldIndex("valorICMS")), icms);
    }
    if (topLeft.column() == 15) { // aliquotaIPI
      if (topLeft.data().toInt() > 100) {
        qDebug() << "bigger than 100! limit.";
        modelNFeItem.setData(topLeft, 100);
      }

      double icms =
          modelNFeItem.data(modelNFeItem.index(topLeft.row(), modelNFeItem.fieldIndex("valorTotal"))).toDouble() *
          (topLeft.data().toDouble() / 100);
      qDebug() << "setting icms: "
               << modelNFeItem.setData(modelNFeItem.index(topLeft.row(), modelNFeItem.fieldIndex("valorIPI")), icms);
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
  return (modelNFeItem.data(modelNFeItem.index(row, modelNFeItem.fieldIndex(column))));
}

QVariant CadastrarNFE::getFromProdModel(int row, QString column) {
  return (modelProd.data(modelProd.index(row, modelProd.fieldIndex(column))));
}

QString CadastrarNFE::calculaDigitoVerificador(QString chave) {
  if (chave.size() != 43) {
    qDebug() << "Erro na chave: " << chave;
    return QString();
  }

  QVector<int> chave2;
  for (int i = 0; i < chave.size(); ++i) {
    chave2.push_back(chave.at(i).digitValue());
  }

  QVector<int> multiplicadores = {4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2, 9, 8, 7,
                                  6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2};
  int soma = 0;

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

  return QString::number(cDV);
}

void CadastrarNFE::writeIdentificacao(QTextStream &stream) {
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
}

bool CadastrarNFE::writeEmitente(QTextStream &stream) {
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

  QSqlQuery endLoja("SELECT * FROM Loja_has_Endereco WHERE idLoja = " + QString::number(UserSession::getLoja()) + "");

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

  return true;
}

bool CadastrarNFE::writeDestinatario(QTextStream &stream) {
  stream << "[Destinatario]" << endl;

  QString idCliente = getFromVenda("idCliente").toString();

  if (idCliente.isEmpty()) {
    qDebug() << "idCliente vazio";
    return false;
  }

  QSqlQuery cliente;

  if (not cliente.exec("SELECT * FROM Cliente LEFT JOIN Cliente_has_Endereco "
                       "ON Cliente.idCliente = "
                       "Cliente_has_Endereco.idCliente "
                       "WHERE Cliente_has_Endereco.idCliente = " +
                       idCliente + "")) {
    qDebug() << "Cliente query failed! : " << cliente.lastError();
    return false;
  }

  //  qDebug() << "cliente size: " << cliente.size();

  if (cliente.next()) {
    //    qDebug() << "Cliente next";

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

  return true;
}

bool CadastrarNFE::writeProduto(QTextStream &stream, double &total, double &icmsTotal) {
  //  qDebug() << "idProduto: " << getFromProdModel(0, "idProduto").toString();

  for (int row = 0; row < modelNFeItem.rowCount(); ++row) {
    QSqlQuery prod("SELECT * FROM Produto WHERE idProduto = '" + getFromProdModel(row, "idProduto").toString() + "'");

    if (not prod.exec()) {
      qDebug() << "Erro buscando produtos: " << prod.lastError();
      return false;
    }

    prod.first();
    QString number = QString("%1").arg(row + 1, 3, 10, QChar('0')); // padding row with zeros
    stream << "[Produto" + number + "]" << endl;
    stream << "CFOP = " + modelNFeItem.data(modelNFeItem.index(row, modelNFeItem.fieldIndex("cfop"))).toString()
           << endl;
    stream << "NCM = " + clearStr(modelNFeItem.data(modelNFeItem.index(row, modelNFeItem.fieldIndex("ncm"))).toString())
           << endl;

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

    stream << "[ICMS" + number + "]" << endl;
    stream << "CST = 650" << endl;
    stream << "ValorBase = " + getFromProdModel(row, "parcial").toString() << endl;
    stream << "Aliquota = 18.00" << endl;

    double icms = getFromProdModel(row, "parcial").toDouble() * 0.18;
    icmsTotal += icms;
    stream << "Valor = " + QString::number(icms) << endl;
  }

  return true;
}

void CadastrarNFE::writeTotal(QTextStream &stream, double &total, double &icmsTotal) {
  stream << "[Total]" << endl;
  stream << "BaseICMS = " + QString::number(total) << endl;
  stream << "ValorICMS = " + QString::number(icmsTotal) << endl;
  stream << "ValorProduto = " + QString::number(total) << endl;
  stream << "ValorNota = " + QString::number(total) << endl;
}

bool CadastrarNFE::writeTXT() {
  QString chave = criarChaveAcesso();

  chave += calculaDigitoVerificador(chave);

  chaveNum = chave;
  chaveAcesso = "NFe" + chave;

  QString dir = getFromLoja("pastaEntACBr").toString();
  QFile file(dir + chaveNum + ".txt");

  if (not file.open(QFile::WriteOnly)) {
    QMessageBox::warning(0, "Aviso!", "Não foi possível criar a nota na pasta do ACBr, favor verificar se as "
                                      "pastas estão corretamente configuradas.");
    return false;
  }

  QFileInfo fileInfo(file);
  arquivo = fileInfo.absoluteFilePath().replace("/", "\\\\");

  QTextStream stream(&file);

  stream << "NFe.CriarEnviarNFe(\"" << endl;

  writeIdentificacao(stream);

  writeEmitente(stream);

  writeDestinatario(stream);

  double total = 0.0;
  double icmsTotal = 0.0;

  writeProduto(stream, total, icmsTotal);

  writeTotal(stream, total, icmsTotal);

  stream << "\",1,1)";

  stream.flush();
  file.close();
  return true;
}
