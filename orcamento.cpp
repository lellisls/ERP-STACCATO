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
#include "cadastrarcliente.h"
#include "mainwindow.h"
#include "orcamento.h"
#include "ui_orcamento.h"
#include "usersession.h"
#include "venda.h"
#include "searchdialog.h"
#include "apagaorcamento.h"

Orcamento::Orcamento(QWidget *parent)
  : RegisterDialog("Orcamento", "idOrcamento", parent), ui(new Ui::Orcamento) {
  QTime time;
  time.start();
  ui->setupUi(this);

  ui->tableProdutos->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tableProdutos->verticalHeader()->setResizeContentsPrecision(5);
  ui->tableProdutos->horizontalHeader()->setResizeContentsPrecision(5);
  ui->tableProdutos->horizontalHeader()->setStretchLastSection(true);

  modelItem.setTable("Orcamento_has_Produto");
  modelItem.setHeaderData(modelItem.fieldIndex("produto"), Qt::Horizontal, "Produto");
  modelItem.setHeaderData(modelItem.fieldIndex("fornecedor"), Qt::Horizontal, "Fornecedor");
  modelItem.setHeaderData(modelItem.fieldIndex("obs"), Qt::Horizontal, "Obs.");
  modelItem.setHeaderData(modelItem.fieldIndex("prcUnitario"), Qt::Horizontal, "Preço/Un.");
  modelItem.setHeaderData(modelItem.fieldIndex("caixas"), Qt::Horizontal, "Caixas");
  modelItem.setHeaderData(modelItem.fieldIndex("qte"), Qt::Horizontal, "Qte.");
  modelItem.setHeaderData(modelItem.fieldIndex("un"), Qt::Horizontal, "Un.");
  modelItem.setHeaderData(modelItem.fieldIndex("unCaixa"), Qt::Horizontal, "Un./Caixa");
  modelItem.setHeaderData(modelItem.fieldIndex("parcial"), Qt::Horizontal, "R$");
  modelItem.setHeaderData(modelItem.fieldIndex("desconto"), Qt::Horizontal, "Desconto");
  modelItem.setHeaderData(modelItem.fieldIndex("parcialDesc"), Qt::Horizontal, "R$ + Desc.");
  modelItem.setHeaderData(modelItem.fieldIndex("total"), Qt::Horizontal, "Preço");

  modelItem.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelItem.setFilter("idOrcamento = '" + ui->lineEditOrcamento->text() + "'");
  modelItem.select();

  ui->tableProdutos->setModel(&modelItem);
  ui->tableProdutos->setColumnHidden(modelItem.fieldIndex("idProduto"), true);
  ui->tableProdutos->setColumnHidden(modelItem.fieldIndex("idOrcamento"), true);
  ui->tableProdutos->setColumnHidden(modelItem.fieldIndex("idLoja"), true);
  ui->tableProdutos->setColumnHidden(modelItem.fieldIndex("item"), true);
  ui->tableProdutos->setColumnHidden(modelItem.fieldIndex("descGlobal"), true);

  SearchDialog *sdCliente = SearchDialog::cliente(ui->itemBoxCliente);
  ui->itemBoxCliente->setSearchDialog(sdCliente);

  SearchDialog *sdProd = SearchDialog::produto(ui->itemBoxProduto);

  ui->itemBoxProduto->setSearchDialog(sdProd);

  RegisterDialog *cadCliente = new CadastroCliente(this);
  ui->itemBoxCliente->setRegisterDialog(cadCliente);

  QSqlQuery qryFrete;
  if (!qryFrete.exec("SELECT * FROM Loja WHERE idLoja = '" + QString::number(UserSession::getLoja()) + "'")) {
    qDebug() << "Erro buscando parâmetros do frete: " << qryFrete.lastError();
  }
  qDebug() << "qry: " << qryFrete.lastQuery();
  if (qryFrete.next()) {
    minimoFrete = qryFrete.value("valorMinimoFrete").toDouble();
    porcFrete = qryFrete.value("porcentagemFrete").toDouble();
  }

  qDebug() << "minimoFrete: " << minimoFrete;
  qDebug() << "%frete: " << porcFrete;

  fillComboBoxes();

  setupMapper();
  newRegister();
  show();
  qDebug() << "time: " << time.elapsed();
}

Orcamento::~Orcamento() {
  delete ui;
}

