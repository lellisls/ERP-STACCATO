#include <QDomDocument>
#include <QFileDialog>
#include <QString>
#include <QDebug>
#include <QSqlError>
#include <QMessageBox>

#include "xml.h"
#include "usersession.h"

XML::XML() {
  modelProduto.setTable("produto");
  modelProduto.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelProduto.select()) {
    qDebug() << "erro model produto: " << modelProduto.lastError();
    return;
  }
}

void XML::importarXML() {
  fileName = QFileDialog::getOpenFileName(0, "Importar arquivo XML", QDir::currentPath(), ("XML (*.xml)"));

  if (fileName.isEmpty()) {
    return;
  }

  fileName.replace("/", "\\\\");

  readXML();
  saveXML();
}

void XML::readXML() {
  QDomDocument document;

  QFile file(fileName);

  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    document.setContent(&file);
    file.close();
  }

  QDomElement root = document.firstChildElement();
  QDomNamedNodeMap map = root.attributes();
  QStandardItem *rootItem;

  if (map.size() > 0) {
    QString attributes = root.nodeName() + " ";

    for (int i = 0; i < map.size(); ++i) {
      if (i > 0) {
        attributes += " ";
      }

      attributes += map.item(i).nodeName() + "=\"" + map.item(i).nodeValue() + "\"";
    }

    rootItem = new QStandardItem(attributes);
  } else {
    rootItem = new QStandardItem(root.nodeName());
  }

  model.appendRow(rootItem);

  readChild(root, rootItem);
}

void XML::readChild(QDomElement element, QStandardItem *elementItem) {
  QDomElement child = element.firstChildElement();
  QStandardItem *childItem;

  for (; not child.isNull(); child = child.nextSiblingElement()) {
    if (child.firstChild().isText()) {
      childItem = new QStandardItem(child.nodeName() + " - " + child.text());
      elementItem->appendRow(childItem);
      continue;
    }

    QDomNamedNodeMap map = child.attributes();
    QString attributes = child.nodeName() + " ";

    if (map.size() > 0) {
      for (int i = 0; i < map.size(); ++i) {
        if (i > 0) {
          attributes += " ";
        }

        attributes += map.item(i).nodeName() + "=\"" + map.item(i).nodeValue() + "\"";
      }

      childItem = new QStandardItem(attributes);
      elementItem->appendRow(childItem);
      readChild(child, childItem);
      continue;
    }

    childItem = new QStandardItem(child.nodeName());
    elementItem->appendRow(childItem);
    readChild(child, childItem);
  }
}

void XML::saveXML() {
  QStandardItem *modelRoot = model.item(0, 0);

  if (modelRoot->hasChildren()) {
    readTree(modelRoot);
  }
}

bool XML::readTree(QStandardItem *item) {
  for (int i = 0; i < item->rowCount(); ++i) {
    for (int j = 0; j < item->columnCount(); ++j) {
      QStandardItem *child = item->child(i, j);

      if (child->text().left(6) == "infNFe") {
        chaveAcesso = child->text().mid(child->text().indexOf("Id=") + 7, 44);

        QSqlQuery query;
        query.prepare("SELECT * FROM nfe WHERE chaveAcesso = :chaveAcesso");
        query.bindValue(":chaveAcesso", chaveAcesso);

        if (not query.exec()) {
          qDebug() << "Erro verificando se nota já cadastrada: " << query.lastError();
          return false;
        }

        if (query.first()) {
          QMessageBox::warning(0, "Aviso!", "Nota já cadastrada!");
          return false;
        }
      }

      if (child->parent()->text() == "emit" and child->text().left(7) == "xFant -") {
        xFant = child->text().remove(0, 8);
      }

      if (child->parent()->text() == "emit" and child->text().left(7) == "xNome -") {
        xNome = child->text().remove(0, 8);
      }

      if (child->parent()->text() == "dest" and child->text().left(6) == "CNPJ -") {
        cnpj = child->text().remove(0, 7);
        // TODO: readd these

        //        bool match = false;

        //        for (const auto cnpjLoja : UserSession::getTodosCNPJ()) {
        //          if (cnpj == cnpjLoja) {
        //            match = true;
        //          }
        //        }

        //        if (not match) {
        //          QMessageBox::warning(0, "Aviso!", "CNPJ do destinatário difere do CNPJ da loja!");
        //          qDebug() << cnpj << " - " << UserSession::getFromLoja("cnpj").remove(".").remove("/").remove("-");
        //          return false;
        //        }
      }

      //      if (child->parent()->text() == "dest" and child->text().left(5) == "CPF -") {
      //        QMessageBox::warning(0, "Aviso!", "Destinatário da nota é pessoa física!");
      //        return false;
      //      }

      lerDadosProduto(child);
      lerICMSProduto(child);
      lerIPIProduto(child);
      lerIPIProduto(child);
      lerCOFINSProduto(child);
      lerTotais(child);

      if (child->hasChildren()) {
        if (not readTree(child)) {
          return false;
        }
      }
    }
  }

  return true;
}

