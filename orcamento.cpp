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

Orcamento::Orcamento(QWidget *parent)
  : RegisterDialog("Orcamento", "idOrcamento", parent), ui(new Ui::Orcamento) {
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
  modelItem.setHeaderData(modelItem.fieldIndex("parcial"), Qt::Horizontal, "Subtotal");
  modelItem.setHeaderData(modelItem.fieldIndex("desconto"), Qt::Horizontal, "Desc. %");
  modelItem.setHeaderData(modelItem.fieldIndex("parcialDesc"), Qt::Horizontal, "Total");
  //  modelItem.setHeaderData(modelItem.fieldIndex("total"), Qt::Horizontal, "Preço");

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

  ui->tableProdutos->horizontalHeader()->setStretchLastSection(false);

  SearchDialog * sdCliente = SearchDialog::cliente(ui->itemBoxCliente);
  ui->itemBoxCliente->setSearchDialog(sdCliente);

  SearchDialog * sdProd = SearchDialog::produto(ui->itemBoxProduto);
  ui->itemBoxProduto->setSearchDialog(sdProd);

  RegisterDialog * cadCliente = new CadastroCliente(this);
  ui->itemBoxCliente->setRegisterDialog(cadCliente);

  SearchDialog * sdVendedor = SearchDialog::vendedor(ui->itemBoxVendedor);
  ui->itemBoxVendedor->setSearchDialog(sdVendedor);

  SearchDialog * sdProfissional = SearchDialog::profissional(ui->itemBoxProfissional);
  ui->itemBoxProfissional->setSearchDialog(sdProfissional);

  SearchDialog * sdEndereco = SearchDialog::endereco(ui->itemBoxEndereco);
  ui->itemBoxEndereco->setSearchDialog(sdEndereco);

  setupMapper();
  newRegister();
  //  show();
  showMaximized();

  if(UserSession::getTipo() == "ADMINISTRADOR") {
    ui->dateTimeEdit->setReadOnly(false);
    ui->dateTimeEdit->setCalendarPopup(true);
    ui->checkBoxFreteManual->show();
  }else{
    ui->checkBoxFreteManual->hide();
  }
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

void Orcamento::novoItem() {
  ui->pushButtonAdicionarItem->show();
  ui->pushButtonAtualizarItem->hide();
  ui->itemBoxProduto->clear();
  ui->itemBoxProduto->setValue(QVariant());
}

void Orcamento::setupMapper() {
  mapper.setModel(&model);

  addMapping(ui->lineEditOrcamento, "idOrcamento");
  addMapping(ui->itemBoxCliente, "idCliente", "value");
  addMapping(ui->itemBoxProfissional, "idProfissional", "value");
  addMapping(ui->itemBoxVendedor, "idUsuario", "value");
  addMapping(ui->itemBoxEndereco, "idEnderecoEntrega", "value");
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
  ui->itemBoxEndereco->setEnabled(false);
}

void Orcamento::updateMode() {
  ui->pushButtonCadastrarOrcamento->hide();
  ui->pushButtonAtualizarOrcamento->show();

  ui->pushButtonImprimir->setEnabled(true);
  ui->pushButtonFecharPedido->setEnabled(true);
  ui->itemBoxEndereco->setVisible(true);
  ui->spinBoxValidade->setEnabled(false);
}

bool Orcamento::newRegister() {
  if (!RegisterDialog::newRegister()) {
    return false;
  }
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
  QSqlQuery queryIdExists("SELECT * FROM Orcamento WHERE idOrcamento = '" + ui->lineEditOrcamento->text() +
                          "'");
  queryIdExists.exec();
  if (queryIdExists.size() != 0) {
    return;
  }
  QString id = UserSession::getSiglaLoja() + "-" + QDate::currentDate().toString("yy");
  QSqlQuery query("SELECT idOrcamento FROM Orcamento WHERE idOrcamento LIKE '" + id +
                  "%' UNION SELECT idVenda AS idOrcamento FROM Venda WHERE idVenda LIKE '" + id + "%' ORDER BY idOrcamento ASC");
  query.last();
  int last = query.value("idOrcamento").toString().mid(id.size()).toInt();
  id += QString("%1")
        .arg(last + 1, 4, 10, QChar('0')); // QString("%1").arg(last + 1, 3, 16, QChar('0')).toUpper();
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
  setData(row, "idCliente", ui->itemBoxCliente->getValue());
  setData(row, "idEnderecoEntrega", ui->itemBoxEndereco->getValue());
  setData(row, "idUsuario", ui->itemBoxVendedor->getValue());
  setData(row, "idProfissional", ui->itemBoxProfissional->getValue());
  setData(row, "validade", ui->spinBoxValidade->value());
  setData(row, "data", ui->dateTimeEdit->dateTime());
  setData(row, "total", ui->doubleSpinBoxFinal->value());
  qDebug() << "desconto global: " << ui->doubleSpinBoxDescontoGlobal->value();
  setData(row, "desconto", ui->doubleSpinBoxDescontoGlobal->value());
  setData(row, "frete", ui->doubleSpinBoxFrete->value());

  if (!model.submitAll() && model.lastError().number() != 1062) {
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
  ui->itemBoxVendedor->setValue(UserSession::getId());
  ui->itemBoxEndereco->setEnabled(false);
}

void Orcamento::on_pushButtonRemoverItem_clicked() {
  removeItem();
}

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

void Orcamento::on_doubleSpinBoxDesconto_valueChanged(double) {
  calcPrecoItemTotal();
}

void Orcamento::on_doubleSpinBoxQte_valueChanged(double) {
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

void Orcamento::calcPrecoGlobalTotal(bool ajusteTotal) {
  subTotal = 0.0;
  subTotalItens = 0.0;
  double subTotalBruto = 0.0;
  double minimoFrete = 0, porcFrete = 0;

  QSqlQuery qryFrete;
  if (!qryFrete.exec("SELECT * FROM Loja WHERE idLoja = '" + QString::number(UserSession::getLoja()) + "'")) {
    qDebug() << "Erro buscando parâmetros do frete: " << qryFrete.lastError();
  }
  if (qryFrete.next()) {
    minimoFrete = qryFrete.value("valorMinimoFrete").toDouble();
    porcFrete = qryFrete.value("porcentagemFrete").toDouble();
  }

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    double prcUnItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("prcUnitario"))).toDouble();
    double qteItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("qte"))).toDouble();
    double descItem =
      modelItem.data(modelItem.index(row, modelItem.fieldIndex("desconto"))).toDouble() / 100.0;
    double itemBruto = qteItem * prcUnItem;
    subTotalBruto += itemBruto;
    double stItem = itemBruto * (1.0 - descItem);
    subTotalItens += stItem;
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("parcial")), itemBruto);  // Pr. Parcial
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("parcialDesc")), stItem); // Pr. Parcial Desc.
  }
  double frete = qMax(subTotalBruto * porcFrete / 100.0, minimoFrete);
  if(ui->checkBoxFreteManual->isChecked()) {
    frete = ui->doubleSpinBoxFrete->value();
  }
  double descGlobal = ui->doubleSpinBoxDescontoGlobal->value() / 100.0;
  subTotal = subTotalItens * (1.0 - descGlobal);
  if (ajusteTotal) {
    const double Final = ui->doubleSpinBoxFinal->value();
    subTotal = Final - frete;
    if(subTotalItens == 0.0) {
      descGlobal = 0;
    } else {
      descGlobal = 1 - (subTotal / subTotalItens);
    }
  }

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    double stItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("parcialDesc"))).toDouble();
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("descGlobal")),
                      descGlobal * 100.0); // Desconto
    // Distr.
    double totalItem = stItem * (1 - descGlobal);
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("total")), totalItem); // Pr. Final
  }

  ui->doubleSpinBoxDescontoGlobal->setValue(descGlobal * 100);
  ui->doubleSpinBoxFrete->setValue(frete);
  ui->doubleSpinBoxTotal->setValue(subTotalItens);