void Orcamento::on_tableProdutos_clicked(const QModelIndex &index) {
  ui->pushButtonAtualizarItem->show();
  ui->pushButtonAdicionarItem->hide();
  //  qDebug() << "before click: " << mapperItem.currentIndex();
  mapperItem.setCurrentModelIndex(index);
  //  qDebug() << "after click: " << mapperItem.currentIndex();
}

bool Orcamento::viewRegister(QModelIndex index) {
  if (!RegisterDialog::viewRegister(index)) {
    return false;
  }
  QString idOrcamento = data(primaryKey).toString();
  modelItem.setFilter("idOrcamento = '" + idOrcamento + "'");
  modelItem.select();
  novoItem();

  calcPrecoGlobalTotal();
  return true;
}

void Orcamento::fillComboBoxes() {
  ui->comboBoxProfissional->clear();
  ui->comboBoxProfissional->addItem("Escolha uma opção!");
  QSqlQuery queryProf("SELECT idProfissional, nome, tipo FROM Profissional;");
  queryProf.exec();
  while (queryProf.next()) {
    QString str;
    if(queryProf.value(2).toString().isEmpty()) {
      str = queryProf.value(1).toString();
    } else {
      str = queryProf.value(1).toString() + " [" + queryProf.value(2).toString() + "] ";
    }
    ui->comboBoxProfissional->addItem(str, queryProf.value(0));
  }
  ui->comboBoxVendedor->clear();
  ui->comboBoxVendedor->addItem("Escolha uma opção!");
  //  QSqlQuery queryVend("SELECT idUsuario, nome FROM Usuario WHERE tipo = 'VENDEDOR';");
  QSqlQuery queryVend("SELECT idUsuario, nome FROM Usuario");
  queryVend.exec();
  while (queryVend.next()) {
    QString str = queryVend.value(0).toString() + " - " + queryVend.value(1).toString();
    ui->comboBoxVendedor->addItem(str, queryVend.value(0));
  }
  if (!ui->comboBoxVendedor->setCurrentValue(UserSession::getId())) {
    ui->comboBoxVendedor->setEnabled(true);
  } else {
    ui->comboBoxVendedor->setDisabled(true);
  }
}

void Orcamento::novoItem() {
  ui->pushButtonAdicionarItem->show();
  ui->pushButtonAtualizarItem->hide();
  ui->itemBoxProduto->clear();
  ui->itemBoxProduto->setValue(QVariant());
  //  mapperItem.setCurrentIndex(-1);
  //  modelItem.insertRow(modelItem.rowCount());
  //  mapperItem.toLast();
}

void Orcamento::setupMapper() {
  mapper.setModel(&model);

  addMapping(ui->lineEditOrcamento, "idOrcamento");
  addMapping(ui->itemBoxCliente, "idCadastroCliente", "value");
  addMapping(ui->comboBoxProfissional, "idProfissional", "currentValue");
  addMapping(ui->comboBoxVendedor, "idUsuario", "currentValue");
  addMapping(ui->comboBoxEndereco, "idEnderecoEntrega", "currentValue");
  addMapping(ui->spinBoxValidade, "validade");
  addMapping(ui->doubleSpinBoxDescontoGlobal, "desconto");
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

  mapperItem.addMapping(ui->itemBoxProduto, modelItem.fieldIndex("idProduto"), "value");
  mapperItem.addMapping(ui->doubleSpinBoxQte, modelItem.fieldIndex("qte"), "value");
  mapperItem.addMapping(ui->doubleSpinBoxDesconto, modelItem.fieldIndex("desconto"), "value");
}

void Orcamento::registerMode() {
  ui->pushButtonCadastrarOrcamento->show();
  ui->pushButtonAtualizarOrcamento->hide();

  ui->pushButtonImprimir->setEnabled(false);
  ui->pushButtonFecharPedido->setEnabled(true);
  ui->itemBoxCliente->setEnabled(true);
  ui->comboBoxProfissional->setEnabled(true);
  ui->comboBoxVendedor->setEnabled(true);
}

void Orcamento::updateMode() {
  ui->pushButtonCadastrarOrcamento->hide();
  ui->pushButtonAtualizarOrcamento->show();

  ui->pushButtonImprimir->setEnabled(true);
  ui->pushButtonFecharPedido->setEnabled(true);
  ui->itemBoxCliente->setDisabled(true);
  ui->comboBoxProfissional->setDisabled(true);
  ui->comboBoxVendedor->setDisabled(true);
}