void XML::inserirNoModel(QStandardItem *item, SqlTableModel *externalModel) {
  for (int i = 0; i < item->rowCount(); ++i) {
    for (int j = 0; j < item->columnCount(); ++j) {
      QStandardItem *child = item->child(i, j);

      lerDadosProduto(child);
      lerICMSProduto(child);
      lerIPIProduto(child);
      lerIPIProduto(child);
      lerCOFINSProduto(child);
      lerTotais(child);

      if (child->text().mid(0, 10) == "det nItem=") {
        inserirItem(externalModel);
      }

      if (child->hasChildren()) {
        inserirNoModel(child, externalModel);
      }
    }
  }
}

int XML::cadastrarNFe() {
  QSqlQuery query;

  query.prepare("SELECT * FROM nfe WHERE chaveAcesso = :chaveAcesso");
  query.bindValue(":chaveAcesso", chaveAcesso);

  if (not query.exec()) {
    qDebug() << "Erro verificando se nota já cadastrada: " << query.lastError();
    return false;
  }

  qDebug() << "fileName: " << fileName;

  QFile file(fileName);

  if (not file.open(QFile::ReadOnly)) {
    qDebug() << "Erro lendo arquivo: " << file.errorString();
    return false;
  }

  QString fileContents = file.readAll();

  if (not query.first()) {
    query.prepare("INSERT INTO nfe (idVendaCompra, tipo, chaveAcesso, xml) VALUES (:idVendaCompra, 'ENTRADA', "
                  ":chaveAcesso, :xml)");
    query.bindValue(":idVendaCompra", "PLACEHOLDER");
    query.bindValue(":chaveAcesso", chaveAcesso);
    query.bindValue(":xml", fileContents);

    if (not query.exec()) {
      qDebug() << "Erro cadastrando xml no estoque: " << query.lastError();
      return false;
    }

    idNFe = query.lastInsertId().toInt();
  }

  return idNFe;
}

