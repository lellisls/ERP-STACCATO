#include <QFileDialog>
#include <QStyleFactory>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QMessageBox>

#include "src/xml.h"
#include "ui_xml.h"

XML::XML(QWidget *parent) : QDialog(parent), ui(new Ui::XML) {
  ui->setupUi(this);
  setWindowFlags(Qt::Window);

  //  darkTheme();

  ui->treeView->setModel(&model);
  ui->treeView->setUniformRowHeights(true);
  ui->treeView->setAnimated(true);
}

XML::~XML() { delete ui; }

void XML::importarXML() {
  fileName = QFileDialog::getOpenFileName(this, "Importar arquivo XML", QDir::currentPath(), tr("XML (*.xml)"));

  if (fileName.isEmpty()) {
    return;
  }

  fileName.replace("/", "\\\\");

  modelProduto.setTable("produto");
  modelProduto.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelProduto.select()) {
    qDebug() << "erro model produto: " << modelProduto.lastError();
    return;
  }

  readFile();
  saveXML();
}

void XML::exibirXML(QString file) {
  if (file.isEmpty()) {
    return;
  }

  QDomDocument document;

  if (not document.setContent(file)) {
    qDebug() << "erro setContent";
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

  ui->treeView->expandAll();
}

void XML::readFile() {
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

void XML::readTree(QStandardItem *item) {
  for (int i = 0; i < item->rowCount(); ++i) {
    for (int j = 0; j < item->columnCount(); ++j) {
      QStandardItem *child = item->child(i, j);

      if (child->text().left(6) == "infNFe") {
        idNFe = child->text().mid(child->text().indexOf("Id=") + 7, 44);
      }

      lerDadosProduto(child);
      lerICMSProduto(child);
      lerIPIProduto(child);
      lerIPIProduto(child);
      lerCOFINSProduto(child);
      lerTotais(child);

      if (child->hasChildren()) {
        readTree(child);
      }

      if (child->text().mid(0, 10) == "det nItem=") {
        if (insertProdutoEstoque()) {
          insertEstoque();
        }
      }
    }
  }
}

void XML::saveXML() {
  //--------------------------------------------------//
  QStandardItem *modelRoot = model.item(0, 0);
  //  qDebug() << modelRoot->text();
  if (modelRoot->hasChildren()) {
    readTree(modelRoot);
  }

  //  qDebug() << "item: " << itemNumero;
  //  qDebug() << "codProd: " << codProd;
  //  qDebug() << "codBarras: " << codBarras;
  //  qDebug() << "descricao: " << descricao;
  //  qDebug() << "ncm: " << ncm;
  //  qDebug() << "cfop: " << cfop;
  //  qDebug() << "un: " << un;
  //  qDebug() << "quant: " << quant;
  //  qDebug() << "valorUnid: " << valorUnid;
  //  qDebug() << "valor: " << valor;
  //  qDebug() << "codBarrasTrib: " << codBarrasTrib;
  //  qDebug() << "unTrib: " << unTrib;
  //  qDebug() << "quantTrib: " << quantTrib;
  //  qDebug() << "valorTrib: " << valorTrib;
  //  qDebug() << "desconto: " << desconto;
  //  qDebug() << "compoeTotal: " << compoeTotal;
  //  qDebug() << "numeroPed: " << numeroPedido;
  //  qDebug() << "itemPedido: " << itemPedido;

  //  QSqlQuery query;
  //  query.prepare(
  //        "INSERT INTO estoque (item, codProd, codBarras, descricao, ncm, cfop, un, quant, valorUnid, "
  //        "valor, codBarrasTrib, unTrib, quantTrib, valorTrib, desconto, compoeTotal, numeroPedido, itemPedido, "
  //        "tipoICMS, orig, cstICMS, modBC, vBC, pICMS, vICMS, modBCST, pMVAST, vBCST, pICMSST, vICMSST, cEnq, "
  //        "cstIPI, cstPIS, vBCPIS, pPIS, vPIS, cstCOFINS, vBCCOFINS, pCOFINS, vCOFINS, vBC_Total, vICMS_Total, "
  //        "vICMSDeson_Total, vBCST_Total, vST_Total, vProd_Total, vFrete_Total, vSeg_Total, vDesc_Total, "
  //        "vII_Total, vIPI_Total, vPIS_Total, vCOFINS_Total, vOutro_Total, vNF_Total) "
  //        "VALUES (:item, :codProd, :codBarras, :descricao, :ncm, :cfop, :un, :quant, :valorUnid, :valor, "
  //        ":codBarrasTrib, :unTrib, :quantTrib, :valorTrib, :desconto, :compoeTotal, :numeroPedido, "
  //        ":itemPedido, :tipoICMS, :orig, :cstICMS, :modBC, :vBC, :pICMS, :vICMS, :modBCST, :pMVAST, :vBCST, :pICMSST,
  //        ":vICMSST, :cEnq, :cstIPI, :cstPIS, :vBCPIS, :pPIS, :vPIS, :cstCOFINS, :vBCCOFINS, :pCOFINS, :vCOFINS, "
  //        ":vBC_Total, :vICMS_Total, :vICMSDeson_Total, :vBCST_Total, :vST_Total, :vProd_Total, :vFrete_Total, "
  //        ":vSeg_Total, :vDesc_Total, :vII_Total, :vIPI_Total, :vPIS_Total, :vCOFINS_Total, :vOutro_Total,
  //        ": vNF_Total) ");
  //  query.bindValue(":item", itemNumero);
  //  query.bindValue(":codProd", codProd);
  //  query.bindValue(":codBarras", codBarras);
  //  query.bindValue(":descricao", descricao);
  //  query.bindValue(":ncm", ncm);
  //  query.bindValue(":cfop", cfop);
  //  query.bindValue(":un", un);
  //  query.bindValue(":quant", quant);
  //  query.bindValue(":valorUnid", valorUnid);
  //  query.bindValue(":valor", valor);
  //  query.bindValue(":codBarrasTrib", codBarrasTrib);
  //  query.bindValue(":unTrib", unTrib);
  //  query.bindValue(":quantTrib", quantTrib);
  //  query.bindValue(":valorTrib", valorTrib);
  //  query.bindValue(":desconto", desconto);
  //  query.bindValue(":compoeTotal", compoeTotal);
  //  query.bindValue(":numeroPedido", numeroPedido);
  //  query.bindValue(":itemPedido", itemPedido);

  //  if (not query.exec()) {
  //    qDebug() << "erro: " << query.lastError();
  //  }

  //  qDebug() << "tipoICMS: " << tipoICMS;
  //  qDebug() << "orig: " << orig;
  //  qDebug() << "cst: " << cst;
  //  qDebug() << "modBC: " << modBC;
  //  qDebug() << "vBC: " << vBC;
  //  qDebug() << "pICMS: " << pICMS;
  //  qDebug() << "vICMS: " << vICMS;
  //  qDebug() << "modBCST: " << modBCST;
  //  qDebug() << "pMVAST: " << pMVAST;
  //  qDebug() << "vBCST: " << vBCST;
  //  qDebug() << "pICMSST: " << pICMSST;
  //  qDebug() << "vICMSST: " << vICMSST;

  //  qDebug() << "cstPIS: " << cstPIS;
  //  qDebug() << "vBC: " << vBCPIS;
  //  qDebug() << "pPIS: " << pPIS;
  //  qDebug() << "vPIS: " << vPIS;

  //  qDebug() << "cstCOFINS: " << cstCOFINS;
  //  qDebug() << "vBCCOFINS: " << vBCCOFINS;
  //  qDebug() << "pCOFINS: " << pCOFINS;
  //  qDebug() << "vCOFINS: " << vCOFINS;

  //  qDebug() << "vBC: " << vBC_Total;
  //  qDebug() << "vICMS: " << vICMS_Total;
  //  qDebug() << "vICMSDeson: " << vICMSDeson_Total;
  //  qDebug() << "vBCST: " << vBCST_Total;
  //  qDebug() << "vST: " << vST_Total;
  //  qDebug() << "vProd: " << vProd_Total;
  //  qDebug() << "vFrete: " << vFrete_Total;
  //  qDebug() << "vSeg: " << vSeg_Total;
  //  qDebug() << "vDesc: " << vDesc_Total;
  //  qDebug() << "vII: " << vII_Total;
  //  qDebug() << "vIPI: " << vIPI_Total;
  //  qDebug() << "vPIS: " << vPIS_Total;
  //  qDebug() << "vCOFINS: " << vCOFINS_Total;
  //  qDebug() << "vOutro: " << vOutro_Total;
  //  qDebug() << "vNF: " << vNF_Total;
}

void XML::darkTheme() {
  qApp->setStyle(QStyleFactory::create("Fusion"));

  QPalette darkPalette;
  darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::WindowText, Qt::white);
  darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
  darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
  darkPalette.setColor(QPalette::ToolTipText, Qt::white);
  darkPalette.setColor(QPalette::Text, Qt::white);
  darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::ButtonText, Qt::white);
  darkPalette.setColor(QPalette::BrightText, Qt::red);
  darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

  darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
  darkPalette.setColor(QPalette::HighlightedText, Qt::black);

  qApp->setPalette(darkPalette);

  qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
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
    //    qDebug() << "icms: " << child->child(0, 0)->text();
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

bool XML::insertProdutoEstoque() {
  QModelIndexList indexList =
      modelProduto.match(modelProduto.index(0, modelProduto.fieldIndex("codComercial")), Qt::DisplayRole, codProd);
  //      modelProduto.match(modelProduto.index(0, 0), Qt::DisplayRole, codProd);

  if (indexList.isEmpty()) {
    qDebug() << "produto nao cadastrado";
  } else {
    idProduto =
        modelProduto.data(modelProduto.index(indexList.first().row(), modelProduto.fieldIndex("idProduto"))).toInt();
  }

  //  qDebug() << "fileName: " << fileName;
  QSqlQuery queryEstoque;

  queryEstoque.prepare(
        "SELECT * FROM produto_has_estoque WHERE idNFe = :idNFe AND idProduto = :idProduto AND quant = :quant");
  queryEstoque.bindValue(":idNFe", idNFe);
  queryEstoque.bindValue(":idProduto", idProduto);
  queryEstoque.bindValue(":quant", quant);

  if (queryEstoque.exec() and queryEstoque.first()) {
    qDebug() << "nota já cadastrada";
    return false;
  }

  if (queryEstoque.exec("SELECT load_file('" + fileName + "')") and queryEstoque.first()) {
    if (queryEstoque.value(0).toString().isEmpty()) {
      QMessageBox::warning(this, "Aviso!", "Erro lendo xml no banco de dados.");
      return false;
    }
  }

  queryEstoque.prepare(
        "INSERT INTO produto_has_estoque (idProduto, quant, idNFe, xml) VALUES (:idProduto, :quant, :idNFe, load_file('" +
        fileName + "'))");
  queryEstoque.bindValue(":idProduto", idProduto);
  queryEstoque.bindValue(":quant", quant);
  queryEstoque.bindValue(":idNFe", idNFe);

  if (not queryEstoque.exec()) {
    qDebug() << "Erro cadastrando xml no estoque: " << queryEstoque.lastError();
    return false;
  }

  return true;
}

bool XML::insertEstoque() {
  qDebug() << "codProd: " << codProd;
  qDebug() << "idProduto: " << idProduto;

  QSqlQuery queryProd;
  queryProd.prepare("SELECT * FROM estoque WHERE idProduto = :idProduto");
  queryProd.bindValue(":idProduto", idProduto);

  if (not queryProd.exec()) {
    qDebug() << "Error: " << queryProd.lastError();
  }

  if (queryProd.first()) {
    queryProd.prepare("SELECT quant FROM estoque WHERE idProduto = :idProduto");
    queryProd.bindValue(":idProduto", idProduto);

    if (not queryProd.exec() or not queryProd.first()) {
      qDebug() << "Error: " << queryProd.lastError();
    }

    double old_quant = queryProd.value(0).toDouble();

    queryProd.prepare("UPDATE estoque SET quant = :quant WHERE idProduto = :idProduto");
    queryProd.bindValue(":quant", old_quant + quant);
    queryProd.bindValue(":idProduto", idProduto);

    if (not queryProd.exec()) {
      qDebug() << "Error: " << queryProd.lastError();
    }
  } else {
    queryProd.prepare("INSERT INTO estoque (idProduto, descricao, quant, un, codBarras, codComercial, m2cx) "
                      "VALUES (:idProduto, :descricao, :quant, :un, :codBarras, :codComercial, :m2cx)");
    queryProd.bindValue(":idProduto", idProduto);
    queryProd.bindValue(":descricao", descricao);
    queryProd.bindValue(":quant", quant);
    queryProd.bindValue(":un", un);
    queryProd.bindValue(":codBarras", codBarras);
    queryProd.bindValue(":codComercial", codProd);
    queryProd.bindValue(":m2cx", quant);

    if (not queryProd.exec()) {
      qDebug() << "Error: " << queryProd.lastError();
      return false;
    }
  }

  return true;
}

// TODO: nao deixar cadastrar uma mesma nota mais de uma vez (guardar o id da nota para comparacao)
// TODO: criar um lote novo para cada produto existente ao importar uma nota nova
