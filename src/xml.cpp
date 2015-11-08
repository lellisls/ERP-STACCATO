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
    QMessageBox::critical(0, "Erro!", "Erro lendo tabela produto: " + modelProduto.lastError().text());
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

  if (not file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QMessageBox::critical(0, "Erro!", "Erro abrindo arquivo: " + file.errorString());
    return;
  }

  QString *error = new QString();

  if (not document.setContent(&file, error)) {
    QMessageBox::critical(0, "Erro!", "Erro lendo arquivo: " + *error);
    return;
  }

  file.close();

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
          QMessageBox::critical(0, "Erro!", "Erro verificando se nota já cadastrada: " + query.lastError().text());
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

        //                bool match = false;

        //                for (const auto cnpjLoja : UserSession::getTodosCNPJ()) {
        //                  if (cnpj == cnpjLoja) {
        //                    match = true;
        //                  }
        //                }

        //                if (not match) {
        //                  QMessageBox::critical(0, "Erro!", "CNPJ do destinatário difere do CNPJ da loja!");
        //                  return false;
        //                }
      }

      //            if (child->parent()->text() == "dest" and child->text().left(5) == "CPF -") {
      //              QMessageBox::critical(0, "Erro!", "Destinatário da nota é pessoa física!");
      //              return false;
      //            }

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
    QMessageBox::critical(0, "Erro!", "Erro verificando se nota já cadastrada: " + query.lastError().text());
    return false;
  }

  qDebug() << "fileName: " << fileName;

  QFile file(fileName);

  if (not file.open(QFile::ReadOnly)) {
    QMessageBox::critical(0, "Erro!", "Erro lendo arquivo: " + file.errorString());
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
      QMessageBox::critical(0, "Erro!", "Erro cadastrando XML no estoque: " + query.lastError().text());
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

  if (child->parent()->text() == "prod") {
    if (child->text().left(7) == "cProd -") {
      codProd = child->text().remove(0, 8);
    }

    if (child->text().left(6) == "cEAN -") {
      codBarras = child->text().remove(0, 7);
    }

    if (child->text().left(7) == "xProd -") {
      descricao = child->text().remove(0, 8);
    }

    if (child->text().left(5) == "NCM -") {
      ncm = child->text().remove(0, 6);
    }

    if (child->text().left(6) == "CFOP -") {
      cfop = child->text().remove(0, 7);
    }

    if (child->text().left(6) == "uCom -") {
      un = child->text().remove(0, 7);
    }

    if (child->text().left(6) == "qCom -") {
      quant = child->text().remove(0, 7).toDouble();
    }

    if (child->text().left(8) == "vUnCom -") {
      valorUnid = child->text().remove(0, 9).toDouble();
    }

    if (child->text().left(7) == "vProd -") {
      valor = child->text().remove(0, 8).toDouble();
    }

    if (child->text().left(10) == "cEANTrib -") {
      codBarrasTrib = child->text().remove(0, 11);
    }

    if (child->text().left(7) == "uTrib -") {
      unTrib = child->text().remove(0, 8);
    }

    if (child->text().left(7) == "qTrib -") {
      quantTrib = child->text().remove(0, 8).toDouble();
    }

    if (child->text().left(9) == "vUnTrib -") {
      valorTrib = child->text().remove(0, 10).toDouble();
    }

    if (child->text().left(7) == "vDesc -") {
      desconto = child->text().remove(0, 8).toDouble();
    }

    if (child->text().left(8) == "indTot -") {
      compoeTotal = child->text().remove(0, 9).toInt();
    }

    if (child->text().left(6) == "xPed -") {
      numeroPedido = child->text().remove(0, 7);
    }

    if (child->text().left(10) == "nItemPed -") {
      itemPedido = child->text().remove(0, 11).toDouble();
    }
  }
}