void XML::lerDadosProduto(QStandardItem *child) {
  if (child->text().mid(0, 10) == "det nItem=") {
    itemNumero = child->text().right(child->text().size() - 10).remove("\"").toInt();
  }

  if (child->parent()->text() == "prod" and child->text().left(7) == "cProd -") {
    codProd = child->text().remove(0, 8);
  }

  if (child->parent()->text() == "prod" and child->text().left(6) == "cEAN -") {
    codBarras = child->text().remove(0, 7);
  }

  if (child->parent()->text() == "prod" and child->text().left(7) == "xProd -") {
    descricao = child->text().remove(0, 8);
  }

  if (child->parent()->text() == "prod" and child->text().left(5) == "NCM -") {
    ncm = child->text().remove(0, 6);
  }

  if (child->parent()->text() == "prod" and child->text().left(6) == "CFOP -") {
    cfop = child->text().remove(0, 7);
  }

  if (child->parent()->text() == "prod" and child->text().left(6) == "uCom -") {
    un = child->text().remove(0, 7);
  }

  if (child->parent()->text() == "prod" and child->text().left(6) == "qCom -") {
    quant = child->text().remove(0, 7).toDouble();
  }

  if (child->parent()->text() == "prod" and child->text().left(8) == "vUnCom -") {
    valorUnid = child->text().remove(0, 9).toDouble();
  }

  if (child->parent()->text() == "prod" and child->text().left(7) == "vProd -") {
    valor = child->text().remove(0, 8).toDouble();
  }

  if (child->parent()->text() == "prod" and child->text().left(10) == "cEANTrib -") {
    codBarrasTrib = child->text().remove(0, 11);
  }

  if (child->parent()->text() == "prod" and child->text().left(7) == "uTrib -") {
    unTrib = child->text().remove(0, 8);
  }

  if (child->parent()->text() == "prod" and child->text().left(7) == "qTrib -") {
    quantTrib = child->text().remove(0, 8).toDouble();
  }

  if (child->parent()->text() == "prod" and child->text().left(9) == "vUnTrib -") {
    valorTrib = child->text().remove(0, 10).toDouble();
  }

  if (child->parent()->text() == "prod" and child->text().left(7) == "vDesc -") {
    desconto = child->text().remove(0, 8).toDouble();
  }

  if (child->parent()->text() == "prod" and child->text().left(8) == "indTot -") {
    compoeTotal = child->text().remove(0, 9).toInt();
  }

  if (child->parent()->text() == "prod" and child->text().left(6) == "xPed -") {
    numeroPedido = child->text().remove(0, 7);
  }

  if (child->parent()->text() == "prod" and child->text().left(10) == "nItemPed -") {
    itemPedido = child->text().remove(0, 11).toDouble();
  }
}

void XML::lerICMSProduto(QStandardItem *child) {
  if (child->text() == "ICMS") {
    tipoICMS = child->child(0, 0)->text();
  }

  if (child->parent()->text() == tipoICMS and child->text().left(6) == "orig -") {
    orig = child->text().remove(0, 7).toInt();
  }

  if (child->parent()->text() == tipoICMS and child->text().left(5) == "CST -") {
    cstICMS = child->text().remove(0, 6).toInt();
  }

  if (child->parent()->text() == tipoICMS and child->text().left(7) == "modBC -") {
    modBC = child->text().remove(0, 8).toInt();
  }

  if (child->parent()->text() == tipoICMS and child->text().left(5) == "vBC -") {
    vBC = child->text().remove(0, 6).toDouble();
  }

  if (child->parent()->text() == tipoICMS and child->text().left(7) == "pICMS -") {
    pICMS = child->text().remove(0, 8).toDouble();
  }

  if (child->parent()->text() == tipoICMS and child->text().left(7) == "vICMS -") {
    vICMS = child->text().remove(0, 8).toDouble();
  }

  if (child->parent()->text() == tipoICMS and child->text().left(9) == "modBCST -") {
    modBCST = child->text().remove(0, 10).toInt();
  }

  if (child->parent()->text() == tipoICMS and child->text().left(8) == "pMVAST -") {
    pMVAST = child->text().remove(0, 9).toDouble();
  }

  if (child->parent()->text() == tipoICMS and child->text().left(7) == "vBCST -") {
    vBCST = child->text().remove(0, 8).toDouble();
  }

  if (child->parent()->text() == tipoICMS and child->text().left(9) == "pICMSST -") {
    pICMSST = child->text().remove(0, 10).toDouble();
  }

  if (child->parent()->text() == tipoICMS and child->text().left(9) == "vICMSST -") {
    vICMSST = child->text().remove(0, 10).toDouble();
  }
}

void XML::lerIPIProduto(QStandardItem *child) {
  if (child->parent()->text() == "IPI" and child->text().left(6) == "cEnq -") {
    cEnq = child->text().remove(0, 7).toInt();
  }

  if (child->parent()->text() == "IPINT" and child->text().left(5) == "CST -") {
    cstIPI = child->text().remove(0, 6).toInt();
  }
}

