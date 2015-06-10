#include <QDate>
#include <QFile>
#include <QLocale>
#include <QMessageBox>
#include <QPrintPreviewDialog>
#include <QSqlError>
#include <QSqlQuery>
#include <QTableWidgetItem>
#include <QTextCodec>
#include <QTextDocument>
#include <QTime>
#include <QtMath>
#include <QtWebKitWidgets/QWebFrame>
#include <QtWebKitWidgets/QWebPage>
#include <QDir>

#include "cadastrocliente.h"
#include "mainwindow.h"
#include "orcamento.h"
#include "ui_orcamento.h"
#include "usersession.h"
#include "venda.h"
#include "searchdialog.h"
#include "apagaorcamento.h"
#include "endereco.h"
#include "doubledelegate.h"

Orcamento::Orcamento(QWidget *parent) : RegisterDialog("Orcamento", "idOrcamento", parent), ui(new Ui::Orcamento) {
  ui->setupUi(this);

  modelItem.setTable("Orcamento_has_Produto");
  modelItem.setHeaderData(modelItem.fieldIndex("produto"), Qt::Horizontal, "Produto");
  modelItem.setHeaderData(modelItem.fieldIndex("fornecedor"), Qt::Horizontal, "Fornecedor");
  modelItem.setHeaderData(modelItem.fieldIndex("obs"), Qt::Horizontal, "Obs.");
  modelItem.setHeaderData(modelItem.fieldIndex("prcUnitario"), Qt::Horizontal, "Preço/Un.");
  modelItem.setHeaderData(modelItem.fieldIndex("caixas"), Qt::Horizontal, "Caixas");
  modelItem.setHeaderData(modelItem.fieldIndex("qte"), Qt::Horizontal, "Qte.");
  modelItem.setHeaderData(modelItem.fieldIndex("un"), Qt::Horizontal, "Un.");
  modelItem.setHeaderData(modelItem.fieldIndex("codComercial"), Qt::Horizontal, "Código");
  modelItem.setHeaderData(modelItem.fieldIndex("formComercial"), Qt::Horizontal, "Formato");
  modelItem.setHeaderData(modelItem.fieldIndex("unCaixa"), Qt::Horizontal, "Un./Caixa");
  modelItem.setHeaderData(modelItem.fieldIndex("parcial"), Qt::Horizontal, "Subtotal");
  modelItem.setHeaderData(modelItem.fieldIndex("desconto"), Qt::Horizontal, "Desc. %");
  modelItem.setHeaderData(modelItem.fieldIndex("parcialDesc"), Qt::Horizontal, "Total");

  modelItem.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelItem.setFilter("idOrcamento = '" + ui->lineEditOrcamento->text() + "'");
  modelItem.select();

  ui->tableProdutos->setModel(&modelItem);
  ui->tableProdutos->setColumnHidden(modelItem.fieldIndex("idProduto"), true);
  ui->tableProdutos->setColumnHidden(modelItem.fieldIndex("idOrcamento"), true);
  ui->tableProdutos->setColumnHidden(modelItem.fieldIndex("idLoja"), true);
  ui->tableProdutos->setColumnHidden(modelItem.fieldIndex("item"), true);
  ui->tableProdutos->setColumnHidden(modelItem.fieldIndex("unCaixa"), true);
  ui->tableProdutos->setColumnHidden(modelItem.fieldIndex("descGlobal"), true);
  ui->tableProdutos->setColumnHidden(modelItem.fieldIndex("total"), true);

  DoubleDelegate *doubleDelegate = new DoubleDelegate(this);
  ui->tableProdutos->setItemDelegate(doubleDelegate);

  ui->tableProdutos->horizontalHeader()->setStretchLastSection(false);

  SearchDialog *sdCliente = SearchDialog::cliente(this);
  ui->itemBoxCliente->setSearchDialog(sdCliente);

  SearchDialog *sdProd = SearchDialog::produto(this);
  ui->itemBoxProduto->setSearchDialog(sdProd);

  RegisterDialog *cadCliente = new CadastroCliente(this);
  ui->itemBoxCliente->setRegisterDialog(cadCliente);

  SearchDialog *sdVendedor = SearchDialog::vendedor(this);
  ui->itemBoxVendedor->setSearchDialog(sdVendedor);

  SearchDialog *sdProfissional = SearchDialog::profissional(this);
  ui->itemBoxProfissional->setSearchDialog(sdProfissional);

  SearchDialog *sdEndereco = SearchDialog::enderecoCliente(this);
  ui->itemBoxEndereco->setSearchDialog(sdEndereco);

  setupMapper();
  newRegister();

  if (UserSession::getTipoUsuario() == "ADMINISTRADOR") {
    ui->dateTimeEdit->setReadOnly(false);
    ui->dateTimeEdit->setCalendarPopup(true);
    ui->checkBoxFreteManual->show();
  } else {
    ui->checkBoxFreteManual->hide();
  }

  ui->lineEditCodComercial->hide();
  ui->lineEditFormComercial->hide();
  ui->labelCodComercial->hide();
  ui->labelFormComercial->hide();

  ui->tableProdutos->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

  show();
  adjustSize();

  ui->tableProdutos->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableProdutos->horizontalHeader()->setResizeContentsPrecision(0);
  ui->tableProdutos->resizeColumnsToContents();
}