bool Orcamento::newRegister() {
  if (!RegisterDialog::newRegister()) {
    return false;
  }
  updateId();
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
  QString str = "SELECT * FROM Orcamento WHERE idOrcamento = '" + ui->lineEditOrcamento->text() + "';";
  QSqlQuery queryIdExists(str);
  queryIdExists.exec();
  if (queryIdExists.size() != 0) {
    return;
  }
  str =
    "SELECT sigla FROM Usuario WHERE idUsuario = '" + ui->comboBoxVendedor->currentData().toString() + "';";
  QSqlQuery queryUsuario(str);
  QString siglaUser = "AAA";
  if (queryUsuario.first()) {
    siglaUser = queryUsuario.value(0).toString();
  }
  QString id =
    UserSession::getSiglaLoja() + "-" + siglaUser + "-" + QDate::currentDate().toString("yyMMdd") + "-";
  str = "SELECT idOrcamento FROM Orcamento WHERE idOrcamento LIKE '" + id + "%'";
  str += "UNION SELECT idVenda AS idOrcamento FROM Venda WHERE idVenda LIKE '" + id + "%'";
  QSqlQuery query(str);
  int last = 0;
  while (query.next()) {
    QString subStr = query.value("idOrcamento").toString().mid(id.size());
    bool ok;
    last = std::max(subStr.toInt(&ok, 16), last);
  }
  id += QString("%1").arg(last + 1, 3, 16, QChar('0'));
  ui->lineEditOrcamento->setText(id);
}

