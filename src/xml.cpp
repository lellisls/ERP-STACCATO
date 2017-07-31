#include <QMessageBox>

#include "xml.h"

XML::XML(const QByteArray &fileContent, const QString &fileName) : fileContent(fileContent), fileName(fileName) { montarArvore(model); }

void XML::readChild(QDomElement &element, QStandardItem *elementItem) {
  QDomElement child = element.firstChildElement();

  for (; not child.isNull(); child = child.nextSiblingElement()) {
    if (child.firstChild().isText()) {
      QStandardItem *childItem = new QStandardItem(child.nodeName() + " - " + child.text());
      elementItem->appendRow(childItem);
      continue;
    }

    QDomNamedNodeMap map = child.attributes();
    QString attributes = child.nodeName();

    if (map.size() > 0) {
      for (int i = 0; i < map.size(); ++i) {
        attributes += " " + map.item(i).nodeName() + R"(=")" + map.item(i).nodeValue() + R"(")";
      }
    }

    auto *childItem = new QStandardItem(attributes);
    elementItem->appendRow(childItem);
    readChild(child, childItem);
  }
}

void XML::lerValores(const QStandardItem *item) {
  for (int row = 0; row < item->rowCount(); ++row) {
    for (int col = 0; col < item->columnCount(); ++col) {
      const QStandardItem *child = item->child(row, col);
      QString text = child->text();

      if (text.left(6) == "infNFe") chaveAcesso = text.mid(text.indexOf("Id=") + 7, 44);
      if (text.left(3) == "nNF") nNF = text.remove(0, 6);

      if (child->parent()->text() == "emit" and text.left(7) == "xFant -") xFant = text.remove(0, 8);
      if (child->parent()->text() == "emit" and text.left(7) == "xNome -") xNome = text.remove(0, 8);
      if (child->parent()->text() == "dest" and text.left(6) == "CNPJ -") cnpj = text.remove(0, 7);
      if (child->parent()->text() == "transporta" and text.left(7) == "xNome -") xNomeTransp = text.remove(0, 8);

      lerDadosProduto(child);
      lerICMSProduto(child);
      lerIPIProduto(child);
      lerPISProduto(child);
      lerCOFINSProduto(child);
      lerTotais(child);

      if (child->hasChildren()) lerValores(child);
    }
  }
}

void XML::lerDadosProduto(const QStandardItem *child) {
  QString text = child->text();

  if (text.mid(0, 10) == "det nItem=") itemNumero = text.right(text.size() - 10).remove(R"(")").toInt();

  if (child->parent()->text() == "prod") {
    if (text.left(7) == "cProd -") codProd = text.remove(0, 8);
    if (text.left(6) == "cEAN -") codBarras = text.remove(0, 7);
    if (text.left(7) == "xProd -") descricao = text.remove(0, 8);
    if (text.left(5) == "NCM -") ncm = text.remove(0, 6);
    if (text.left(6) == "CFOP -") cfop = text.remove(0, 7);
    if (text.left(6) == "uCom -") un = text.remove(0, 7).toUpper();
    if (text.left(6) == "qCom -") quant = text.remove(0, 7).toDouble();
    if (text.left(8) == "vUnCom -") valorUnid = text.remove(0, 9).toDouble();
    if (text.left(7) == "vProd -") valor = text.remove(0, 8).toDouble();
    if (text.left(10) == "cEANTrib -") codBarrasTrib = text.remove(0, 11);
    if (text.left(7) == "uTrib -") unTrib = text.remove(0, 8);
    if (text.left(7) == "qTrib -") quantTrib = text.remove(0, 8).toDouble();
    if (text.left(9) == "vUnTrib -") valorTrib = text.remove(0, 10).toDouble();
    if (text.left(7) == "vDesc -") desconto = text.remove(0, 8).toDouble();
    if (text.left(8) == "indTot -") compoeTotal = static_cast<bool>(text.remove(0, 9).toInt());
    if (text.left(6) == "xPed -") numeroPedido = text.remove(0, 7);
    if (text.left(10) == "nItemPed -") itemPedido = text.remove(0, 11).toInt();
  }
}