Orcamento::~Orcamento() { delete ui; }

void Orcamento::on_tableProdutos_clicked(const QModelIndex &index) {
  ui->pushButtonAtualizarItem->show();
  ui->pushButtonAdicionarItem->hide();
  mapperItem.setCurrentModelIndex(index);
}

bool Orcamento::viewRegister(QModelIndex index) {
  if (not RegisterDialog::viewRegister(index)) {
    return false;
  }

  QString idOrcamento = data(primaryKey).toString();
  modelItem.setFilter("idOrcamento = '" + idOrcamento + "'");
  modelItem.select();
  novoItem();

  calcPrecoGlobalTotal();

  QDateTime time = ui->dateTimeEdit->dateTime();
  if (time.addDays(data("validade").toInt()).date() < QDateTime::currentDateTime().date()) {
    ui->pushButtonGerarVenda->hide();
  } else {
    ui->pushButtonReplicar->hide();
  }

  ui->tableProdutos->resizeColumnsToContents();

  return true;
}

void Orcamento::novoItem() {
  ui->pushButtonAdicionarItem->show();
  ui->pushButtonAtualizarItem->hide();
  ui->itemBoxProduto->clear();
  ui->itemBoxProduto->setValue(QVariant());
  ui->tableProdutos->clearSelection();
  ui->tableProdutos->resizeColumnsToContents();
}

void Orcamento::setupMapper() {
  mapper.setModel(&model);

  addMapping(ui->lineEditOrcamento, "idOrcamento");
  addMapping(ui->itemBoxCliente, "idCliente", "value");
  addMapping(ui->itemBoxProfissional, "idProfissional", "value");
  addMapping(ui->itemBoxVendedor, "idUsuario", "value");
  addMapping(ui->itemBoxEndereco, "idEnderecoEntrega", "value");
  addMapping(ui->spinBoxValidade, "validade");
  addMapping(ui->doubleSpinBoxDescontoGlobal, "descontoPorc");
  addMapping(ui->doubleSpinBoxFrete, "frete");
  addMapping(ui->doubleSpinBoxFinal, "total");
  addMapping(ui->dateTimeEdit, "data");

  mapperItem.setModel(&modelItem);
  mapperItem.setSubmitPolicy(QDataWidgetMapper::ManualSubmit);

  mapperItem.addMapping(ui->lineEditPrecoUn, modelItem.fieldIndex("prcUnitario"), "value");
  mapperItem.addMapping(ui->lineEditPrecoTotal, modelItem.fieldIndex("total"), "value");
  mapperItem.addMapping(ui->lineEditOrcamento, modelItem.fieldIndex("idOrcamento"), "text");
  mapperItem.addMapping(ui->lineEditObs, modelItem.fieldIndex("obs"), "text");
  mapperItem.addMapping(ui->lineEditUn, modelItem.fieldIndex("un"), "text");
  mapperItem.addMapping(ui->lineEditCodComercial, modelItem.fieldIndex("codComercial"));
  mapperItem.addMapping(ui->lineEditFormComercial, modelItem.fieldIndex("formComercial"));
  mapperItem.addMapping(ui->itemBoxProduto, modelItem.fieldIndex("idProduto"), "value");
  mapperItem.addMapping(ui->doubleSpinBoxQte, modelItem.fieldIndex("qte"), "value");
  mapperItem.addMapping(ui->doubleSpinBoxDesconto, modelItem.fieldIndex("desconto"), "value");
}

void Orcamento::registerMode() {
  ui->pushButtonCadastrarOrcamento->show();
  ui->pushButtonAtualizarOrcamento->hide();

  ui->pushButtonImprimir->setEnabled(false);
  ui->pushButtonGerarVenda->setEnabled(true);
  ui->itemBoxEndereco->setEnabled(false);
}

void Orcamento::updateMode() {
  ui->pushButtonCadastrarOrcamento->hide();
  ui->pushButtonAtualizarOrcamento->show();

  ui->pushButtonImprimir->setEnabled(true);
  ui->pushButtonGerarVenda->setEnabled(true);
  ui->itemBoxEndereco->setVisible(true);
  ui->spinBoxValidade->setEnabled(false);
}