void XML::lerPISProduto(QStandardItem *child) {
  if (child->parent()->text() == "PISAliq" and child->text().left(5) == "CST -") {
    cstPIS = child->text().remove(0, 6).toInt();
  }

  if (child->parent()->text() == "PISAliq" and child->text().left(5) == "vBC -") {
    vBCPIS = child->text().remove(0, 6).toDouble();
  }

  if (child->parent()->text() == "PISAliq" and child->text().left(6) == "pPIS -") {
    pPIS = child->text().remove(0, 7).toDouble();
  }

  if (child->parent()->text() == "PISAliq" and child->text().left(6) == "vPIS -") {
    vPIS = child->text().remove(0, 7).toDouble();
  }
}

void XML::lerCOFINSProduto(QStandardItem *child) {
  if (child->parent()->text() == "COFINSAliq" and child->text().left(5) == "CST -") {
    cstCOFINS = child->text().remove(0, 6).toInt();
  }

  if (child->parent()->text() == "COFINSAliq" and child->text().left(5) == "vBC -") {
    vBCCOFINS = child->text().remove(0, 6).toDouble();
  }

  if (child->parent()->text() == "COFINSAliq" and child->text().left(9) == "pCOFINS -") {
    pCOFINS = child->text().remove(0, 10).toDouble();
  }

  if (child->parent()->text() == "COFINSAliq" and child->text().left(9) == "vCOFINS -") {
    vCOFINS = child->text().remove(0, 10).toDouble();
  }
}

void XML::lerTotais(QStandardItem *child) {
  if (child->parent()->text() == "ICMSTot" and child->text().left(5) == "vBC -") {
    vBC_Total = child->text().remove(0, 6).toDouble();
  }

  if (child->parent()->text() == "ICMSTot" and child->text().left(7) == "vICMS -") {
    vICMS_Total = child->text().remove(0, 8).toDouble();
  }

  if (child->parent()->text() == "ICMSTot" and child->text().left(12) == "vICMSDeson -") {
    vICMSDeson_Total = child->text().remove(0, 13).toDouble();
  }

  if (child->parent()->text() == "ICMSTot" and child->text().left(7) == "vBCST -") {
    vBCST_Total = child->text().remove(0, 8).toDouble();
  }

  if (child->parent()->text() == "ICMSTot" and child->text().left(5) == "vST -") {
    vST_Total = child->text().remove(0, 6).toDouble();
  }

  if (child->parent()->text() == "ICMSTot" and child->text().left(7) == "vProd -") {
    vProd_Total = child->text().remove(0, 8).toDouble();
  }

  if (child->parent()->text() == "ICMSTot" and child->text().left(8) == "vFrete -") {
    vFrete_Total = child->text().remove(0, 9).toDouble();
  }

  if (child->parent()->text() == "ICMSTot" and child->text().left(6) == "vSeg -") {
    vSeg_Total = child->text().remove(0, 7).toDouble();
  }

  if (child->parent()->text() == "ICMSTot" and child->text().left(7) == "vDesc -") {
    vDesc_Total = child->text().remove(0, 8).toDouble();
  }

  if (child->parent()->text() == "ICMSTot" and child->text().left(5) == "vII -") {
    vII_Total = child->text().remove(0, 6).toDouble();
  }

  if (child->parent()->text() == "ICMSTot" and child->text().left(6) == "vIPI -") {
    vPIS_Total = child->text().remove(0, 7).toDouble();
  }

  if (child->parent()->text() == "ICMSTot" and child->text().left(6) == "vPIS -") {
    vPIS_Total = child->text().remove(0, 7).toDouble();
  }

  if (child->parent()->text() == "ICMSTot" and child->text().left(9) == "vCOFINS -") {
    vCOFINS_Total = child->text().remove(0, 10).toDouble();
  }

  if (child->parent()->text() == "ICMSTot" and child->text().left(8) == "vOutro -") {
    vOutro_Total = child->text().remove(0, 9).toDouble();
  }

  if (child->parent()->text() == "ICMSTot" and child->text().left(5) == "vNF -") {
    vNF_Total = child->text().remove(0, 6).toDouble();
  }
}

