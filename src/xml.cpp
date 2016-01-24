#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "xml.h"

XML::XML(const QByteArray &fileContent, const QString &fileName) : XML(model, fileContent, fileName) {}

XML::XML(QStandardItemModel &model, const QByteArray &fileContent, const QString &fileName)
  : fileContent(fileContent), fileName(fileName) {
  modelProduto.setTable("produto");
  modelProduto.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelProduto.select()) {
    QMessageBox::critical(0, "Erro!", "Erro lendo tabela produto: " + modelProduto.lastError().text());
  }

  if (not fileContent.isEmpty()) {
    QDomDocument document;
    QString error;

    if (not document.setContent(fileContent, &error)) {
      QMessageBox::critical(0, "Erro!", "Erro lendo arquivo: " + error);
      return;
    }

    QDomElement root = document.firstChildElement();
    QDomNamedNodeMap map = root.attributes();
    QString attributes = root.nodeName();

    if (map.size() > 0) {
      for (int i = 0; i < map.size(); ++i) {
        attributes += " " + map.item(i).nodeName() + "=\"" + map.item(i).nodeValue() + "\"";
      }
    }

    QStandardItem *rootItem = new QStandardItem(attributes);

    model.appendRow(rootItem);

    readChild(root, rootItem);

    lerValores(model.item(0, 0));
  }
}

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
        attributes += " " + map.item(i).nodeName() + "=\"" + map.item(i).nodeValue() + "\"";
      }
    }

    QStandardItem *childItem = new QStandardItem(attributes);
    elementItem->appendRow(childItem);
    readChild(child, childItem);
  }
}

bool XML::lerValores(const QStandardItem *item) {
  for (int row = 0; row < item->rowCount(); ++row) {
    for (int col = 0; col < item->columnCount(); ++col) {
      QStandardItem *child = item->child(row, col);
      QString text = child->text();

      if (text.left(6) == "infNFe") chaveAcesso = text.mid(text.indexOf("Id=") + 7, 44);

      if (child->parent()->text() == "emit" and text.left(7) == "xFant -") xFant = text.remove(0, 8);
      if (child->parent()->text() == "emit" and text.left(7) == "xNome -") xNome = text.remove(0, 8);
      if (child->parent()->text() == "dest" and text.left(6) == "CNPJ -") cnpj = text.remove(0, 7);

      if (child->parent()->text() == "dest" and text.left(5) == "CPF -") {
        QMessageBox::critical(0, "Erro!", "Destinatário da nota é pessoa física!");
        return false;
      }

      lerDadosProduto(child);
      lerICMSProduto(child);
      lerIPIProduto(child);
      lerPISProduto(child);
      lerCOFINSProduto(child);
      lerTotais(child);

      if (child->hasChildren()) {
        if (not lerValores(child)) return false;
      }
    }
  }

  return true;
}

void XML::inserirNoSqlModel(const QStandardItem *item, SqlTableModel *externalModel) {
  for (int row = 0; row < item->rowCount(); ++row) {
    for (int col = 0; col < item->columnCount(); ++col) {
      QStandardItem *child = item->child(row, col);
      QString text = child->text();

      if (text.mid(0, 10) == "det nItem=") {
        lerValores(child);
        inserirItemSql(externalModel);
      }

      if (child->hasChildren()) inserirNoSqlModel(child, externalModel);
    }
  }
}

int XML::cadastrarNFe(const QString &tipo) {
  if (not verificaCNPJ() or verificaExiste()) return 0;

  QSqlQuery query;

  query.prepare("SELECT idNFe FROM nfe WHERE chaveAcesso = :chaveAcesso");
  query.bindValue(":chaveAcesso", chaveAcesso);

  if (not query.exec()) {
    QMessageBox::critical(0, "Erro!", "Erro verificando se nota já cadastrada: " + query.lastError().text());
    return false;
  }

  if (query.first()) return idNFe = query.value("idNFe").toInt();

  QFile file(fileName);

  if (not file.open(QFile::ReadOnly)) {
    QMessageBox::critical(0, "Erro!", "Erro lendo arquivo: " + file.errorString());
    return false;
  }

  query.prepare("INSERT INTO nfe (idCompra, tipo, chaveAcesso, xml) VALUES (:idCompra, :tipo, "
                ":chaveAcesso, :xml)");
  query.bindValue(":idCompra", "PLACEHOLDER");
  query.bindValue(":tipo", tipo);
  query.bindValue(":chaveAcesso", chaveAcesso);
  query.bindValue(":xml", file.readAll());

  if (not query.exec()) {
    QMessageBox::critical(0, "Erro!", "Erro cadastrando XML no estoque: " + query.lastError().text());
    return false;
  }

  return idNFe = query.lastInsertId().toInt();
}