bool Orcamento::newRegister() {
  if (not RegisterDialog::newRegister()) {
    return false;
  }

  ui->pushButtonReplicar->hide();
  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
  ui->spinBoxValidade->setValue(7);
  novoItem();

  return true;
}

void Orcamento::removeItem() {
  modelItem.removeRow(ui->tableProdutos->currentIndex().row());
  calcPrecoGlobalTotal();
}

void Orcamento::updateId() {
  QSqlQuery queryIdExists("SELECT * FROM Orcamento WHERE idOrcamento = '" + ui->lineEditOrcamento->text() + "'");
  queryIdExists.exec();

  if (queryIdExists.size() != 0) {
    return;
  }

  QString id = UserSession::getSiglaLoja() + "-" + QDate::currentDate().toString("yy");
  QSqlQuery query("SELECT idOrcamento FROM Orcamento WHERE idOrcamento LIKE '" + id +
                  "%' UNION SELECT idVenda AS idOrcamento FROM Venda WHERE idVenda LIKE '" + id +
                  "%' ORDER BY idOrcamento ASC");
  int last = 0;

  if (query.size() > 0) {
    query.last();
    last = query.value("idOrcamento").toString().mid(id.size()).toInt();
  }

  id += QString("%1").arg(last + 1, 4, 10, QChar('0'));
  ui->lineEditOrcamento->setText(id);

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex(primaryKey)), id);
  }
}

bool Orcamento::verifyFields(int row) {
  Q_UNUSED(row)

  if (ui->itemBoxCliente->text().isEmpty()) {
    ui->itemBoxCliente->setFocus();
    QMessageBox::warning(this, "Atenção!", "Cliente inválido.", QMessageBox::Ok, QMessageBox::NoButton);
    return false;
  }

  if (ui->itemBoxVendedor->text().isEmpty()) {
    ui->itemBoxVendedor->setFocus();
    QMessageBox::warning(this, "Atenção!", "Vendedor inválido.", QMessageBox::Ok, QMessageBox::NoButton);
    return false;
  }

  if (ui->itemBoxProfissional->text().isEmpty()) {
    ui->itemBoxProfissional->setFocus();
    QMessageBox::warning(this, "Atenção!", "Profissional inválido.", QMessageBox::Ok, QMessageBox::NoButton);
    return false;
  }

  if (modelItem.rowCount() == 0) {
    ui->itemBoxProduto->setFocus();
    QMessageBox::warning(this, "Atenção!", "Você não pode cadastrar um orçamento sem itens.", QMessageBox::Ok,
                         QMessageBox::NoButton);
    return false;
  }

  return true;
}

bool Orcamento::savingProcedures(int row) {
  updateId();
  QLocale locale(QLocale::Portuguese);
  QString idOrcamento = ui->lineEditOrcamento->text();

  if (model.data(model.index(row, model.fieldIndex("idOrcamento"))).toString() != idOrcamento) {
    setData(row, "idOrcamento", idOrcamento);
  }

  setData(row, "idLoja", UserSession::getLoja());
  setData(row, "idCliente", ui->itemBoxCliente->value());
  setData(row, "idEnderecoEntrega", ui->itemBoxEndereco->value());
  setData(row, "idUsuario", ui->itemBoxVendedor->value());
  setData(row, "idProfissional", ui->itemBoxProfissional->value());
  setData(row, "validade", ui->spinBoxValidade->value());
  setData(row, "data", ui->dateTimeEdit->dateTime());
  setData(row, "subTotalBru", ui->doubleSpinBoxSubTotalBruto->value());
  setData(row, "subTotalLiq", ui->doubleSpinBoxTotal->value());
  setData(row, "frete", ui->doubleSpinBoxFrete->value());
  setData(row, "descontoPorc", ui->doubleSpinBoxDescontoGlobal->value());
  setData(row, "descontoReais", ui->doubleSpinBoxDescontoRS->value());
  setData(row, "total", ui->doubleSpinBoxFinal->value());

  if (not model.submitAll() and model.lastError().number() != 1062) {
    QMessageBox::warning(this, "Atenção!", "Não foi possível cadastrar este orçamento.", QMessageBox::Ok,
                         QMessageBox::NoButton);
    qDebug() << "SUBMITALL ERROR: " << model.lastError();
    qDebug() << "QUERY: " << model.query().lastQuery();
    return false;
  }

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    modelItem.setData(model.index(row, modelItem.fieldIndex(primaryKey)), idOrcamento);
    modelItem.setData(model.index(row, modelItem.fieldIndex("idLoja")), UserSession::getLoja());
  }

  if (not modelItem.submitAll()) {
    qDebug() << "Failed to add item! : " << modelItem.lastError().text();
    qDebug() << "QUERY: " << modelItem.query().lastQuery();
    QMessageBox::warning(this, "Atenção!", "Erro ao adicionar um item ao orçamento.", QMessageBox::Ok,
                         QMessageBox::NoButton);
    return false;
  }

  novoItem();
  viewRegisterById(idOrcamento);

  if (MainWindow *window = qobject_cast<MainWindow *>(parentWidget())) {
    window->updateTables();
  }

  return true;
}