void XML::lerICMSProduto(QStandardItem *child) {
  if (child->text() == "ICMS") {
    tipoICMS = child->child(0, 0)->text();
  }

  if (child->parent()->text() == tipoICMS) {
    if (child->text().left(6) == "orig -") {
      orig = child->text().remove(0, 7).toInt();
    }

    if (child->text().left(5) == "CST -") {
      cstICMS = child->text().remove(0, 6).toInt();
    }

    if (child->text().left(7) == "modBC -") {
      modBC = child->text().remove(0, 8).toInt();
    }

    if (child->text().left(5) == "vBC -") {
      vBC = child->text().remove(0, 6).toDouble();
    }

    if (child->text().left(7) == "pICMS -") {
      pICMS = child->text().remove(0, 8).toDouble();
    }

    if (child->text().left(7) == "vICMS -") {
      vICMS = child->text().remove(0, 8).toDouble();
    }

    if (child->text().left(9) == "modBCST -") {
      modBCST = child->text().remove(0, 10).toInt();
    }

    if (child->text().left(8) == "pMVAST -") {
      pMVAST = child->text().remove(0, 9).toDouble();
    }

    if (child->text().left(7) == "vBCST -") {
      vBCST = child->text().remove(0, 8).toDouble();
    }

    if (child->text().left(9) == "pICMSST -") {
      pICMSST = child->text().remove(0, 10).toDouble();
    }

    if (child->text().left(9) == "vICMSST -") {
      vICMSST = child->text().remove(0, 10).toDouble();
    }
  }
}

void XML::lerIPIProduto(QStandardItem *child) {
  if (child->parent()->text() == "IPI") {
    if (child->text().left(6) == "cEnq -") {
      cEnq = child->text().remove(0, 7).toInt();
    }

    if (child->text().left(5) == "CST -") {
      cstIPI = child->text().remove(0, 6).toInt();
    }
  }
}

void XML::lerPISProduto(QStandardItem *child) {
  if (child->parent()->text() == "PISAliq") {
    if (child->text().left(5) == "CST -") {
      cstPIS = child->text().remove(0, 6).toInt();
    }

    if (child->text().left(5) == "vBC -") {
      vBCPIS = child->text().remove(0, 6).toDouble();
    }

    if (child->text().left(6) == "pPIS -") {
      pPIS = child->text().remove(0, 7).toDouble();
    }

    if (child->text().left(6) == "vPIS -") {
      vPIS = child->text().remove(0, 7).toDouble();
    }
  }
}

void XML::lerCOFINSProduto(QStandardItem *child) {
  if (child->parent()->text() == "COFINSAliq") {
    if (child->text().left(5) == "CST -") {
      cstCOFINS = child->text().remove(0, 6).toInt();
    }

    if (child->text().left(5) == "vBC -") {
      vBCCOFINS = child->text().remove(0, 6).toDouble();
    }

    if (child->text().left(9) == "pCOFINS -") {
      pCOFINS = child->text().remove(0, 10).toDouble();
    }

    if (child->text().left(9) == "vCOFINS -") {
      vCOFINS = child->text().remove(0, 10).toDouble();
    }
  }
}

void XML::lerTotais(QStandardItem *child) {
  if (child->parent()->text() == "ICMSTot") {
    if (child->text().left(5) == "vBC -") {
      vBC_Total = child->text().remove(0, 6).toDouble();
    }

    if (child->text().left(7) == "vICMS -") {
      vICMS_Total = child->text().remove(0, 8).toDouble();
    }

    if (child->text().left(12) == "vICMSDeson -") {
      vICMSDeson_Total = child->text().remove(0, 13).toDouble();
    }

    if (child->text().left(7) == "vBCST -") {
      vBCST_Total = child->text().remove(0, 8).toDouble();
    }

    if (child->text().left(5) == "vST -") {
      vST_Total = child->text().remove(0, 6).toDouble();
    }

    if (child->text().left(7) == "vProd -") {
      vProd_Total = child->text().remove(0, 8).toDouble();
    }

    if (child->text().left(8) == "vFrete -") {
      vFrete_Total = child->text().remove(0, 9).toDouble();
    }

    if (child->text().left(6) == "vSeg -") {
      vSeg_Total = child->text().remove(0, 7).toDouble();
    }

    if (child->text().left(7) == "vDesc -") {
      vDesc_Total = child->text().remove(0, 8).toDouble();
    }

    if (child->text().left(5) == "vII -") {
      vII_Total = child->text().remove(0, 6).toDouble();
    }

    if (child->text().left(6) == "vIPI -") {
      vPIS_Total = child->text().remove(0, 7).toDouble();
    }

    if (child->text().left(6) == "vPIS -") {
      vPIS_Total = child->text().remove(0, 7).toDouble();
    }

    if (child->text().left(9) == "vCOFINS -") {
      vCOFINS_Total = child->text().remove(0, 10).toDouble();
    }

    if (child->text().left(8) == "vOutro -") {
      vOutro_Total = child->text().remove(0, 9).toDouble();
    }

    if (child->text().left(5) == "vNF -") {
      vNF_Total = child->text().remove(0, 6).toDouble();
    }
  }
}