//  ui->doubleSpinBoxTotalFrete->setValue(subTotalItens + frete);
  ui->doubleSpinBoxDescontoRS->setValue(subTotalItens - subTotal);
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

  //  qDebug() << "New total = " << new_total << ", frete = " << frete << ", new sub. = " << new_subtotal
  //           << ", subTotalItens = " << subTotalItens;
  if (new_subtotal >= subTotalItens) {
    ui->doubleSpinBoxDescontoGlobal->setValue(0.0);
    calcPrecoGlobalTotal();
  } else {
    calcPrecoGlobalTotal(true);
  }
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
  QDir dir(QApplication::applicationDirPath());
  QFile file(dir.absoluteFilePath("orcamento.html"));
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
  html.replace("LOGO", QUrl::fromLocalFile(dir.absoluteFilePath("logo.png")).toString());
  //  html.replace("LOGO", QDir::home().absoluteFilePath("logo.png"));

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
  html.replace("VENDEDOR", ui->itemBoxVendedor->text());
  html.replace("CLIENTE", ui->itemBoxCliente->text());
  html.replace("DATA", ui->dateTimeEdit->text());

  QString itens = getItensHtml();
  html.replace("ITENS", itens);

//  html.replace("SUBTOTAL", ui->doubleSpinBoxTotalFrete->text());
  html.replace("DESCONTO", ui->doubleSpinBoxDescontoGlobal->text());
  html.replace("FRETE", ui->doubleSpinBoxFrete->text());
  html.replace("TOTAL", ui->doubleSpinBoxFinal->text());

  frame->setHtml(html);
  //  qDebug() << html;
  //  frame->setTextSizeMultiplier(1.2);
  frame->print(printer);
  QFile outputFile(dir.absoluteFilePath("orc.html"));
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

  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("idOrcamento")),
                    ui->lineEditOrcamento->text());                                                // Item
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("idLoja")), UserSession::getLoja()); // Item
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("idProduto")), ui->itemBoxProduto->getValue());

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
  if (!save(true)) {
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

  int idCliente = ui->itemBoxCliente->getValue().toInt();
  QSqlQuery qryCadastro;
  if (!qryCadastro.exec("SELECT incompleto FROM Orcamento LEFT JOIN Cliente ON Orcamento.idCliente "
                        "= Cliente.idCliente WHERE Cliente.idCliente = " +
                        QString::number(idCliente) + " AND incompleto = 1")) {
    qDebug() << "Erro verificando se cadastro do cliente está completo: " << qryCadastro.lastError();
    return;
  }
  if (qryCadastro.next()) {
//    qDebug() << "terminar cadastro do cliente";
    QMessageBox::warning(this, "Aviso!", "Cadastro incompleto, deve terminar.");
    RegisterDialog *cadCliente = new CadastroCliente(this);
    cadCliente->viewRegisterById(idCliente);
//    sdEndereco = SearchDialog::endereco(ui->itemBoxEndereco);
//    ui->itemBoxEndereco->setSearchDialog(sdEndereco);
    return;
  }
  if (ui->itemBoxEndereco->text().isEmpty()) {
//    qDebug() << "deve ter endereço";
    QMessageBox::warning(this, "Aviso!", "Deve escolher um endereço.");
    return;
  }

  Venda *venda = new Venda(parentWidget());
  venda->fecharOrcamento(ui->lineEditOrcamento->text());
  close();
}