void Orcamento::clearFields() {
  RegisterDialog::clearFields();
  ui->itemBoxVendedor->setValue(UserSession::getId());
  ui->itemBoxEndereco->setEnabled(false);
}

void Orcamento::on_pushButtonRemoverItem_clicked() { removeItem(); }

void Orcamento::calcPrecoItemTotal() {
  if (ui->itemBoxProduto->text().isEmpty()) {
    return;
  }

  double qte = ui->doubleSpinBoxQte->value();
  double prcUn = ui->lineEditPrecoUn->getValue();
  double desc = ui->doubleSpinBoxDesconto->value() / 100.0;
  double itemBruto = qte * prcUn;
  double subTotalItem = itemBruto * (1.0 - desc);

  ui->lineEditPrecoTotal->setValue(subTotalItem);
}

void Orcamento::on_doubleSpinBoxDesconto_valueChanged(double) { calcPrecoItemTotal(); }

void Orcamento::on_doubleSpinBoxQte_valueChanged(double) {
  double caixas = ui->doubleSpinBoxQte->value() / ui->doubleSpinBoxQte->singleStep();
  ui->spinBoxCaixas->setValue(caixas);
}

void Orcamento::on_doubleSpinBoxQte_editingFinished() {
  double step = ui->doubleSpinBoxQte->singleStep();
  double mult = ui->doubleSpinBoxQte->value() / step;
  ui->doubleSpinBoxQte->setValue(qCeil(mult) * step);
}

void Orcamento::on_pushButtonCadastrarOrcamento_clicked() { save(); }

void Orcamento::calcPrecoGlobalTotal(bool ajusteTotal) {
  subTotal = 0.0;
  subTotalItens = 0.0;
  double subTotalBruto = 0.0;
  double minimoFrete = 0, porcFrete = 0;

  QSqlQuery qryFrete;
  if (not qryFrete.exec("SELECT * FROM Loja WHERE idLoja = '" + QString::number(UserSession::getLoja()) + "'")) {
    qDebug() << "Erro buscando parâmetros do frete: " << qryFrete.lastError();
  }
  if (qryFrete.next()) {
    minimoFrete = qryFrete.value("valorMinimoFrete").toDouble();
    porcFrete = qryFrete.value("porcentagemFrete").toDouble();
  }

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    double prcUnItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("prcUnitario"))).toDouble();
    double qteItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("qte"))).toDouble();
    double descItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("desconto"))).toDouble() / 100.0;
    double itemBruto = qteItem * prcUnItem;
    subTotalBruto += itemBruto;
    double stItem = itemBruto * (1.0 - descItem);
    subTotalItens += stItem;
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("parcial")), itemBruto);  // Pr. Parcial
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("parcialDesc")), stItem); // Pr. Parcial Desc.
  }

  double frete = qMax(subTotalBruto * porcFrete / 100.0, minimoFrete);

  if (ui->checkBoxFreteManual->isChecked()) {
    frete = ui->doubleSpinBoxFrete->value();
    qDebug() << "novo frete: " << frete;
  }

  double descGlobal = ui->doubleSpinBoxDescontoGlobal->value() / 100.0;
  subTotal = subTotalItens * (1.0 - descGlobal);

  if (ajusteTotal) {
    const double Final = ui->doubleSpinBoxFinal->value();
    subTotal = Final - frete;
    if (subTotalItens == 0.0) {
      descGlobal = 0;
    } else {
      descGlobal = 1 - (subTotal / subTotalItens);
    }
  }

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    double stItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("parcialDesc"))).toDouble();
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("descGlobal")), descGlobal * 100.0); // Desconto
    // Distr.
    double totalItem = stItem * (1 - descGlobal);
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("total")), totalItem); // Pr. Final
  }

  ui->doubleSpinBoxSubTotalBruto->setValue(subTotalBruto);
  ui->doubleSpinBoxDescontoGlobal->setValue(descGlobal * 100);
  ui->doubleSpinBoxDescontoRS->setValue(subTotalItens - subTotal);
  ui->doubleSpinBoxFrete->setValue(frete);
  ui->doubleSpinBoxTotal->setValue(subTotalItens);
  ui->doubleSpinBoxFinal->setValue(subTotal + frete);
}