bool XML::insertEstoque() {
  QSqlQuery query;

  if (not query.exec("SELECT fornecedor FROM produto WHERE codComercial = '" + codProd + "'")) {
    qDebug() << "Erro buscando fornecedor: " << query.lastError();
    return false;
  }

  QString fornecedor;

  if (query.first()) {
    fornecedor = query.value(0).toString();
  } else {
    // TODO: cadastrar produto
    QMessageBox::warning(0, "Aviso!", "Produto não cadastrado, fornecedor em branco.");
  }

  qDebug() << "fornecedor: " << fornecedor;

  query.prepare(
        "INSERT INTO estoque (idProduto, idXML, fornecedor, descricao, quant, un, codBarras, codComercial, ncm, cfop, "
        "valorUnid, valor, codBarrasTrib, unTrib, quantTrib, valorTrib, desconto, compoeTotal, numeroPedido, itemPedido, "
        "tipoICMS, orig, cstICMS, modBC, vBC, pICMS, vICMS, modBCST, pMVAST, vBCST, pICMSST, vICMSST, cEnq, cstIPI, "
        "cstPIS, vBCPIS, pPIS, vPIS, cstCOFINS, vBCCOFINS, pCOFINS, vCOFINS) VALUES (:idProduto, :idXML, :fornecedor, "
        ":descricao, :quant, :un, :codBarras, :codComercial, :ncm, :cfop, :valorUnid, :valor, :codBarrasTrib, :unTrib, "
        ":quantTrib, :valorTrib, :desconto, :compoeTotal, :numeroPedido, :itemPedido, :tipoICMS, :orig, :cstICMS, "
        ":modBC, :vBC, :pICMS, :vICMS, :modBCST, :pMVAST, :vBCST, :pICMSST, :vICMSST, :cEnq, :cstIPI, :cstPIS, :vBCPIS, "
        ":pPIS, :vPIS, :cstCOFINS, :vBCCOFINS, :pCOFINS, :vCOFINS)");
  query.bindValue(":idProduto", idProduto);
  query.bindValue(":idXML", idNFe);
  query.bindValue(":fornecedor", fornecedor);
  query.bindValue(":descricao", descricao);
  query.bindValue(":quant", quant);
  query.bindValue(":un", un);
  query.bindValue(":codBarras", codBarras);
  query.bindValue(":codComercial", codProd);
  query.bindValue(":ncm", ncm);
  query.bindValue(":cfop", cfop);
  query.bindValue(":valorUnid", valorUnid);
  query.bindValue(":valor", valor);
  query.bindValue(":codBarrasTrib", codBarrasTrib);
  query.bindValue(":unTrib", unTrib);
  query.bindValue(":quantTrib", quantTrib);
  query.bindValue(":valorTrib", valorTrib);
  query.bindValue(":desconto", desconto);
  query.bindValue(":compoeTotal", compoeTotal);
  query.bindValue(":numeroPedido", numeroPedido);
  query.bindValue(":itemPedido", itemPedido);
  query.bindValue(":tipoICMS", tipoICMS);
  query.bindValue(":orig", orig);
  query.bindValue(":cstICMS", cstICMS);
  query.bindValue(":modBC", modBC);
  query.bindValue(":vBC", vBC);
  query.bindValue(":pICMS", pICMS);
  query.bindValue(":vICMS", vICMS);
  query.bindValue(":modBCST", modBCST);
  query.bindValue(":pMVAST", pMVAST);
  query.bindValue(":vBCST", vBCST);
  query.bindValue(":pICMSST", pICMSST);
  query.bindValue(":vICMSST", vICMSST);
  query.bindValue(":cEnq", cEnq);
  query.bindValue(":cstIPI", cstIPI);
  query.bindValue(":cstPIS", cstPIS);
  query.bindValue(":vBCPIS", vBCPIS);
  query.bindValue(":pPIS", pPIS);
  query.bindValue(":vPIS", vPIS);
  query.bindValue(":cstCOFINS", cstCOFINS);
  query.bindValue(":vBCCOFINS", vBCCOFINS);
  query.bindValue(":pCOFINS", pCOFINS);
  query.bindValue(":vCOFINS", vCOFINS);

  if (not query.exec()) {
    qDebug() << "Error: " << query.lastError();
    return false;
  }

  return true;
}