void Orcamento::on_doubleSpinBoxFrete_editingFinished() {
  calcPrecoGlobalTotal();
}

void Orcamento::on_pushButtonCancelar_clicked() {
  close();
}

void Orcamento::on_pushButtonCancelarItem_clicked() {
  novoItem();
}

void Orcamento::on_doubleSpinBoxTotal_valueChanged(double) {
  calcPrecoGlobalTotal();
}

void Orcamento::on_doubleSpinBoxCaixas_valueChanged(double caixas) {
  double qte = caixas * ui->doubleSpinBoxQte->singleStep();
  ui->doubleSpinBoxQte->setValue(qte);
  calcPrecoItemTotal();
}

void Orcamento::on_pushButtonApagarOrc_clicked() {
  ApagaOrcamento *apaga = new ApagaOrcamento(this);
  //  qDebug() << "mapper: " << mapperOrc.currentIndex();
  apaga->apagar(mapper.currentIndex());
}

void Orcamento::on_itemBoxProduto_textChanged(const QString &text) {
  Q_UNUSED(text);
  //  qDebug() << "changed: " << text;
  ui->doubleSpinBoxQte->setValue(0.0);
  ui->doubleSpinBoxCaixas->setValue(0.0);

  if (ui->itemBoxProduto->text().isEmpty()) {
    ui->doubleSpinBoxCaixas->setDisabled(true);
    ui->doubleSpinBoxQte->setDisabled(true);
    ui->doubleSpinBoxDesconto->setDisabled(true);
    ui->doubleSpinBoxQte->setSingleStep(1.0);
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
  //  qDebug() << "value: " << ui->itemBoxProduto->getValue().toInt();
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
  Q_UNUSED(text);
//  qDebug() << "id: " << ui->itemBoxCliente->getValue().toInt();
  ui->itemBoxEndereco->getSearchDialog()->setFilter("idCliente = " + QString::number(ui->itemBoxCliente->getValue().toInt()) +
      " AND ativo = 1");
  QSqlQuery queryCliente;
  queryCliente.prepare("SELECT idProfissionalRel FROM Cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", ui->itemBoxCliente->getValue());
  if (!queryCliente.exec() || !queryCliente.first()) {
    qDebug() << "Erro ao buscar cliente: " << queryCliente.lastError();
  }
  ui->itemBoxProfissional->setValue(queryCliente.value("idProfissionalRel"));
  ui->itemBoxEndereco->setEnabled(true);
  ui->itemBoxEndereco->clear();
}

void Orcamento::successMessage() {
  qDebug() << "teste";
  QMessageBox::information(this, "Atenção!", "Orçamento atualizado com sucesso!", QMessageBox::Ok,
                           QMessageBox::NoButton);
}

void Orcamento::on_pushButtonLimparSelecao_clicked() {
  ui->tableProdutos->clearSelection();
  novoItem();
}

void Orcamento::on_checkBoxFreteManual_clicked(bool checked) {
  if(checked == true && UserSession::getTipo() != "ADMINISTRADOR") {
    ui->checkBoxFreteManual->setChecked(false);
    return;
  }
  ui->doubleSpinBoxFrete->setFrame(checked);
  ui->doubleSpinBoxFinal->setReadOnly(!checked);
}