void Orcamento::on_doubleSpinBoxDescontoGlobal_valueChanged(double) {
  calcPrecoItemTotal();
  calcPrecoGlobalTotal();
}

void Orcamento::on_doubleSpinBoxFinal_editingFinished() {
  if (modelItem.rowCount() == 0 or subTotalItens == 0) {
    calcPrecoGlobalTotal();
    return;
  }

  double new_total = ui->doubleSpinBoxFinal->value();
  double frete = ui->doubleSpinBoxFrete->value();
  double new_subtotal = new_total - frete;

  if (new_subtotal >= subTotalItens) {
    ui->doubleSpinBoxDescontoGlobal->setValue(0.0);
    calcPrecoGlobalTotal();
  } else {
    calcPrecoGlobalTotal(true);
  }
}

void Orcamento::on_pushButtonImprimir_clicked() {
  QPrinter printer;
  //  printer.setFullPage(true);
  //  printer.setResolution(300);
  printer.setPageMargins(QMargins(300, 600, 300, 200), QPageLayout::Millimeter);
  printer.setOrientation(QPrinter::Portrait);
  printer.setPaperSize(QPrinter::A4);
  QPrintPreviewDialog preview(&printer, this, Qt::Window);
  preview.setModal(true);
  preview.setWindowTitle("Impressão de Orçamento");
  connect(&preview, &QPrintPreviewDialog::paintRequested, this, &Orcamento::print);
  preview.showMaximized();
  preview.exec();
}

QString Orcamento::itemData(int row, QString key) {
  return modelItem.data(modelItem.index(row, modelItem.fieldIndex(key))).toString();
}

QString Orcamento::getItensHtml() {
  QLocale locale(QLocale::Portuguese);
  QDir dir(QApplication::applicationDirPath());
  QFile file(dir.absoluteFilePath("itens.html"));
  QString html;

  if (file.open(QFile::ReadOnly)) {
    QByteArray data = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(data);
    html = codec->toUnicode(data);
    file.close();
  }

  QString itens = html;

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    itens.replace("#MARCA#", itemData(row, "fornecedor"));
    itens.replace("#CODIGO#", itemData(row, "idProduto"));
    itens.replace("#DESCRICAO#", itemData(row, "produto"));
    itens.replace("#AMBIENTE#", itemData(row, "obs"));
    itens.replace("#PRECOUN#", itemData(row, "prcUnitario"));
    itens.replace("#QTE#", locale.toString(itemData(row, "qte").toDouble()));
    itens.replace("#UN#", itemData(row, "un"));
    itens.replace("#TOTAL#", locale.toString(itemData(row, "total").toDouble()));
  }

  return itens;
}

