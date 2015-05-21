#include <QDebug>
#include <QSqlError>
#include <QtCore>
#include <QInputDialog>
#include <QMessageBox>

#include "nfe.h"
#include "usersession.h"

NFe::NFe(QString idVenda, QObject *parent) : QObject(parent), idVenda(idVenda) {
  modelVenda.setTable("Venda");
  modelVenda.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelVenda.setFilter("idVenda = '" + idVenda + "'");
  if (not modelVenda.select() or modelVenda.rowCount() == 0) {
    qDebug() << "Failed to populate venda!" << modelVenda.lastError();
  }

  modelLoja.setTable("Loja");
  modelLoja.setEditStrategy(QSqlTableModel::OnManualSubmit);
  QString idLoja = QString::number(UserSession::getLoja());
  modelLoja.setFilter("idLoja = '" + idLoja + "'");
  if (not modelLoja.select() or modelLoja.rowCount() == 0) {
    qDebug() << "Failed to populate loja!" << modelLoja.lastError();
  }

  modelItem.setTable("Venda_has_Produto");
  modelItem.setFilter("idVenda = '" + idVenda + "'");
  modelItem.setEditStrategy(QSqlTableModel::OnManualSubmit);
  if (not modelItem.select() or modelItem.rowCount() == 0) {
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

bool NFe::TXT() {
  QString chave = criarChaveAcesso();

  chave += calculaDigitoVerificador(chave);

  return writeTXT(chave);
}

bool NFe::TXT_Pedido(QList<int> rows) {
  QString chave = criarChaveAcesso();

  chave += calculaDigitoVerificador(chave);

  return writeTXT_Pedido(chave, rows);
}

NFe::~NFe() {}

bool NFe::writeTXT(QString chave) {
  //  QFile file(idVenda + ".txt");
  QFile file("C:/ACBrNFeMonitor/ENTNFE.TXT/" + idVenda + ".txt");
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
  for (int row = 0; row < modelItem.rowCount(); ++row) {
    QSqlQuery prod("SELECT * FROM Produto WHERE idProduto = '" +
                   getFromItemModel(row, "idProduto").toString() + "'");
    if (not prod.exec()) {
      qDebug() << "Erro buscando produtos: " << prod.lastError();
      return false;
    }

    prod.first();
    QString number = QString("%1").arg(row + 1, 3, 10, QChar('0')); // padding row with zeros
    stream << "[Produto" + number + "]" << endl;
    //  stream << "CFOP = " + prod.value("cfop").toString() << endl;
    QString cfop = QInputDialog::getText(0, "CFOP", "Insira o CFOP (4 digítos):<br>O CFOP de entrada era '" +
                                         prod.value("cfop").toString() + "'");
    stream << "CFOP = " + cfop << endl;

    // QString ncm = QInputDialog::getText(0, "NCM", "Insira o NCM (8 dígitos): ");
    // stream << "NCM = " + ncm << endl;

    stream << "NCM = " + clearStr(prod.value("ncm").toString()) << endl;
    qDebug() << "NCM = " << clearStr(prod.value("ncm").toString());

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

    if (getFromItemModel(row, "qte").toString().isEmpty()) {
      qDebug() << "qte vazio";
      return false;
    }
    stream << "Quantidade = " + getFromItemModel(row, "qte").toString() << endl;

    if (prod.value("precoVenda").toDouble() == 0) {
      qDebug() << "precoVenda = 0";
      return false;
    }
    double preco = prod.value("precoVenda").toDouble();
    double rounded_number = static_cast<double>(static_cast<int>(preco * 100 + 0.5)) / 100.0;
    stream << "ValorUnitario = " + QString::number(rounded_number) << endl;

    if (getFromItemModel(row, "parcial").toString().isEmpty()) {
      qDebug() << "parcial vazio";
      return false;
    }
    stream << "ValorTotal = " + getFromItemModel(row, "parcial").toString() << endl;

    total += getFromItemModel(row, "parcial").toDouble();

    //    if(cfop.endsWith("405")){
    //      stream << "[ICMS" + number + "]" << endl;
    //      stream << "CST = 60";
    //      stream << "ValorBase = 0";
    //      stream << "Aliquota = 0";
    //    }else{
    stream << "[ICMS" + number + "]" << endl;
    stream << "CST = 650" << endl;
    stream << "ValorBase = " + getFromItemModel(row, "parcial").toString() << endl;
    stream << "Aliquota = 18.00" << endl;
    //    }

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
  return true;
}

bool NFe::writeTXT_Pedido(QString chave, QList<int> rows) {
  //  QFile file(idVenda + ".txt");
  QFile file("C:/ACBrNFeMonitor/ENTNFE.TXT/" + idVenda + ".txt");
  //  qDebug() << QDir::current().absoluteFilePath(idVenda + ".txt");
  file.open(QFile::WriteOnly);
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
  if (clearStr(getFromLoja("inscEstadual").toString()).isEmpty()) {
    qDebug() << "inscEstadual vazia";
    return false;
  }
  stream << "IE = " + clearStr(getFromLoja("inscEstadual").toString()) << endl;
  // stream << "IE = 110042490114" << endl; //TODO: fix inscricao estadual emitente

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
    }
    if (cliente.value("pfpj").toString() == "PJ") {
      if (clearStr(cliente.value("cnpj").toString()).isEmpty()) {
        qDebug() << "cnpj dest vazio";
        return false;
      }
      stream << "CNPJ = " + clearStr(cliente.value("cnpj").toString()) << endl;

      //      stream << "IE = 110042490114" << endl;
      stream << "IE = " + clearStr(cliente.value("inscEstadual").toString()) << endl;
      //  stream << "IE = ISENTO" << endl;
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
  int i = 0;
  foreach (int row, rows) {
    QSqlQuery prod("SELECT * FROM Produto WHERE idProduto = '" +
                   getFromItemModel(row, "idProduto").toString() + "'");
    if (not prod.exec()) {
      qDebug() << "Erro buscando produtos: " << prod.lastError();
      return false;
    }

    prod.first();
    QString number = QString("%1").arg(++i, 3, 10, QChar('0')); // padding row with zeros
    stream << "[Produto" + number + "]" << endl;
    //  stream << "CFOP = " + prod.value("cfop").toString() << endl;
    QString cfop = QInputDialog::getText(0, "CFOP", "Insira o CFOP (4 digítos): ");
    stream << "CFOP = " + cfop << endl;
    QString ncm = QInputDialog::getText(0, "NCM", "Insira o NCM (8 dígitos): ");
    stream << "NCM = " + ncm << endl;

    if (prod.value("codBarras").toString().isEmpty()) {
      qDebug() << "codBarras vazio";
      return false;
    }
    stream << "Codigo = " + clearStr(prod.value("codBarras").toString()) << endl;

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

    if (getFromItemModel(row, "qte").toString().isEmpty()) {
      qDebug() << "qte vazio";
      return false;
    }
    stream << "Quantidade = " + getFromItemModel(row, "qte").toString() << endl;

    if (prod.value("precoVenda").toDouble() == 0) {
      qDebug() << "precoVenda = 0";
      return false;
    }
    double preco = prod.value("precoVenda").toDouble();
    double rounded_number = static_cast<double>(static_cast<int>(preco * 100 + 0.5)) / 100.0;
    stream << "ValorUnitario = " + QString::number(rounded_number) << endl;

    if (getFromItemModel(row, "parcial").toString().isEmpty()) {
      qDebug() << "parcial vazio";
      return false;
    }
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

  stream << "\"), 1, [1]";

  stream.flush();
  file.close();
  return true;
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

QString NFe::getArquivo() const { return arquivo; }

QString NFe::getChaveAcesso() const { return chaveAcesso; }