bool XML::insertEstoque() {
  QSqlQuery query;

  if (not query.exec("SELECT fornecedor FROM produto WHERE codComercial = '" + codProd + "'")) {
    QMessageBox::critical(0, "Erro!", "Erro buscando fornecedor: " + query.lastError().text());
    return false;
  }

  QString fornecedor;

  // TODO: cadastrar produto
  if (not query.first()) {
    QMessageBox::warning(0, "Aviso!", "Produto não cadastrado, fornecedor em branco.");
    return false;
  }

  fornecedor = query.value(0).toString();

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
    QMessageBox::critical(0, "Erro!", "Erro: " + query.lastError().text());
    return false;
  }

  return true;
}

void XML::mostrarNoModel(QString file, SqlTableModel &externalModel) {
  QFile fileXML(file);

  if (not fileXML.open(QFile::ReadOnly)) {
    QMessageBox::critical(0, "Erro!", "Erro abrindo arquivo: " + fileXML.errorString());
    return;
  }

  QDomDocument document;
  QString *error = new QString();

  if (not document.setContent(fileXML.readAll(), error)) {
    QMessageBox::critical(0, "Erro!", "Erro lendo arquivo: " + *error);
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
    QMessageBox::critical(0, "Erro!", "Erro inserindo linha na tabela: " + externalModel->lastError().text());
    return false;
  }

  QModelIndexList indexList =
      modelProduto.match(modelProduto.index(0, modelProduto.fieldIndex("codComercial")), Qt::DisplayRole, codProd, 1,
                         Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap));

  if (indexList.size() == 0) {
    QMessageBox::critical(0, "Erro!", "Não encontrou produto " + codProd);
    return false;
  }

  int idTemp = modelProduto.data(indexList.first().row(), "idProduto").toInt();

  return externalModel->setData(row, "fornecedor", xNome) and externalModel->setData(row, "idProduto", idTemp) and
      externalModel->setData(row, "descricao", descricao) and externalModel->setData(row, "quant", quant) and
      externalModel->setData(row, "un", un) and externalModel->setData(row, "codBarras", codBarras) and
      externalModel->setData(row, "codComercial", codProd) and externalModel->setData(row, "ncm", ncm) and
      externalModel->setData(row, "cfop", cfop) and externalModel->setData(row, "valorUnid", valorUnid) and
      externalModel->setData(row, "valor", valor) and externalModel->setData(row, "codBarrasTrib", codBarrasTrib) and
      externalModel->setData(row, "unTrib", unTrib) and externalModel->setData(row, "quantTrib", quantTrib) and
      externalModel->setData(row, "valorTrib", valorTrib) and externalModel->setData(row, "desconto", desconto) and
      externalModel->setData(row, "compoeTotal", compoeTotal) and
      externalModel->setData(row, "numeroPedido", numeroPedido) and
      externalModel->setData(row, "itemPedido", itemPedido) and externalModel->setData(row, "tipoICMS", tipoICMS) and
      externalModel->setData(row, "orig", orig) and externalModel->setData(row, "cstICMS", cstICMS) and
      externalModel->setData(row, "modBC", modBC) and externalModel->setData(row, "vBC", vBC) and
      externalModel->setData(row, "pICMS", pICMS) and externalModel->setData(row, "vICMS", vICMS) and
      externalModel->setData(row, "modBCST", modBCST) and externalModel->setData(row, "pMVAST", pMVAST) and
      externalModel->setData(row, "vBCST", vBCST) and externalModel->setData(row, "pICMSST", pICMSST) and
      externalModel->setData(row, "vICMSST", vICMSST) and externalModel->setData(row, "cEnq", cEnq) and
      externalModel->setData(row, "cstIPI", cstIPI) and externalModel->setData(row, "cstPIS", cstPIS) and
      externalModel->setData(row, "vBCPIS", vBCPIS) and externalModel->setData(row, "pPIS", pPIS) and
      externalModel->setData(row, "vPIS", vPIS) and externalModel->setData(row, "cstCOFINS", cstCOFINS) and
      externalModel->setData(row, "vBCCOFINS", vBCCOFINS) and externalModel->setData(row, "pCOFINS", pCOFINS) and
      externalModel->setData(row, "vCOFINS", vCOFINS);
}