void Orcamento::print(QPrinter *printer) {
  QWebPage *page = new QWebPage(this);
  QWebFrame *frame = page->mainFrame();
  QDir appDir(QApplication::applicationDirPath());
  QFile file(appDir.absoluteFilePath("orcamento.html"));
  QString html;

  if (file.open(QFile::ReadOnly)) {
    QByteArray data = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(data);
    html = codec->toUnicode(data);
    file.close();
  } else {
    QMessageBox::warning(this, "Aviso!", "Erro ao abrir orcamento.html");
    return;
  }

  QString str = "SELECT * FROM Loja WHERE idLoja = '" + QString::number(UserSession::getLoja()) + "';";
  QSqlQuery queryLoja(str);

  if (not queryLoja.exec(str)) {
    qDebug() << __FILE__ << ": ERROR IN QUERY: " << queryLoja.lastError();
  }

  queryLoja.first();

  // Loja
  html.replace("#LOGO#", QUrl::fromLocalFile(appDir.absoluteFilePath("logo.jpg")).toString());
  qDebug() << QUrl::fromLocalFile(appDir.absoluteFilePath("logo.jpg")).toString();
  //  html.replace("NOME FANTASIA", queryLoja.value("nomeFantasia").toString());
  //  html.replace("RAZAO SOCIAL", queryLoja.value("razaoSocial").toString());
  html.replace("#TELLOJA#", queryLoja.value("tel").toString());

  Endereco endLoja(queryLoja.value("idEndereco").toInt(), "Loja_has_Endereco");
  // End. Loja
  html.replace("#ENDLOJA01#", endLoja.linhaUm());
  html.replace("#ENDLOJA02#", endLoja.linhaDois());
  //  html.replace("TELLOJA",end);
  // Orcamento
  html.replace("#ORCAMENTO#", ui->lineEditOrcamento->text());
  html.replace("#DATA#", ui->dateTimeEdit->text());

  // Cliente
  str = "SELECT * FROM Cliente WHERE idCliente = '" + ui->itemBoxCliente->value().toString() + "';";
  QSqlQuery queryCliente(str);

  if (not queryCliente.exec()) {
    qDebug() << __FILE__ << ": ERROR IN QUERY: " << queryCliente.lastError();
  }

  queryCliente.first();
  html.replace("#NOME#", ui->itemBoxCliente->text());

  if (queryCliente.value("pfpj") == "PF") {
    html.replace("#CPFCNPJ#", queryCliente.value("cpf").toString());
  } else {
    html.replace("#CPFCNPJ#", queryCliente.value("cnpj").toString());
  }

  html.replace("#EMAILCLIENTE#", queryCliente.value("email").toString());
  html.replace("#TEL01#", queryCliente.value("tel").toString());
  html.replace("#TEL02#", queryCliente.value("telCel").toString());

  // End. Cliente
  Endereco endEntrega(data("idEnderecoEntrega").toInt(), "Cliente_has_Endereco");
  html.replace("#ENDENTREGA#", endEntrega.umaLinha());
  html.replace("#CEPENTREGA#", endEntrega.cep());

  // Profissional
  str = "SELECT * FROM Profissional WHERE idProfissional='" + ui->itemBoxProfissional->value().toString() + "'";
  QSqlQuery queryProf;

  if (not queryProf.exec(str)) {
    qDebug() << __FILE__ << ": ERROR IN QUERY: " << queryProf.lastError();
  }

  queryProf.first();
  html.replace("#NOMEPRO#", ui->itemBoxProfissional->text());
  html.replace("#TELPRO#", queryProf.value("tel").toString());
  html.replace("#EMAILPRO#", queryProf.value("email").toString());

  // Vendedor
  html.replace("#NOMEVEND#", ui->itemBoxVendedor->text());
  html.replace("#EMAILVEND#", "");
  // Itens
  QString itens = getItensHtml();
  html.replace("<!-- #ITENS# -->", itens);

  // Totais
  //  html.replace("SUBTOTAL", ui->doubleSpinBoxTotalFrete->text());
  html.replace("#SUBTOTALBRUTO#", ui->doubleSpinBoxSubTotalBruto->text());
  html.replace("#SUBTOTALLIQ#", ui->doubleSpinBoxTotal->text());
  html.replace("#DESCONTORS#", ui->doubleSpinBoxDescontoRS->text());
  html.replace("#FRETE#", ui->doubleSpinBoxFrete->text());
  html.replace("#TOTALFINAL#", ui->doubleSpinBoxFinal->text());

  // Prazos
  html.replace("#PRAZOENTREGA#", "A definir");
  html.replace("#FORMAPAGAMENTO#", "A definir");
  frame->setHtml(html);
  //  frame->setTextSizeMultiplier(1.2);
  frame->print(printer);
  QFile outputFile(appDir.absoluteFilePath("orc.html"));

  if (outputFile.open(QIODevice::WriteOnly)) {
    QTextStream out(&outputFile);
    out << html;
    outputFile.close();
  }
}

void Orcamento::adicionarItem() {
  calcPrecoItemTotal();

  if (ui->itemBoxProduto->text().isEmpty()) {
    QMessageBox::warning(this, "Atenção!", "Item inválido!", QMessageBox::Ok);
    return;
  }

  if (ui->doubleSpinBoxQte->value() == 0) {
    QMessageBox::warning(this, "Atenção!", "Quantidade inválida!", QMessageBox::Ok, QMessageBox::NoButton);
    return;
  }

  //  qDebug() << "rowCount: " << modelItem.rowCount();
  modelItem.insertRow(modelItem.rowCount());
  int row = modelItem.rowCount() - 1;

  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("idOrcamento")), ui->lineEditOrcamento->text());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("idLoja")), UserSession::getLoja());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("idProduto")), ui->itemBoxProduto->value().toInt());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("item")), row);
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("fornecedor")), ui->lineEditFornecedor->text());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("produto")), ui->itemBoxProduto->text());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("obs")), ui->lineEditObs->text());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("prcUnitario")), ui->lineEditPrecoUn->getValue());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("caixas")), ui->spinBoxCaixas->value());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("qte")), ui->doubleSpinBoxQte->value());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("unCaixa")), ui->doubleSpinBoxQte->singleStep());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("un")), ui->lineEditUn->text());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("codComercial")), ui->lineEditCodComercial->text());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("formComercial")), ui->lineEditFormComercial->text());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("desconto")), ui->doubleSpinBoxDesconto->value());

  calcPrecoGlobalTotal();
  novoItem();
}