bool Orcamento::verifyFields() {
  // TODO : VErifyFields Orçamento
  //  if(!RegisterDialog::verifyFields())
  //    return false;
  if (ui->itemBoxCliente->text().isEmpty()) {
    ui->itemBoxCliente->setFocus();
    QMessageBox::warning(this, "Atenção!", "Cliente inválido.", QMessageBox::Ok, QMessageBox::NoButton);
    return false;
  }

  if (ui->comboBoxVendedor->currentData().isNull()) {
    ui->comboBoxVendedor->setFocus();
    QMessageBox::warning(this, "Atenção!", "Vendedor inválido.", QMessageBox::Ok, QMessageBox::NoButton);
    return false;
  }

  if (ui->comboBoxProfissional->currentData().isNull()) {
    ui->comboBoxVendedor->setFocus();
    QMessageBox::warning(this, "Atenção!", "Profissional inválido.", QMessageBox::Ok, QMessageBox::NoButton);
    return false;
  }

  if (modelItem.rowCount() == 0) {
    //    ui->comboBoxProduto->setFocus();
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
  if (model.data(model.index(row, model.fieldIndex("idOrcamento"))).toString() != idOrcamento)
    setData(row, "idOrcamento", idOrcamento);
  setData(row, "idLoja", UserSession::getLoja());
  qDebug() << ui->itemBoxCliente->getValue();
  setData(row, "idCadastroCliente", ui->itemBoxCliente->getValue());
  setData(row, "idEnderecoEntrega", ui->comboBoxEndereco->currentData()); // get user from userSession
  setData(row, "idUsuario", ui->comboBoxVendedor->currentData());         // get user from userSession
  setData(row, "idProfissional", ui->comboBoxProfissional->currentData());
  setData(row, "validade", ui->spinBoxValidade->value());
  setData(row, "data", ui->dateTimeEdit->dateTime());
  setData(row, "total", ui->doubleSpinBoxFinal->value());
  setData(row, "desconto", ui->doubleSpinBoxDescontoGlobal->value());
  //  modelOrc.setData(modelOrc.index(mapperOrc.currentIndex(), modelOrc.fieldIndex("desconto",
  //                   ui->doubleSpinBoxDescontoGlobal->value());
  // TODO: setando desconto duas vezes?

  setData(row, "frete", ui->doubleSpinBoxFrete->value());

  QSqlQuery("START TRANSACTION").exec();
  if (!model.submitAll() && model.lastError().number() != 1062) {
    QMessageBox::warning(this, "Atenção!", "Não foi possível cadastrar este orçamento.", QMessageBox::Ok,
                         QMessageBox::NoButton);
    qDebug() << "SUBMITALL ERROR: " << model.lastError();
    qDebug() << "QUERY: " << model.query().lastQuery();
    return false;
  }

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    modelItem.setData(model.index(row, modelItem.fieldIndex("idOrcamento")), idOrcamento);
    modelItem.setData(model.index(row, modelItem.fieldIndex("idLoja")), UserSession::getLoja());
  }

  if (!modelItem.submitAll()) {
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
  ui->comboBoxEndereco->clear();
  ui->comboBoxEndereco->addItem("Escolha uma opção!");
}

void Orcamento::on_pushButtonRemoverItem_clicked() {
  removeItem();
}

void Orcamento::calcPrecoItemTotal() {
  //  if (ui->comboBoxProduto->currentData().isNull()) {
  //    return;
  //  }
  if (ui->itemBoxProduto->text().isEmpty()) {
    return;
  }

  double qte = ui->doubleSpinBoxQte->value();
  //  double qteCx = ui->doubleSpinBoxQte->singleStep();
  double prcUn = ui->lineEditPrecoUn->getValue();
  double desc = ui->doubleSpinBoxDesconto->value() / 100.0;
  double descGlobal = ui->doubleSpinBoxDescontoGlobal->value() / 100.0;
  double parcial = qte * prcUn;
  double parcialDesc = parcial * (1.0 - desc);
  double total = parcialDesc * (1.0 - descGlobal);

  ui->lineEditPrecoTotal->setValue(total);
}

void Orcamento::on_doubleSpinBoxDesconto_valueChanged(double) {
  calcPrecoItemTotal();
}

void Orcamento::on_doubleSpinBoxQte_valueChanged(double) {
  //  calcPrecoItemTotal();
  double caixas = ui->doubleSpinBoxQte->value() / ui->doubleSpinBoxQte->singleStep();
  ui->doubleSpinBoxCaixas->setValue(caixas);
}

void Orcamento::on_doubleSpinBoxQte_editingFinished() {
  double step = ui->doubleSpinBoxQte->singleStep();
  double mult = ui->doubleSpinBoxQte->value() / step;
  ui->doubleSpinBoxQte->setValue(qCeil(mult) * step);
}

void Orcamento::on_pushButtonCadastrarOrcamento_clicked() {
  save();
}

void Orcamento::on_comboBoxVendedor_currentIndexChanged(int) {
  updateId();
}

void Orcamento::calcPrecoGlobalTotal() {
  subTotal = 0.0;
  subTotalItens = 0.0;
  double bruto = 0.0;
  for (int row = 0; row < modelItem.rowCount(); ++row) {
    double prcUnItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("prcUnitario"))).toDouble();
    double qteItem   = modelItem.data(modelItem.index(row, modelItem.fieldIndex("qte"))).toDouble();
    double descItem  = modelItem.data(modelItem.index(row, modelItem.fieldIndex("desconto"))).toDouble() / 100.0;
    double descGlobal = ui->doubleSpinBoxDescontoGlobal->value() / 100.0;
    double parcialItem = qteItem * prcUnItem;
    bruto += parcialItem;
    double parcialItemDesc = parcialItem * (1.0 - descItem);
    double totalItem = parcialItemDesc * (1.0 - descGlobal);
    subTotal += totalItem;
    subTotalItens += parcialItemDesc;
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("parcial")), parcialItem); // Pr. Parcial
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("parcialDesc")),
                      parcialItemDesc); // Pr. Parcial Desc.
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("descGlobal")), descGlobal); // Desconto
    // Distr.
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("total")), totalItem); // Pr. Final
  }
  double frete = ui->doubleSpinBoxFrete->value();
  if (ui->checkBoxCalculaFrete->isChecked()) {
    frete = ui->doubleSpinBoxTotal->value() * (porcFrete / 100);
    if (frete < minimoFrete) {
      frete = minimoFrete;
    }
  }

  ui->doubleSpinBoxFrete->setValue(frete);
  ui->doubleSpinBoxTotal->setValue(bruto);
  ui->doubleSpinBoxTotalFrete->setValue(bruto+frete);
  ui->doubleSpinBoxDescontoRS->setValue(bruto - subTotal);
  ui->doubleSpinBoxFinal->setValue(subTotal + frete);
}

void Orcamento::on_doubleSpinBoxDescontoGlobal_valueChanged(double) {
  calcPrecoItemTotal();
  calcPrecoGlobalTotal();
}