void XML::lerICMSProduto(const QStandardItem *child) {
  QString text = child->text();

  if (text == "ICMS") tipoICMS = child->child(0, 0)->text();

  if (child->parent()->text() == tipoICMS) {
    if (text.left(6) == "orig -") orig = text.remove(0, 7).toInt();
    if (text.left(5) == "CST -") cstICMS = text.remove(0, 6).toInt();
    if (text.left(7) == "modBC -") modBC = text.remove(0, 8).toInt();
    if (text.left(5) == "vBC -") vBC = text.remove(0, 6).toDouble();
    if (text.left(7) == "pICMS -") pICMS = text.remove(0, 8).toDouble();
    if (text.left(7) == "vICMS -") vICMS = text.remove(0, 8).toDouble();
    if (text.left(9) == "modBCST -") modBCST = text.remove(0, 10).toInt();
    if (text.left(8) == "pMVAST -") pMVAST = text.remove(0, 9).toDouble();
    if (text.left(7) == "vBCST -") vBCST = text.remove(0, 8).toDouble();
    if (text.left(9) == "pICMSST -") pICMSST = text.remove(0, 10).toDouble();
    if (text.left(9) == "vICMSST -") vICMSST = text.remove(0, 10).toDouble();
  }
}

void XML::lerIPIProduto(const QStandardItem *child) {
  QString text = child->text();

  if (child->parent()->text() == "IPI") {
    if (text.left(6) == "cEnq -") cEnq = text.remove(0, 7).toInt();
    if (text.left(5) == "CST -") cstIPI = text.remove(0, 6).toInt();
  }
}

void XML::lerPISProduto(const QStandardItem *child) {
  QString text = child->text();

  if (child->parent()->text() == "PISAliq") {
    if (text.left(5) == "CST -") cstPIS = text.remove(0, 6).toInt();
    if (text.left(5) == "vBC -") vBCPIS = text.remove(0, 6).toDouble();
    if (text.left(6) == "pPIS -") pPIS = text.remove(0, 7).toDouble();
    if (text.left(6) == "vPIS -") vPIS = text.remove(0, 7).toDouble();
  }
}

void XML::lerCOFINSProduto(const QStandardItem *child) {
  QString text = child->text();

  if (child->parent()->text() == "COFINSAliq") {
    if (text.left(5) == "CST -") cstCOFINS = text.remove(0, 6).toInt();
    if (text.left(5) == "vBC -") vBCCOFINS = text.remove(0, 6).toDouble();
    if (text.left(9) == "pCOFINS -") pCOFINS = text.remove(0, 10).toDouble();
    if (text.left(9) == "vCOFINS -") vCOFINS = text.remove(0, 10).toDouble();
  }
}

void XML::lerTotais(const QStandardItem *child) {
  QString text = child->text();

  if (child->parent()->text() == "ICMSTot") {
    if (text.left(5) == "vBC -") vBC_Total = text.remove(0, 6).toDouble();
    if (text.left(7) == "vICMS -") vICMS_Total = text.remove(0, 8).toDouble();
    if (text.left(12) == "vICMSDeson -") vICMSDeson_Total = text.remove(0, 13).toDouble();
    if (text.left(7) == "vBCST -") vBCST_Total = text.remove(0, 8).toDouble();
    if (text.left(5) == "vST -") vST_Total = text.remove(0, 6).toDouble();
    if (text.left(7) == "vProd -") vProd_Total = text.remove(0, 8).toDouble();
    if (text.left(8) == "vFrete -") vFrete_Total = text.remove(0, 9).toDouble();
    if (text.left(6) == "vSeg -") vSeg_Total = text.remove(0, 7).toDouble();
    if (text.left(7) == "vDesc -") vDesc_Total = text.remove(0, 8).toDouble();
    if (text.left(5) == "vII -") vII_Total = text.remove(0, 6).toDouble();
    if (text.left(6) == "vIPI -") vPIS_Total = text.remove(0, 7).toDouble();
    if (text.left(6) == "vPIS -") vPIS_Total = text.remove(0, 7).toDouble();
    if (text.left(9) == "vCOFINS -") vCOFINS_Total = text.remove(0, 10).toDouble();
    if (text.left(8) == "vOutro -") vOutro_Total = text.remove(0, 9).toDouble();
    if (text.left(5) == "vNF -") vNF_Total = text.remove(0, 6).toDouble();
  }
}

void XML::montarArvore(QStandardItemModel &model) {
  if (fileContent.isEmpty()) return;

  QDomDocument document;
  QString error;

  if (not document.setContent(fileContent, &error)) {
    QMessageBox::critical(nullptr, "Erro!", "Erro lendo arquivo: " + error);
    return;
  }

  QDomElement root = document.firstChildElement();
  QDomNamedNodeMap map = root.attributes();
  QString attributes = root.nodeName();

  if (map.size() > 0) {
    for (int i = 0; i < map.size(); ++i) {
      attributes += " " + map.item(i).nodeName() + R"(=")" + map.item(i).nodeValue() + R"(")";
    }
  }

  auto *rootItem = new QStandardItem(attributes);

  model.appendRow(rootItem);

  readChild(root, rootItem);

  lerValores(model.item(0, 0));
}