void Orcamento::atualizarItem() {
  if (ui->itemBoxProduto->text().isEmpty()) {
    QMessageBox::warning(this, "Atenção!", "Item inválido!", QMessageBox::Ok, QMessageBox::NoButton);
    return;
  }

  if (ui->doubleSpinBoxQte->value() == 0) {
    QMessageBox::warning(this, "Atenção!", "Quantidade inválida!", QMessageBox::Ok, QMessageBox::NoButton);
    return;
  }

  int row = mapperItem.currentIndex();

  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("idOrcamento")), ui->lineEditOrcamento->text()); // Item
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("idLoja")), UserSession::getLoja());             // Item
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("idProduto")), ui->itemBoxProduto->value());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("item")), row); // Item
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("fornecedor")), ui->lineEditFornecedor->text());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("produto")), ui->itemBoxProduto->text());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("obs")), ui->lineEditObs->text()); // Obs
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("prcUnitario")), ui->lineEditPrecoUn->getValue());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("caixas")), ui->spinBoxCaixas->value());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("qte")), ui->doubleSpinBoxQte->value());          // Qte
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("unCaixa")), ui->doubleSpinBoxQte->singleStep()); // Un/Cx
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("un")), ui->lineEditUn->text());                  // Un
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("desconto")), ui->doubleSpinBoxDesconto->value());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("descGlobal")), ui->doubleSpinBoxDescontoGlobal->value());

  calcPrecoGlobalTotal();
  novoItem();
}

void Orcamento::on_pushButtonAdicionarItem_clicked() { adicionarItem(); }

void Orcamento::on_pushButtonAtualizarItem_clicked() {
  atualizarItem();
  ui->tableProdutos->clearSelection();
}

void Orcamento::on_pushButtonAtualizarOrcamento_clicked() { save(); }

void Orcamento::on_pushButtonGerarVenda_clicked() {
  if (not save(true)) {
    return;
  }

  QDateTime time = ui->dateTimeEdit->dateTime();

  if (not time.isValid()) {
    return;
  }

  if (time.addDays(data("validade").toInt()).date() < QDateTime::currentDateTime().date()) {
    QMessageBox::warning(this, "Aviso!", "Orçamento vencido!", QMessageBox::Ok);
    return;
  }

  if (ui->itemBoxEndereco->text().isEmpty()) {
    QMessageBox::warning(this, "Aviso!", "Deve selecionar endereço!", QMessageBox::Ok);
    ui->itemBoxEndereco->setFocus();
    return;
  }

  int idCliente = ui->itemBoxCliente->value().toInt();
  QSqlQuery qryCadastro;

  if (not qryCadastro.exec("SELECT incompleto FROM Orcamento LEFT JOIN Cliente ON Orcamento.idCliente "
                           "= Cliente.idCliente WHERE Cliente.idCliente = " +
                           QString::number(idCliente) + " AND incompleto = 1")) {
    qDebug() << "Erro verificando se cadastro do cliente está completo: " << qryCadastro.lastError();
    return;
  }

  if (qryCadastro.next()) {
    QMessageBox::warning(this, "Aviso!", "Cadastro incompleto, deve terminar.");
    RegisterDialog *cadCliente = new CadastroCliente(this);
    cadCliente->viewRegisterById(idCliente);
    return;
  }

  if (ui->itemBoxEndereco->text().isEmpty()) {
    QMessageBox::warning(this, "Aviso!", "Deve escolher um endereço.");
    return;
  }

  Venda *venda = new Venda(parentWidget());
  venda->fecharOrcamento(ui->lineEditOrcamento->text());
  close();
}

void Orcamento::on_doubleSpinBoxFrete_editingFinished() { calcPrecoGlobalTotal(); }

void Orcamento::on_pushButtonCancelar_clicked() { close(); }

void Orcamento::on_pushButtonCancelarItem_clicked() { novoItem(); }

void Orcamento::on_doubleSpinBoxTotal_valueChanged(double) { calcPrecoGlobalTotal(); }

void Orcamento::on_spinBoxCaixas_valueChanged(int caixas) {
  double qte = caixas * ui->doubleSpinBoxQte->singleStep();
  ui->doubleSpinBoxQte->setValue(qte);
  calcPrecoItemTotal();
}

void Orcamento::on_pushButtonApagarOrc_clicked() {
  ApagaOrcamento *apaga = new ApagaOrcamento(this);
  apaga->apagar(mapper.currentIndex());
}