void Orcamento::on_doubleSpinBoxFinal_editingFinished() {
  if (modelItem.rowCount() == 0 || subTotalItens == 0) {
    calcPrecoGlobalTotal();
    return;
  }

  double new_total = ui->doubleSpinBoxFinal->value();
  double frete = ui->doubleSpinBoxFrete->value();
  double new_subtotal = new_total - frete;
  double descGlobal = 0.0;

  qDebug() << "New total = " << new_total << ", frete = " << frete << ", new sub. = " << new_subtotal
           << ", subTotalItens = " << subTotalItens;
  if (new_subtotal < subTotalItens) {
    double dif = subTotalItens - new_subtotal;
    descGlobal = qMax((dif / subTotalItens) * 100.0, 0.0);
  }
  bool calcFreteChecked = ui->checkBoxCalculaFrete->isChecked();
  ui->checkBoxCalculaFrete->setChecked(false);
  ui->doubleSpinBoxDescontoGlobal->setValue(descGlobal);

  calcPrecoItemTotal();
  calcPrecoGlobalTotal();
  ui->checkBoxCalculaFrete->setChecked(calcFreteChecked);
  //  ui->doubleSpinBoxTotal->setValue(new_total);
}

void Orcamento::on_pushButtonImprimir_clicked() {
  QPrinter printer(QPrinter::HighResolution);
  printer.setPageMargins(QMargins(30, 60, 30, 20));
  QPrintPreviewDialog preview(&printer, this, Qt::Window);
  preview.setModal(true);
  connect(&preview, &QPrintPreviewDialog::paintRequested, this, &Orcamento::print);
  preview.showMaximized();
  preview.exec();
}

QString Orcamento::getItensHtml() {
  QLocale locale(QLocale::Portuguese);
  QString itens;
  for (int row = 0; row < modelItem.rowCount(); ++row) {
    itens += "<tr>";
    itens += "  <td>" + modelItem.data(modelItem.index(row, modelItem.fieldIndex("item"))).toString() +
             "</td>"; // item
    itens += "  <td>" + modelItem.data(modelItem.index(row, modelItem.fieldIndex("produto"))).toString() +
             "</td>"; // Produto
    itens += "  <td>" + modelItem.data(modelItem.index(row, modelItem.fieldIndex("obs"))).toString() +
             "</td>"; // Obs
    itens +=
      "  <td>" +
      locale.toString(modelItem.data(modelItem.index(row, modelItem.fieldIndex("prcUnitario"))).toDouble(),
                      'f', 2) +
      "</td>"; // Prc. Un
    itens += "  <td>" +
             locale.toString(modelItem.data(modelItem.index(row, modelItem.fieldIndex("qte"))).toDouble(),
                             'f', 2) +
             "</td>"; // Qte
    itens += "  <td>" + modelItem.data(modelItem.index(row, modelItem.fieldIndex("un"))).toString() +
             "</td>"; // Un
    itens += "  <td>" +
             locale.toString(modelItem.data(modelItem.index(row, modelItem.fieldIndex("parcial"))).toDouble(),
                             'f', 2) +
             "</td>"; // Pr. Parcial
    itens += "  <td>" +
             locale.toString(
               modelItem.data(modelItem.index(row, modelItem.fieldIndex("desconto"))).toDouble(), 'f', 2) +
             "</td>"; // Desconto
    itens +=
      "  <td>" +
      locale.toString(modelItem.data(modelItem.index(row, modelItem.fieldIndex("descGlobal"))).toDouble(),
                      'f', 2) +
      "</td>"; // Desconto Disr.
    itens += "  <td>" +
             locale.toString(modelItem.data(modelItem.index(row, modelItem.fieldIndex("total"))).toDouble(),
                             'f', 2) +
             "</td>"; // Total
    itens += "</tr>";
  }
  return itens;
}

