#include <QDebug>
#include <QSqlError>
#include <QtCore>

#include "nfe.h"
#include "usersession.h"

QString clearStr(QString str) {
  return str.remove(".").remove("/").remove("-").remove(" ").remove("(").remove(")");
}

NFe::NFe(QString idVenda, QObject *parent) : QObject(parent), idVenda(idVenda) {
  modelVenda.setTable("Venda");
  modelVenda.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelVenda.setFilter("idVenda = '" + idVenda + "'");
  if (!modelVenda.select() || modelVenda.rowCount() == 0) {
    qDebug() << "Failed to populate venda!" << modelVenda.lastError();
  }

  modelLoja.setTable("Loja");
  modelLoja.setEditStrategy(QSqlTableModel::OnManualSubmit);
  QString idLoja = QString::number(UserSession::getLoja());
  modelLoja.setFilter("idLoja = '" + idLoja + "'");
  if (!modelLoja.select() || modelLoja.rowCount() == 0) {
    qDebug() << "Failed to populate loja!" << modelLoja.lastError();
  }

  modelItem.setTable("Venda_has_Produto");
  modelItem.setFilter("idVenda = '" + idVenda + "'");
  modelItem.setEditStrategy(QSqlTableModel::OnManualSubmit);
  if (!modelItem.select() || modelItem.rowCount() == 0) {
    qDebug() << "Failed to populate itemModel!" << modelItem.lastError();
  }
}

QVariant NFe::getFromVenda(QString column) {
  return (modelVenda.data(modelVenda.index(0, modelVenda.fieldIndex(column))));
}

QVariant NFe::getFromLoja(QString column) {
  return (modelLoja.data(modelLoja.index(0, modelLoja.fieldIndex(column))));
}

QVariant NFe::getFromItemModel(int row, QString column) {
  return (modelItem.data(modelItem.index(row, modelItem.fieldIndex(column))));
}

QString NFe::criarChaveAcesso() {
  QVector<QString> vetorChave;
  vetorChave.push_back("35");                                  // cUF - código da UF
  vetorChave.push_back(QDate::currentDate().toString("yyMM")); // Ano/Mês
  QString cnpj = clearStr(getFromLoja("cnpj").toString());
  vetorChave.push_back(cnpj);        // CNPJ
  vetorChave.push_back("55");        // modelo
  vetorChave.push_back("001");       // série
  vetorChave.push_back("123456789"); // número nf-e
  vetorChave.push_back("1");         // tpEmis - forma de emissão
  vetorChave.push_back("76543210");  // código númerico
  //  qDebug() << "chave: " << vetorChave;

  QStringList listChave = vetorChave.toList();
  QString chave = listChave.join("");

  return chave;
}

bool NFe::XML() {
  QString chave = criarChaveAcesso();

  //  int cDV = calculaDigitoVerificador(chave);

  chave += calculaDigitoVerificador(chave);
  //  qDebug() << "cDV: " << cDV;
  //  qDebug() << "chave: " << chave;

  //  if (cDV != -1) {
  //  writeXML(chave, cDV);
  //  }

  return true;
}

bool NFe::TXT() {
  QString chave = criarChaveAcesso();

  //  int cDV = calculaDigitoVerificador(chave);

  chave += calculaDigitoVerificador(chave);
  //  qDebug() << "cDV: " << cDV;
  //  qDebug() << "chave: " << chave;

  //  if (cDV != -1) {
  writeTXT(chave);
  //  }

  return true;
}

NFe::~NFe() {}

