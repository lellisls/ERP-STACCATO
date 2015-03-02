#include <QDebug>
#include <QSqlError>
#include <QtCore>

#include "nfe.h"
#include "usersession.h"

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

QString clearStr(QString str) {
  return str.remove(".").remove("/").remove("-").remove(" ").remove("(").remove(")");
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

QString NFe::getArquivo() const {
  return arquivo;
}

QString NFe::getChaveAcesso() const {
  return chaveAcesso;
}