void Orcamento::print(QPrinter *printer) {
  QWebPage *page = new QWebPage(this);
  QWebFrame *frame = page->mainFrame();
  QFile file(":/orcamento.html");
  QString html;
  if (file.open(QFile::ReadOnly)) {
    QByteArray data = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(data);
    html = codec->toUnicode(data);
    file.close();
  }
  QString str = "SELECT * FROM Loja WHERE idLoja = '" + QString::number(UserSession::getLoja()) + "';";
  QSqlQuery queryLoja(str);
  if (!queryLoja.exec(str)) {
    qDebug() << __FILE__ << ": ERROR IN QUERY: " << queryLoja.lastError();
  }
  queryLoja.first();

  //  html.replace("LOGO", "");
  html.replace("LOGO", QUrl::fromLocalFile(QDir::home().filePath("logo.png")).toString());

  html.replace("NOME FANTASIA", queryLoja.value("nomeFantasia").toString());
  html.replace("RAZAO SOCIAL", queryLoja.value("razaoSocial").toString());
  html.replace("TELEFONE", queryLoja.value("tel").toString());

  str = "SELECT * FROM Endereco WHERE idEndereco='" + queryLoja.value("idEndereco").toString() + "';";
  QSqlQuery queryEnd(str);
  if (!queryEnd.exec(str)) {
    qDebug() << __FILE__ << ": ERROR IN QUERY: " << queryEnd.lastError();
  }
  queryEnd.first();

  html.replace("LOGRADOURO", queryEnd.value("logradouro").toString());
  html.replace("NUMERO", queryEnd.value("numero").toString());
  html.replace("BAIRRO", queryEnd.value("bairro").toString());
  html.replace("CIDADE", queryEnd.value("cidade").toString());
  html.replace("UF", queryEnd.value("uf").toString());

  html.replace("IDORCAMENTO", ui->lineEditOrcamento->text());
  html.replace("VENDEDOR", ui->comboBoxVendedor->currentText());
  html.replace("CLIENTE", ui->itemBoxCliente->text());
  html.replace("DATA", ui->dateTimeEdit->text());

  QString itens = getItensHtml();
  html.replace("ITENS", itens);

  html.replace("SUBTOTAL", ui->doubleSpinBoxTotalFrete->text());
  html.replace("DESCONTO", ui->doubleSpinBoxDescontoGlobal->text());
  html.replace("FRETE", ui->doubleSpinBoxFrete->text());
  html.replace("TOTAL", ui->doubleSpinBoxFinal->text());

  frame->setHtml(html);
  qDebug() << html;
  //  frame->setTextSizeMultiplier(1.2);
  frame->print(printer);

  QFile outputFile(QDir::home().absoluteFilePath("orc.html"));
  if (outputFile.open(QIODevice::WriteOnly)) {
    QTextStream out(&outputFile);
    out << html;
    outputFile.close();
  }
}

void Orcamento::adicionarItem() {
  //  if (ui->comboBoxProduto->currentData().isNull()) {
  //    QMessageBox::warning(this, "Atenção!", "Item inválido!", QMessageBox::Ok, QMessageBox::NoButton);
  //    return;
  //  }
  if (ui->itemBoxProduto->text().isEmpty()) {
    QMessageBox::warning(this, "Atenção!", "Item inválido!", QMessageBox::Ok);
    return;
  }
  if (ui->doubleSpinBoxQte->value() == 0) {
    QMessageBox::warning(this, "Atenção!", "Quantidade inválida!", QMessageBox::Ok, QMessageBox::NoButton);
    return;
  }
  //  int row = mapperItem.currentIndex();
  //  if (row == -1) {

  //  qDebug() << "rowCount: " << modelItem.rowCount();
  modelItem.insertRow(modelItem.rowCount());
  int row = modelItem.rowCount() - 1;

  //  mapperItem.toLast();
  //  int row = mapperItem.currentIndex();
  //  qDebug() << "adicionar row: " << row;

  //  modelItem.insertRow(row);
  //  mapperItem.toLast();
  //  }
  //  double qte = ui->doubleSpinBoxQte->value();
  //  double prcUn = ui->lineEditPrecoUn->getValue();
  //  double desc = ui->doubleSpinBoxDesconto->value();

  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("idOrcamento")), ui->lineEditOrcamento->text());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("idLoja")), UserSession::getLoja());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("idProduto")),
                    ui->itemBoxProduto->getValue().toInt());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("item")), row); // Item
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("fornecedor")), ui->lineEditFornecedor->text());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("produto")), ui->itemBoxProduto->text());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("obs")), ui->lineEditObs->text()); // Obs
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("prcUnitario")),
                    ui->lineEditPrecoUn->getValue()); // Pr. Un
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("caixas")), ui->doubleSpinBoxCaixas->value());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("qte")), ui->doubleSpinBoxQte->value()); // Qte
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("unCaixa")),
                    ui->doubleSpinBoxQte->singleStep());                                       // Un/Cx
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("un")), ui->lineEditUn->text()); // Un
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("desconto")),
                    ui->doubleSpinBoxDesconto->value()); // Desconto
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("descGlobal")),
                    ui->doubleSpinBoxDescontoGlobal->value()); // Desconto global

  calcPrecoGlobalTotal();

  novoItem();
  //  modelItem.insertRow(modelItem.rowCount());
}