void XML::lerDadosProduto(const QStandardItem *child) {
  QString text = child->text();

  if (text.mid(0, 10) == "det nItem=") itemNumero = text.right(text.size() - 10).remove("\"").toInt();

  if (child->parent()->text() == "prod") {
    if (text.left(7) == "cProd -") codProd = text.remove(0, 8);
    if (text.left(6) == "cEAN -") codBarras = text.remove(0, 7);
    if (text.left(7) == "xProd -") descricao = text.remove(0, 8);
    if (text.left(5) == "NCM -") ncm = text.remove(0, 6);
    if (text.left(6) == "CFOP -") cfop = text.remove(0, 7);
    if (text.left(6) == "uCom -") un = text.remove(0, 7);
    if (text.left(6) == "qCom -") quant = text.remove(0, 7).toDouble();
    if (text.left(8) == "vUnCom -") valorUnid = text.remove(0, 9).toDouble();
    if (text.left(7) == "vProd -") valor = text.remove(0, 8).toDouble();
    if (text.left(10) == "cEANTrib -") codBarrasTrib = text.remove(0, 11);
    if (text.left(7) == "uTrib -") unTrib = text.remove(0, 8);
    if (text.left(7) == "qTrib -") quantTrib = text.remove(0, 8).toDouble();
    if (text.left(9) == "vUnTrib -") valorTrib = text.remove(0, 10).toDouble();
    if (text.left(7) == "vDesc -") desconto = text.remove(0, 8).toDouble();
    if (text.left(8) == "indTot -") compoeTotal = text.remove(0, 9).toInt();
    if (text.left(6) == "xPed -") numeroPedido = text.remove(0, 7);
    if (text.left(10) == "nItemPed -") itemPedido = text.remove(0, 11).toDouble();
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

bool XML::cadastrarEstoque() {
  QSqlQuery query;

  if (not query.exec("SELECT fornecedor FROM produto WHERE codComercial = '" + codProd + "'")) {
    QMessageBox::critical(0, "Erro!", "Erro buscando fornecedor: " + query.lastError().text());
    return false;
  }

  if (not query.first()) {
    QMessageBox::warning(0, "Aviso!", "Produto não cadastrado, fornecedor em branco.");
    return false;
  }

  const QString fornecedor = query.value("fornecedor").toString();

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

void XML::mostrarNoSqlModel(SqlTableModel &externalModel) { inserirNoSqlModel(model.item(0, 0), &externalModel); }

bool XML::inserirItemSql(SqlTableModel *externalModel) {
  int row = externalModel->rowCount();

  if (not externalModel->insertRow(row)) {
    QMessageBox::critical(0, "Erro!", "Erro inserindo linha na tabela: " + externalModel->lastError().text());
    return false;
  }

  QSqlQuery query;
  query.prepare(
        "SELECT idProduto, m2cx, pccx FROM produto WHERE codComercial = :codComercial OR codBarras = :codBarras");
  query.bindValue(":codComercial", codProd);
  query.bindValue(":codBarras", codBarras);

  if (not query.exec()) {
    QMessageBox::critical(0, "Erro!", "Erro lendo tabela produto: " + query.lastError().text());
    return false;
  }

  query.first();

  double quantCaixa =
      ((un == "M²") or (un == "M2") or (un == "ML")) ? query.value("m2cx").toDouble() : query.value("pccx").toDouble();

  int caixas = quant / quantCaixa;

  if (not externalModel->setData(row, "fornecedor", xNome)) return false;
  if (not externalModel->setData(row, "local", "TEMP")) return false;
  if (not externalModel->setData(row, "idProduto", query.first() ? query.value("idProduto").toInt() : 0)) return false;
  if (not externalModel->setData(row, "descricao", descricao)) return false;
  if (not externalModel->setData(row, "quant", quant)) return false;
  if (not externalModel->setData(row, "un", un)) return false;
  if (not externalModel->setData(row, "caixas", caixas)) return false;
  if (not externalModel->setData(row, "codBarras", codBarras)) return false;
  if (not externalModel->setData(row, "codComercial", codProd)) return false;
  if (not externalModel->setData(row, "ncm", ncm)) return false;
  if (not externalModel->setData(row, "cfop", cfop)) return false;
  if (not externalModel->setData(row, "valorUnid", valorUnid)) return false;
  if (not externalModel->setData(row, "valor", valor)) return false;
  if (not externalModel->setData(row, "codBarrasTrib", codBarrasTrib)) return false;
  if (not externalModel->setData(row, "unTrib", unTrib)) return false;
  if (not externalModel->setData(row, "quantTrib", quantTrib)) return false;
  if (not externalModel->setData(row, "valorTrib", valorTrib)) return false;
  if (not externalModel->setData(row, "desconto", desconto)) return false;
  if (not externalModel->setData(row, "compoeTotal", compoeTotal)) return false;
  if (not externalModel->setData(row, "numeroPedido", numeroPedido)) return false;
  if (not externalModel->setData(row, "itemPedido", itemPedido)) return false;
  if (not externalModel->setData(row, "tipoICMS", tipoICMS)) return false;
  if (not externalModel->setData(row, "orig", orig)) return false;
  if (not externalModel->setData(row, "cstICMS", cstICMS)) return false;
  if (not externalModel->setData(row, "modBC", modBC)) return false;
  if (not externalModel->setData(row, "vBC", vBC)) return false;
  if (not externalModel->setData(row, "pICMS", pICMS)) return false;
  if (not externalModel->setData(row, "vICMS", vICMS)) return false;
  if (not externalModel->setData(row, "modBCST", modBCST)) return false;
  if (not externalModel->setData(row, "pMVAST", pMVAST)) return false;
  if (not externalModel->setData(row, "vBCST", vBCST)) return false;
  if (not externalModel->setData(row, "pICMSST", pICMSST)) return false;
  if (not externalModel->setData(row, "vICMSST", vICMSST)) return false;
  if (not externalModel->setData(row, "cEnq", cEnq)) return false;
  if (not externalModel->setData(row, "cstIPI", cstIPI)) return false;
  if (not externalModel->setData(row, "cstPIS", cstPIS)) return false;
  if (not externalModel->setData(row, "vBCPIS", vBCPIS)) return false;
  if (not externalModel->setData(row, "pPIS", pPIS)) return false;
  if (not externalModel->setData(row, "vPIS", vPIS)) return false;
  if (not externalModel->setData(row, "cstCOFINS", cstCOFINS)) return false;
  if (not externalModel->setData(row, "vBCCOFINS", vBCCOFINS)) return false;
  if (not externalModel->setData(row, "pCOFINS", pCOFINS)) return false;
  if (not externalModel->setData(row, "vCOFINS", vCOFINS)) return false;
  if (not externalModel->setData(row, "status", "TEMP")) return false;

  return true;
}

bool XML::verificaCNPJ() {
  QSqlQuery queryLoja;

  if (not queryLoja.exec("SELECT cnpj FROM loja")) {
    QMessageBox::critical(0, "Erro!", "Erro na query CNPJ: " + queryLoja.lastError().text());
    return false;
  }

  QStringList list;

  while (queryLoja.next()) {
    list << queryLoja.value("cnpj").toString().remove(".").remove("/").remove("-");
  }

  if (not list.contains(cnpj)) {
    QMessageBox::critical(0, "Erro!", "CNPJ do destinatário difere do CNPJ da loja!");
    return false;
  }

  return true;
}

bool XML::verificaExiste() {
  QSqlQuery query;
  query.prepare("SELECT idNFe FROM nfe WHERE chaveAcesso = :chaveAcesso");
  query.bindValue(":chaveAcesso", chaveAcesso);

  if (not query.exec()) {
    QMessageBox::critical(0, "Erro!", "Erro verificando se nota já cadastrada: " + query.lastError().text());
    return false;
  }

  if (query.first()) {
    QMessageBox::critical(0, "Erro!", "Nota já cadastrada!");
    return true;
  }

  return false;
}

// NOTE: verificar se é possivel otimizar?