void NFe::writeTXT(QString chave) {
  QFile file(idVenda + ".txt");
  qDebug() << QDir::current().absoluteFilePath(idVenda + ".txt");
  file.open(QFile::WriteOnly);
  QFileInfo fileInfo(file);
  arquivo = fileInfo.absoluteFilePath().replace("/", "\\\\");

  chaveAcesso = "NFe" + chave;
  //  qDebug() << chaveAcesso;
  //  qDebug() << chaveAcesso.mid(38, 8);
  //  qDebug() << chaveAcesso.mid(25, 3);
  //  qDebug() << chaveAcesso.mid(28, 9);

  QTextStream stream(&file);

  stream << "NFe.CriarNFe(\"" << endl;

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
  stream << "CNPJ = " + clearStr(getFromLoja("cnpj").toString()) << endl;
  //  stream << "IE = " + getFromLoja("inscEstadual").toString() << endl;
  stream << "IE = 110042490114" << endl;
  stream << "Razao = " + getFromLoja("razaoSocial").toString() << endl;
  stream << "Fantasia = " + getFromLoja("nomeFantasia").toString() << endl;
  stream << "Fone = " + getFromLoja("tel").toString() << endl;

  QString idEndLoja = getFromLoja("idEndereco").toString();
  QSqlQuery endLoja("SELECT * FROM Endereco WHERE idEndereco = '" + idEndLoja + "'");
  if (!endLoja.exec()) {
    qDebug() << "End. loja failed! : " << endLoja.lastError();
  }
  endLoja.first();

  stream << "CEP = " + clearStr(endLoja.value("CEP").toString()) << endl;
  stream << "Logradouro = " + endLoja.value("logradouro").toString() << endl;
  stream << "Numero = " + endLoja.value("numero").toString() << endl;
  stream << "Complemento = " + endLoja.value("complemento").toString() << endl;
  stream << "Bairro = " + endLoja.value("bairro").toString() << endl;
  stream << "Cidade = " + endLoja.value("cidade").toString() << endl;
  stream << "UF = " + endLoja.value("uf").toString() << endl;

  qDebug() << "[Destinatario]";
  stream << "[Destinatario]" << endl;

  QString idCliente = getFromVenda("idCadastroCliente").toString();
  QSqlQuery cliente("SELECT * FROM Cadastro LEFT JOIN Cadastro_has_Endereco ON Cadastro.idCadastro = "
                    "Cadastro_has_Endereco.idCadastro LEFT JOIN Endereco ON Cadastro_has_Endereco.idEndereco "
                    "= Endereco.idEndereco WHERE Cadastro.idCadastro = '" +
                    idCliente + "'");
  if (!cliente.exec()) {
    qDebug() << "Cliente query failed! : " << cliente.lastError();
  }
  cliente.first();

  if (cliente.value("pfpj").toString() == "PF") {
    stream << "CPF = " + clearStr(cliente.value("cpf").toString()) << endl;
  } else {
    stream << "CNPJ = " + clearStr(cliente.value("cnpj").toString()) << endl;
    stream << "IE = 110042490114" << endl;
    //  stream << "IE = " + cliente.value("inscEstadual").toString() << endl;
    //  stream << "IE = ISENTO" << endl;
  }
  stream << "NomeRazao = " + cliente.value("razaoSocial").toString() << endl;
  stream << "Fone = " + cliente.value("tel").toString() << endl;
  stream << "CEP = " + cliente.value("CEP").toString() << endl;
  stream << "Logradouro = " + cliente.value("logradouro").toString() << endl;
  stream << "Numero = " + cliente.value("numero").toString() << endl;
  stream << "Complemento = " + cliente.value("complemento").toString() << endl;
  stream << "Bairro = " + cliente.value("bairro").toString() << endl;
  stream << "Cidade = " + cliente.value("cidade").toString() << endl;
  stream << "UF = " + cliente.value("uf").toString() << endl;

  qDebug() << "[Produto]";
  double total = 0;
  double icmsTotal = 0;
  for (int row = 0; row < modelItem.rowCount(); ++row) {
    QSqlQuery prod("SELECT * FROM Produto WHERE idProduto = '" +
                   getFromItemModel(row, "idProduto").toString() + "'");
    prod.exec();
    prod.first();
    QString number = QString("%1").arg(row + 1, 3, 10, QChar('0'));
    stream << "[Produto" + number + "]" << endl;
    //  stream << "CFOP = " + prod.value("cfop").toString() << endl;
    stream << "CFOP = 5105" << endl;
    stream << "NCM = 40169100" << endl;
    stream << "Codigo = " + prod.value("codBarras").toString() << endl;
    stream << "Descricao = " + prod.value("descricao").toString() << endl;
    stream << "Unidade = " + prod.value("un").toString() << endl;
    stream << "Quantidade = " + getFromItemModel(row, "qte").toString() << endl;

    double preco = prod.value("precoVenda").toDouble();
    double rounded_number = static_cast<double>(static_cast<int>(preco * 100 + 0.5)) / 100.0;
    stream << "ValorUnitario = " + QString::number(rounded_number) << endl;
    stream << "ValorTotal = " + getFromItemModel(row, "parcial").toString() << endl;
    total += getFromItemModel(row, "parcial").toDouble();

    stream << "[ICMS" + number + "]" << endl;
    stream << "CST = 00" << endl;
    stream << "ValorBase = " + getFromItemModel(row, "parcial").toString() << endl;
    stream << "Aliquota = 18.00" << endl;

    double icms = getFromItemModel(row, "parcial").toDouble() * 0.18;
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
}

void NFe::writeXML(QString chave, int cDV) {
  QFile file(idVenda + "-nfe.xml");
  qDebug() << QDir::current().absoluteFilePath(idVenda + "-nfe.xml");
  file.open(QFile::WriteOnly);
  QFileInfo fileInfo(file);
  arquivo = fileInfo.absoluteFilePath().replace("/", "\\\\");

  chaveAcesso = "NFe" + chave;
  //  qDebug() << chaveAcesso;
  //  qDebug() << chaveAcesso.mid(38, 8);
  //  qDebug() << chaveAcesso.mid(25, 3);
  //  qDebug() << chaveAcesso.mid(28, 9);

  QXmlStreamWriter stream(&file);
  stream.setAutoFormatting(true);
  stream.writeStartDocument();

  stream.writeStartElement("enviNFe");
  stream.writeAttribute("xmlns", "http://www.portalfiscal.inf.br/nfe");
  stream.writeAttribute("versao", "2.00");
  stream.writeTextElement("idLote", "33");
  stream.writeStartElement("NFe");
  stream.writeStartElement("infNFe");
  stream.writeAttribute("Id", chaveAcesso); // 42 1012 08187168000160 55 100 100000003 1 99999996 7
  stream.writeAttribute("versao", "2.00");
  stream.writeStartElement("ide");
  stream.writeTextElement("cUF", getFromLoja("codUF").toString());
  stream.writeTextElement("cNF", chaveAcesso.mid(38, 8));
  stream.writeTextElement("natOp", "5405-VENDA PROD/SERV  D.ESTADO");
  stream.writeTextElement("indPag", "1");
  stream.writeTextElement("mod", "55");
  stream.writeTextElement("serie", chaveAcesso.mid(25, 3));
  stream.writeTextElement("nNF", chaveAcesso.mid(28, 9));
  stream.writeTextElement("dEmi", QDate::currentDate().toString("yyyy-MM-dd"));
  stream.writeTextElement("dSaiEnt", QDate::currentDate().toString("yyyy-MM-dd"));
  stream.writeTextElement("tpNF", "1");
  stream.writeTextElement("cMunFG", "4216602");
  stream.writeTextElement("tpImp", "1");
  stream.writeTextElement("tpEmis", "1");
  stream.writeTextElement("cDV", QString::number(cDV));
  stream.writeTextElement("tpAmb", "2");
  stream.writeTextElement("finNFe", "1");
  stream.writeTextElement("procEmi", "0");
  stream.writeTextElement("verProc", "1");
  stream.writeEndElement(); // ide
  stream.writeStartElement("emit");

  stream.writeTextElement("CNPJ", clearStr(getFromLoja("cnpj").toString()));
  stream.writeTextElement("xNome", getFromLoja("razaoSocial").toString());
  //                          modelLoja.data(modelLoja.index(0,
  //                          modelLoja.fieldIndex("razaoSocial"))).toString());
  stream.writeTextElement("xFant",
                          getFromLoja("nomeFantasia").toString()); // modelLoja.data(modelLoja.index(0,
  // modelLoja.fieldIndex("nomeFantasia"))).toString());
  QString idEndLoja = getFromLoja("idEndereco").toString();
  QSqlQuery endLoja("SELECT * FROM Endereco WHERE idEndereco = '" + idEndLoja + "'");
  endLoja.exec();
  endLoja.first();
  stream.writeStartElement("enderEmit");
  stream.writeTextElement("xLgr", endLoja.value("logradouro").toString());
  stream.writeTextElement("nro", endLoja.value("numero").toString());
  stream.writeTextElement("xBairro", endLoja.value("bairro").toString());
  stream.writeTextElement("cMun", "4216602"); // TODO Buscar codigo municipal!
  stream.writeTextElement("xMun", endLoja.value("cidade").toString());
  stream.writeTextElement("UF", endLoja.value("uf").toString());
  stream.writeTextElement("CEP", clearStr(endLoja.value("CEP").toString()));
  stream.writeTextElement("cPais", "1058");
  stream.writeTextElement("xPais", "BRASIL");
  stream.writeTextElement("fone", clearStr(getFromLoja("tel").toString()));
  stream.writeEndElement(); // enderEmit
  stream.writeTextElement("IE", clearStr(getFromLoja("inscEstadual").toString()));
  stream.writeTextElement("CRT", "1");
  stream.writeEndElement(); // emit
  stream.writeStartElement("dest");

  QString idCliente = getFromVenda("idCadastroCliente").toString();
  QSqlQuery cliente(
        "SELECT * FROM Cadastro LEFT JOIN Endereco ON idEnderecoFaturamento = idEndereco WHERE idCadastro = '" +
        idCliente + "'");
  cliente.exec();
  cliente.first();

  stream.writeTextElement("CNPJ", clearStr(cliente.value("cnpj").toString()));
  stream.writeTextElement("xNome", cliente.value("razaoSocial").toString());
  stream.writeStartElement("enderDest");
  //  int idEndDestino = modelVenda.data(modelVenda.fieldIndex())
  //  QSqlQuery endDest("SELECT * FROM Endereco WHERE idEndereco = " + QString::number(idEndDestino));
  stream.writeTextElement("xLgr", cliente.value("logradouro").toString());
  stream.writeTextElement("nro", cliente.value("numero").toString());
  stream.writeTextElement("xBairro", cliente.value("bairro").toString());
  stream.writeTextElement("cMun", "4216602"); // TODO Buscar codigo municipal!
  stream.writeTextElement("xMun", cliente.value("cidade").toString());
  stream.writeTextElement("UF", cliente.value("uf").toString());
  stream.writeTextElement("CEP", clearStr(cliente.value("CEP").toString()));
  stream.writeTextElement("cPais", "1058");
  stream.writeTextElement("xPais", "BRASIL");
  stream.writeTextElement("fone", clearStr(cliente.value("tel").toString()));
  stream.writeEndElement(); // enderDest
  stream.writeTextElement("IE", clearStr(cliente.value("inscEstadual").toString()));
  stream.writeEndElement(); // dest

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    QSqlQuery prod("SELECT * FROM Produto WHERE idProduto = '" +
                   getFromItemModel(row, "idProduto").toString() + "'");
    prod.exec();
    prod.first();

    stream.writeStartElement("det");
    stream.writeAttribute("nItem", QString::number(row + 1));
    stream.writeStartElement("prod");
    stream.writeTextElement("cProd", getFromItemModel(row, "idProduto").toString());
    stream.writeStartElement("cEAN");
    stream.writeEndElement(); // cEAN
    stream.writeTextElement("xProd", getFromItemModel(row, "produto").toString());
    stream.writeTextElement("NCM", prod.value("ncm").toString());
    stream.writeTextElement("CFOP", prod.value("cfop").toString());
    stream.writeTextElement("uCom", prod.value("un").toString());
    stream.writeTextElement("qCom", getFromItemModel(row, "qte").toString());
    double desc = getFromItemModel(row, "desconto").toDouble();
    double prcUn = getFromItemModel(row, "prcUnitario").toDouble();
    double unCx = getFromItemModel(row, "unCaixa").toDouble();
    double prcUnNf = (prcUn / unCx) * (1.0 - desc);
    // TODO Verificar validade do cálculo, talvez tenha que calcular desconto total da nota.
    stream.writeTextElement("vUnCom", QString::number(prcUnNf));
    stream.writeTextElement("vProd", getFromItemModel(row, "parcialDesc").toString());
    stream.writeStartElement("cEANTrib");
    stream.writeEndElement(); // cEANTrib
    stream.writeTextElement("uTrib", "Un");
    stream.writeTextElement("qTrib", "1.0000");
    stream.writeTextElement("vUnTrib", "10.0000");
    stream.writeTextElement("indTot", "1");
    stream.writeEndElement(); // prod
    stream.writeStartElement("imposto");
    stream.writeStartElement("ICMS");
    stream.writeStartElement("ICMSSN102");
    stream.writeTextElement("orig", "0");
    stream.writeTextElement("CSOSN", "400");
    stream.writeEndElement(); // ICMSSN102
    stream.writeEndElement(); // ICMS
    stream.writeStartElement("PIS");
    stream.writeStartElement("PISNT");
    stream.writeTextElement("CST", "07");
    stream.writeEndElement(); // PISNT
    stream.writeEndElement(); // PIS
    stream.writeStartElement("COFINS");
    stream.writeStartElement("COFINSNT");
    stream.writeTextElement("CST", "07");
    stream.writeEndElement(); // COFINSNT
    stream.writeEndElement(); // COFINS
    stream.writeEndElement(); // imposto
    stream.writeEndElement(); // det
  }
  stream.writeStartElement("total");
  stream.writeStartElement("ICMSTot");
  stream.writeTextElement("vBC", "0.00");
  stream.writeTextElement("vICMS", "0.00");
  stream.writeTextElement("vBCST", "0.00");
  stream.writeTextElement("vST", "0.00");
  stream.writeTextElement("vProd", "10.00");
  stream.writeTextElement("vFrete", "0.00");
  stream.writeTextElement("vSeg", "0.00");
  stream.writeTextElement("vDesc", "0.00");
  stream.writeTextElement("vII", "0.00");
  stream.writeTextElement("vIPI", "0.00");
  stream.writeTextElement("vPIS", "0.00");
  stream.writeTextElement("vCOFINS", "0.00");
  stream.writeTextElement("vOutro", "0.00");
  stream.writeTextElement("vNF", "10.00");
  stream.writeEndElement(); // ICMSTot
  stream.writeEndElement(); // total
  stream.writeStartElement("transp");
  stream.writeTextElement("modFrete", "1");
  stream.writeEndElement(); // transp
  stream.writeStartElement("cobr");
  stream.writeEndElement(); // cobr
  stream.writeStartElement("infAdic");
  stream.writeTextElement("infCpl", "Docto emitido por ME ou EPP optante pelo Simples Nacional Nao gera "
                                    "direito a Credito Fiscal de ICMS e de ISS. SEM VALOR FISCAL");
  stream.writeEndElement(); // infAdic
  stream.writeEndElement(); // infNFe
  stream.writeEndElement(); // NFe
  stream.writeEndElement();

  //    stream.writeEndDocument();
}

QString NFe::calculaDigitoVerificador(QString chave) {
  if (chave.size() != 43) {
    qDebug() << "Erro na chave!";
    return QString();
  }

  QVector<int> chave2;
  for (int i = 0; i < chave.size(); ++i) {
    chave2.push_back(chave.at(i).digitValue());
  }

  QVector<int> multiplicadores = {4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2, 9, 8, 7,
                                  6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2
                                 };
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

bool NFe::assinaXML() {
  QProcess process(this);
  QStringList args;
  args << "nfephp/assinaxml.php" << idVenda + "-nfe.xml";
  process.start("nfephp", args);
  if (!process.waitForStarted()) {
    return false;
  }

  if (!process.waitForFinished()) {
    return false;
  }

  QByteArray result = process.readAll();
  return true;
}

QString NFe::getArquivo() const {
  return arquivo;
}

QString NFe::getChaveAcesso() const {
  return chaveAcesso;
}