void Orcamento::atualizarItem() {
  //  if (ui->comboBoxProduto->currentData().isNull()) {
  //    QMessageBox::warning(this, "Atenção!", "Item inválido!", QMessageBox::Ok, QMessageBox::NoButton);
  //    return;
  //  }
  if (ui->itemBoxProduto->text().isEmpty()) {
    QMessageBox::warning(this, "Atenção!", "Item inválido!", QMessageBox::Ok, QMessageBox::NoButton);
    return;
  }
  if (ui->doubleSpinBoxQte->value() == 0) {
    QMessageBox::warning(this, "Atenção!", "Quantidade inválida!", QMessageBox::Ok, QMessageBox::NoButton);
    return;
  }
  int row = mapperItem.currentIndex();
  //  qDebug() << "atualizar row: " << row;
  //  if (row == -1) {
  //    row = modelItem.rowCount();
  //    modelItem.insertRow(row);
  //  }
  //  double qte = ui->doubleSpinBoxQte->value();
  //  double prcUn = ui->lineEditPrecoUn->getValue();
  //  double desc = ui->doubleSpinBoxDesconto->value();

  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("idOrcamento")),
                    ui->lineEditOrcamento->text());                                                // Item
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("idLoja")), UserSession::getLoja()); // Item
  //  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("idProduto")),
  //                    ui->comboBoxProduto->currentData()); // idProduto
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("idProduto")), ui->itemBoxProduto->getValue());

  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("item")), row); // Item

  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("fornecedor")), ui->lineEditFornecedor->text());
  //  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("produto")),
  //                    ui->comboBoxProduto->currentText());                                         //
  //                    Produto
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("produto")), ui->itemBoxProduto->text());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("obs")), ui->lineEditObs->text()); // Obs
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("prcUnitario")),
                    ui->lineEditPrecoUn->getValue()); // Pr. Un
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("caixas")), ui->doubleSpinBoxCaixas->value());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("qte")), ui->doubleSpinBoxQte->value()); // Qte
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("unCaixa")),
                    ui->doubleSpinBoxQte->singleStep());                                       // Un/Cx
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("un")), ui->lineEditUn->text()); // Un
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("desconto")),
                    ui->doubleSpinBoxDesconto->value()); // Desconto
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("descGlobal")),
                    ui->doubleSpinBoxDescontoGlobal->value()); // Desconto global

  calcPrecoGlobalTotal();

  novoItem();
}

void Orcamento::on_pushButtonAdicionarItem_clicked() {
  adicionarItem();
}

void Orcamento::on_pushButtonAtualizarItem_clicked() {
  atualizarItem();
  ui->tableProdutos->clearSelection();
}

void Orcamento::on_pushButtonAtualizarOrcamento_clicked() {
  save();
}

void Orcamento::on_pushButtonFecharPedido_clicked() {
  if (!save()) {
    return;
  }

  QDateTime time = ui->dateTimeEdit->dateTime();
  if (!time.isValid()) {
    qDebug() << "Invalid time!";
    return;
  }
  if (time.addDays(data("validade").toInt()).date() > QDateTime::currentDateTime().date()) {
    //    qDebug() << "newer";
  } else {
    //    qDebug() << "older";
    QMessageBox::warning(this, "Aviso!", "Orçamento vencido!", QMessageBox::Ok);
    return;
  }

  Venda *venda = new Venda(parentWidget());
  venda->fecharOrcamento(ui->lineEditOrcamento->text());
  close();
}

void Orcamento::reject() {
  emit finished();
  QDialog::reject();
}

void Orcamento::on_checkBoxCalculaFrete_clicked() {
  calcPrecoGlobalTotal();
}

void Orcamento::on_doubleSpinBoxFrete_editingFinished() {
  ui->checkBoxCalculaFrete->setChecked(false);
  calcPrecoGlobalTotal();
}

void Orcamento::on_pushButtonCancelar_clicked() {
  cancel();
}

void Orcamento::on_pushButtonCancelarItem_clicked() {
  novoItem();
}

void Orcamento::on_doubleSpinBoxTotal_valueChanged(double) {
  calcPrecoGlobalTotal();
}

void Orcamento::on_doubleSpinBoxCaixas_valueChanged(double arg1) {
  double qte = arg1 * ui->doubleSpinBoxQte->singleStep();
  ui->doubleSpinBoxQte->setValue(qte);
  calcPrecoItemTotal();
}

void Orcamento::on_pushButtonApagarOrc_clicked() {
  ApagaOrcamento *apaga = new ApagaOrcamento(this);
  //  qDebug() << "mapper: " << mapperOrc.currentIndex();
  apaga->apagar(mapper.currentIndex());
}