void Orcamento::on_itemBoxProduto_textChanged(const QString &text) {
  Q_UNUSED(text);

  ui->doubleSpinBoxQte->setValue(0.0);
  ui->spinBoxCaixas->setValue(0.0);
  ui->doubleSpinBoxDesconto->setValue(0.0);

  if (ui->itemBoxProduto->text().isEmpty()) {
    ui->spinBoxCaixas->setDisabled(true);
    ui->doubleSpinBoxQte->setDisabled(true);
    ui->doubleSpinBoxDesconto->setDisabled(true);
    ui->doubleSpinBoxQte->setSingleStep(1.0);
    ui->lineEditFornecedor->clear();
    ui->lineEditPrecoTotal->clear();
    ui->lineEditPrecoUn->clear();
    ui->lineEditPrecoUn->setDisabled(true);
    ui->lineEditPrecoTotal->setDisabled(true);
    ui->lineEditUn->clear();
    ui->lineEditObs->clear();
    return;
  }

  QSqlQuery query;
  query.prepare("SELECT * FROM Produto WHERE idProduto = :idx");
  query.bindValue(":idx", ui->itemBoxProduto->value().toInt());

  if (not query.exec()) {
    qDebug() << "Erro na busca do produto: " << query.lastError();
  }

  query.first();
  QString un = query.value("un").toString();
  ui->lineEditUn->setText(un);
  ui->lineEditPrecoUn->setValue(query.value("precoVenda").toDouble());
  ui->lineEditEstoque->setValue(query.value("estoque").toInt());
  ui->lineEditFornecedor->setText(query.value("fornecedor").toString());
  ui->lineEditCodComercial->setText(query.value("codComercial").toString());
  ui->lineEditFormComercial->setText(query.value("formComercial").toString());

  ui->spinBoxCaixas->setEnabled(true);
  ui->doubleSpinBoxQte->setEnabled(true);
  ui->spinBoxCaixas->setEnabled(true);
  ui->doubleSpinBoxDesconto->setEnabled(true);
  ui->lineEditPrecoUn->setEnabled(true);
  ui->lineEditPrecoTotal->setEnabled(true);

  if (un.contains("m2") or un.contains("ml")) { // TODO: refactor this to use "m2/ml else pç"
    ui->doubleSpinBoxQte->setSingleStep(query.value("m2cx").toDouble());
  } else if (un.contains("pç") or un.contains("pc")) {
    ui->doubleSpinBoxQte->setSingleStep(query.value("pccx").toDouble());
  }

  ui->doubleSpinBoxQte->setValue(0);

  calcPrecoItemTotal();
}

void Orcamento::on_itemBoxCliente_textChanged(const QString &text) {
  Q_UNUSED(text);

  ui->itemBoxEndereco->searchDialog()->setFilter("idCliente = " + QString::number(ui->itemBoxCliente->value().toInt()) +
                                                 " AND desativado = false OR idEndereco = 1");

  QSqlQuery queryCliente;
  queryCliente.prepare("SELECT idProfissionalRel FROM Cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", ui->itemBoxCliente->value());

  if (not queryCliente.exec() or !queryCliente.first()) {
    qDebug() << "Erro ao buscar cliente: " << queryCliente.lastError();
  }

  ui->itemBoxProfissional->setValue(queryCliente.value("idProfissionalRel"));
  ui->itemBoxEndereco->setEnabled(true);
  ui->itemBoxEndereco->clear();
}

void Orcamento::successMessage() {
  QMessageBox::information(this, "Atenção!", "Orçamento atualizado com sucesso!", QMessageBox::Ok,
                           QMessageBox::NoButton);
}

void Orcamento::on_pushButtonLimparSelecao_clicked() {
  novoItem();
}

void Orcamento::on_checkBoxFreteManual_clicked(bool checked) {
  if (checked == true and UserSession::getTipoUsuario() != "ADMINISTRADOR") {
    ui->checkBoxFreteManual->setChecked(false);
    return;
  }

  ui->doubleSpinBoxFrete->setFrame(checked);
  ui->doubleSpinBoxFrete->setReadOnly(not checked);

  if (checked) {
    ui->doubleSpinBoxFrete->setButtonSymbols(QDoubleSpinBox::UpDownArrows);
  } else {
    ui->doubleSpinBoxFrete->setButtonSymbols(QDoubleSpinBox::NoButtons);
  }

  calcPrecoGlobalTotal();
}

void Orcamento::on_pushButtonReplicar_clicked() {
  QString lastOrc = ui->lineEditOrcamento->text();

  ui->lineEditOrcamento->clear();
  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
  newRegister();

  updateId();
  QLocale locale(QLocale::Portuguese);

  save();
}