void XML::mostrarNoModel(QString file, SqlTableModel &externalModel) {
  QFile fileXML(file);

  if (not fileXML.open(QFile::ReadOnly)) {
    qDebug() << "Erro abrindo arquivo: " << fileXML.errorString();
    return;
  }

  QString fileContent = fileXML.readAll();

  if (fileContent.isEmpty()) {
    qDebug() << "is empty";
    return;
  }

  QDomDocument document;

  if (not document.setContent(fileContent)) {
    qDebug() << "erro setContent";
    qDebug() << "file: " << fileContent;
    return;
  }

  QDomElement root = document.firstChildElement();
  QDomNamedNodeMap map = root.attributes();
  QStandardItem *rootItem;

  if (map.size() > 0) {
    QString attributes = root.nodeName() + " ";

    for (int i = 0; i < map.size(); ++i) {
      if (i > 0) {
        attributes += " ";
      }

      attributes += map.item(i).nodeName() + "=\"" + map.item(i).nodeValue() + "\"";
    }

    rootItem = new QStandardItem(attributes);
  } else {
    rootItem = new QStandardItem(root.nodeName());
  }

  //---------------------

  fileName = file;
  model.appendRow(rootItem);
  readChild(root, rootItem);

  QStandardItem *modelRoot = model.item(0, 0);

  if (not readTree(modelRoot)) {
    return;
  }

  inserirNoModel(modelRoot, &externalModel);
}

bool XML::inserirItem(SqlTableModel *externalModel) {
  int row = externalModel->rowCount();

  if (not externalModel->insertRow(row)) {
    qDebug() << "Erro inserindo linha na tabela: " << externalModel->lastError();
  }

  QModelIndexList indexList =
      modelProduto.match(modelProduto.index(0, modelProduto.fieldIndex("codComercial")), Qt::DisplayRole, codProd, 1,
                         Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap));

  int idTemp = 0;

  if (indexList.size() > 0) {
    idTemp = modelProduto.data(indexList.first().row(), "idProduto").toInt();
  } else {
    qDebug() << "Nao encontrou produto: " << codProd;
  }

  if (not externalModel->setData(row, "fornecedor", xNome)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "idProduto", idTemp)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "descricao", descricao)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "quant", quant)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "un", un)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "codBarras", codBarras)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "codComercial", codProd)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "ncm", ncm)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "cfop", cfop)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "valorUnid", valorUnid)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "valor", valor)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "codBarrasTrib", codBarrasTrib)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "unTrib", unTrib)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "quantTrib", quantTrib)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "valorTrib", valorTrib)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "desconto", desconto)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "compoeTotal", compoeTotal)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "numeroPedido", numeroPedido)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "itemPedido", itemPedido)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "tipoICMS", tipoICMS)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "orig", orig)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "cstICMS", cstICMS)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "modBC", modBC)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "vBC", vBC)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "pICMS", pICMS)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "vICMS", vICMS)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "modBCST", modBCST)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "pMVAST", pMVAST)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "vBCST", vBCST)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "pICMSST", pICMSST)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "vICMSST", vICMSST)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "cEnq", cEnq)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "cstIPI", cstIPI)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "cstPIS", cstPIS)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if (not externalModel->setData(row, "vBCPIS", vBCPIS)) {
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if(not externalModel->setData(row, "pPIS", pPIS)){
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if(not externalModel->setData(row, "vPIS", vPIS)){
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if(not externalModel->setData(row, "cstCOFINS", cstCOFINS)){
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if(not externalModel->setData(row, "vBCCOFINS", vBCCOFINS)){
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if(not externalModel->setData(row, "pCOFINS", pCOFINS)){
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  if(not externalModel->setData(row, "vCOFINS", vCOFINS)){
    qDebug() << "Erro inserindo dados na tabela: " << externalModel->lastError();
  }

  return true;
}