void Orcamento::on_itemBoxProduto_textChanged(const QString &text) {
  qDebug() << "changed: " << text;

  if (ui->itemBoxProduto->text().isEmpty()) {
    ui->doubleSpinBoxCaixas->setDisabled(true);
    ui->doubleSpinBoxQte->setDisabled(true);
    ui->doubleSpinBoxDesconto->setDisabled(true);
    ui->doubleSpinBoxQte->setSingleStep(1.0);
    ui->doubleSpinBoxQte->setValue(0.0);
    ui->doubleSpinBoxCaixas->setValue(0.0);
    ui->lineEditFornecedor->clear();
    ui->lineEditPrecoTotal->clear();
    ui->lineEditPrecoUn->clear();
    ui->lineEditPrecoUn->setDisabled(true);
    ui->lineEditPrecoTotal->setDisabled(true);
    ui->lineEditUn->clear();
    return;
  }

  QSqlQuery query;
  query.prepare("SELECT * FROM Produto WHERE idProduto = :idx");
  query.bindValue(":idx", ui->itemBoxProduto->getValue().toInt());
  qDebug() << "value: " << ui->itemBoxProduto->getValue().toInt();
  if (!query.exec()) {
    qDebug() << "Erro na busca do produto: " << query.lastError();
  }
  query.first();
  QString un = query.value("un").toString();
  ui->lineEditUn->setText(un);
  ui->lineEditPrecoUn->setValue(query.value("precoVenda").toDouble());
  ui->lineEditEstoque->setValue(query.value("estoque").toInt());
  ui->lineEditFornecedor->setText(query.value("fornecedor").toString());

  ui->doubleSpinBoxCaixas->setEnabled(true);
  ui->doubleSpinBoxQte->setEnabled(true);
  ui->doubleSpinBoxCaixas->setEnabled(true);
  ui->doubleSpinBoxDesconto->setEnabled(true);
  ui->lineEditPrecoUn->setEnabled(true);
  ui->lineEditPrecoTotal->setEnabled(true);
  if (un.contains("m2")) {
    ui->doubleSpinBoxQte->setSingleStep(query.value("m2cx").toDouble());
  } else if (un.contains("pç") or un.contains("pc")) {
    ui->doubleSpinBoxQte->setSingleStep(query.value("pccx").toDouble());
  }
  ui->doubleSpinBoxQte->setValue(0);

  calcPrecoItemTotal();
}

void Orcamento::on_itemBoxCliente_textChanged(const QString &text) {
  if (!text.isEmpty()) {
    QSqlQuery queryEndereco;
    queryEndereco.prepare("SELECT * FROM Endereco WHERE idEndereco IN (SELECT idEndereco FROM Cadastro_has_Endereco WHERE idCadastro = :idCadastro)");
    queryEndereco.bindValue(":idCadastro",ui->itemBoxCliente->getValue());
    if (!queryEndereco.exec()) {
      qDebug() << "Erro ao buscar endereços " << queryEndereco.lastError();
    }

    ui->comboBoxEndereco->clear();
    ui->comboBoxEndereco->addItem("Escolha uma opção!");
    ui->comboBoxEndereco->addItem("Não há", 1);
    while (queryEndereco.next()) {
      QString descricao = queryEndereco.value("descricao").toString();
      QString cep = queryEndereco.value("CEP").toString();
      QString logradouro = queryEndereco.value("logradouro").toString();
      QString numero = queryEndereco.value("numero").toString();
      QString cidade = queryEndereco.value("cidade").toString();

      QStringList list;
      list << descricao << cep << logradouro << numero << cidade;
      QString str;
      foreach (QString txt, list) {
        if(!txt.isEmpty()) {
          if(str.isEmpty()) {
            str += txt;
          } else {
            str += " - " + txt;
          }
        }
      }

      ui->comboBoxEndereco->addItem(str, queryEndereco.value(0));
    }

    QSqlQuery queryCliente;
    queryCliente.prepare("SELECT idProfissionalRel FROM Cadastro WHERE idCadastro = :idCadastro");
    queryCliente.bindValue(":idCadastro",ui->itemBoxCliente->getValue());
    if (!queryCliente.exec() || !queryCliente.first()) {
      qDebug() << "Erro ao buscar cliente: " << queryCliente.lastError();
    }
    ui->comboBoxEndereco->setCurrentValue(queryCliente.value(0));

    ui->comboBoxProfissional->setCurrentValue(queryCliente.value("idProfissionalRel"));
  }
}

void Orcamento::successMessage() {
  QMessageBox::information(this, "Atenção!", "Orçamento atualizado com sucesso!", QMessageBox::Ok,
                           QMessageBox::NoButton);
}
