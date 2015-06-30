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
#include <QSqlDriver>
#include <QSqlRecord>

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

  setupTables();

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

  foreach (const QLineEdit *line, findChildren<QLineEdit *>()) {
    connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);
  }

  if (UserSession::getTipoUsuario() == "ADMINISTRADOR") {
    ui->dateTimeEdit->setReadOnly(false);
    ui->dateTimeEdit->setCalendarPopup(true);
    ui->checkBoxFreteManual->show();
  } else {
    ui->checkBoxFreteManual->hide();
    // NOTE: remove this later
    ui->dateTimeEdit->setReadOnly(false);
    ui->dateTimeEdit->setCalendarPopup(true);
  }

  ui->lineEditCodComercial->hide();
  ui->lineEditFormComercial->hide();
  ui->labelCodComercial->hide();
  ui->labelFormComercial->hide();
}

Orcamento::~Orcamento() { delete ui; }

void Orcamento::show() {
  QDialog::show();
  adjustSize();
  ui->tableProdutos->resizeColumnsToContents();
}

void Orcamento::on_tableProdutos_clicked(const QModelIndex &index) {
  ui->pushButtonAtualizarItem->show();
  ui->pushButtonAdicionarItem->hide();
  mapperItem.setCurrentModelIndex(index);
}

bool Orcamento::viewRegister(const QModelIndex index) {
  if (not RegisterDialog::viewRegister(index)) {
    return false;
  }

  const QString idOrcamento = data(primaryKey).toString();
  modelItem.setFilter("idOrcamento = '" + idOrcamento + "'");

  if (not modelItem.select()) {
    qDebug() << "erro modelItem: " << modelItem.lastError();
    return false;
  }

  novoItem();

  calcPrecoGlobalTotal();

  const QDateTime time = ui->dateTimeEdit->dateTime();

  if (time.addDays(data("validade").toInt()).date() < QDateTime::currentDateTime().date()) {
    ui->pushButtonGerarVenda->hide();
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
  mapperItem.addMapping(ui->doubleSpinBoxPrecoTotal, model.fieldIndex("total"));
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
  ui->pushButtonReplicar->hide();

  ui->pushButtonImprimir->setEnabled(false);
  ui->pushButtonGerarVenda->setEnabled(true);
  ui->itemBoxEndereco->setEnabled(false);
}

void Orcamento::updateMode() {
  ui->pushButtonCadastrarOrcamento->hide();
  ui->pushButtonAtualizarOrcamento->show();
  ui->pushButtonReplicar->show();

  ui->pushButtonImprimir->setEnabled(true);
  ui->pushButtonGerarVenda->setEnabled(true);
  ui->itemBoxEndereco->setVisible(true);
  ui->spinBoxValidade->setEnabled(false);
}

bool Orcamento::newRegister() {
  if (not RegisterDialog::newRegister()) {
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
  QSqlQuery query;
  query.prepare("SELECT * FROM Orcamento WHERE idOrcamento = :idOrcamento");
  query.bindValue(":idOrcamento", ui->lineEditOrcamento->text());

  if (not query.exec()) {
    qDebug() << "Erro na query: " << query.lastError();
  }

  if (query.size() != 0) {
    return;
  }

  QString id = UserSession::getSiglaLoja() + "-" + QDate::currentDate().toString("yy");

  query.prepare("SELECT idOrcamento FROM Orcamento WHERE idOrcamento LIKE :id UNION SELECT idVenda AS idOrcamento FROM "
                "Venda WHERE idVenda LIKE :id ORDER BY idOrcamento ASC");
  query.bindValue(":id", id + "%");

  if (not query.exec()) {
    qDebug() << "Erro na query: " << query.lastError();
  }

  int last = 0;

  if (query.size() > 0) {
    query.last();
    last = query.value("idOrcamento").toString().mid(id.size()).toInt();
  }

  id += QString("%1").arg(last + 1, 4, 10, QChar('0'));
  ui->lineEditOrcamento->setText(id);

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex(primaryKey)), id);
  }
}

bool Orcamento::verifyFields(const int row) {
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

bool Orcamento::savingProcedures(const int row) {
  updateId();

  const QString idOrcamento = ui->lineEditOrcamento->text();

  if (model.data(model.index(row, model.fieldIndex("idOrcamento"))).toString() != idOrcamento) {
    setData(row, "idOrcamento", idOrcamento);
  }

  setData(row, "idLoja", UserSession::getLoja());
  setData(row, "idCliente", ui->itemBoxCliente->value());
  setData(row, "idEnderecoEntrega", ui->itemBoxEndereco->value());
  setData(row, "idEnderecoFaturamento", ui->itemBoxEndereco->value());
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

  if (not model.submitAll()) {
    QMessageBox::warning(this, "Atenção!", "Não foi possível cadastrar este orçamento.", QMessageBox::Ok,
                         QMessageBox::NoButton);
    qDebug() << "SUBMITALL ERROR: " << model.lastError();
    qDebug() << "QUERY: " << model.query().lastQuery();
    return false;
  }

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    modelItem.setData(model.index(row, modelItem.fieldIndex(primaryKey)), idOrcamento);
    modelItem.setData(model.index(row, modelItem.fieldIndex("idLoja")), UserSession::getLoja());
  }

  if (not modelItem.submitAll()) {
    qDebug() << "Failed to add item! : " << modelItem.lastError().text();
    qDebug() << "QUERY: " << modelItem.query().lastQuery();
    qDebug() << "Last query: "
             << modelItem.database().driver()->sqlStatement(QSqlDriver::InsertStatement, modelItem.tableName(),
                                                            modelItem.record(row), false);
    QMessageBox::warning(this, "Atenção!", "Erro ao adicionar um item ao orçamento.", QMessageBox::Ok,
                         QMessageBox::NoButton);
    return false;
  }

  novoItem();
  viewRegisterById(idOrcamento);

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

  const double qte = ui->doubleSpinBoxQte->value();
  const double prcUn = ui->lineEditPrecoUn->getValue();
  const double desc = ui->doubleSpinBoxDesconto->value() / 100.0;
  const double itemBruto = qte * prcUn;
  const double subTotalItem = itemBruto * (1.0 - desc);

  if (not ui->itemBoxProduto->text().isEmpty()) {
    ui->doubleSpinBoxPrecoTotal->setValue(subTotalItem);
  }
}

void Orcamento::on_doubleSpinBoxDesconto_valueChanged(const double) { calcPrecoItemTotal(); }

void Orcamento::on_doubleSpinBoxQte_valueChanged(const double) {
  const double caixas = ui->doubleSpinBoxQte->value() / ui->doubleSpinBoxQte->singleStep();
  ui->spinBoxCaixas->setValue(caixas);
}

void Orcamento::on_doubleSpinBoxQte_editingFinished() {
  const double step = ui->doubleSpinBoxQte->singleStep();
  const double mult = ui->doubleSpinBoxQte->value() / step;
  ui->doubleSpinBoxQte->setValue(qCeil(mult) * step);
}

void Orcamento::on_pushButtonCadastrarOrcamento_clicked() { save(); }

void Orcamento::on_pushButtonAtualizarOrcamento_clicked() { update(); }

void Orcamento::calcPrecoGlobalTotal(const bool ajusteTotal) {
  subTotal = 0.0;
  subTotalItens = 0.0;
  double subTotalBruto = 0.0;
  double minimoFrete = 0, porcFrete = 0;

  QSqlQuery queryFrete;
  queryFrete.prepare("SELECT * FROM Loja WHERE idLoja = :idLoja");
  queryFrete.bindValue(":idLoja", UserSession::getLoja());

  if (not queryFrete.exec()) {
    qDebug() << "Erro buscando parâmetros do frete: " << queryFrete.lastError();
  }

  if (queryFrete.next()) {
    minimoFrete = queryFrete.value("valorMinimoFrete").toDouble();
    porcFrete = queryFrete.value("porcentagemFrete").toDouble();
  }

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    const double prcUnItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("prcUnitario"))).toDouble();
    const double qteItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("qte"))).toDouble();
    const double descItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("desconto"))).toDouble() / 100.0;
    const double itemBruto = qteItem * prcUnItem;
    subTotalBruto += itemBruto;
    const double stItem = itemBruto * (1.0 - descItem);
    subTotalItens += stItem;
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("parcial")), itemBruto);
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("parcialDesc")), stItem);
  }

  double frete = qMax(subTotalBruto * porcFrete / 100.0, minimoFrete);

  if (ui->checkBoxFreteManual->isChecked()) {
    frete = ui->doubleSpinBoxFrete->value();
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

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    const double stItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("parcialDesc"))).toDouble();
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("descGlobal")), descGlobal * 100.0);

    const double totalItem = stItem * (1 - descGlobal);
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("total")), totalItem);
  }

  ui->doubleSpinBoxSubTotalBruto->setValue(subTotalBruto);
  ui->doubleSpinBoxDescontoGlobal->setValue(descGlobal * 100);
  ui->doubleSpinBoxDescontoRS->setValue(subTotalItens - subTotal);
  ui->doubleSpinBoxFrete->setValue(frete);
  ui->doubleSpinBoxTotal->setValue(subTotalItens);
  ui->doubleSpinBoxFinal->setValue(subTotal + frete);
}

void Orcamento::on_doubleSpinBoxDescontoGlobal_valueChanged(const double) {
  calcPrecoItemTotal();
  calcPrecoGlobalTotal();
}

void Orcamento::on_doubleSpinBoxFinal_editingFinished() {
  if (modelItem.rowCount() == 0 or subTotalItens == 0) {
    calcPrecoGlobalTotal();
    return;
  }

  const double new_total = ui->doubleSpinBoxFinal->value();
  const double frete = ui->doubleSpinBoxFrete->value();
  const double new_subtotal = new_total - frete;

  if (new_subtotal >= subTotalItens) {
    ui->doubleSpinBoxDescontoGlobal->setValue(0.0);
    calcPrecoGlobalTotal();
  } else {
    calcPrecoGlobalTotal(true);
  }
}

void Orcamento::on_pushButtonImprimir_clicked() {
  QMessageBox::warning(this, "Aviso!", "Ainda não implementado");
  //  QPrinter printer;
  //  //  printer.setFullPage(true);
  //  //  printer.setResolution(300);
  //  printer.setPageMargins(QMargins(300, 600, 300, 200), QPageLayout::Millimeter);
  //  printer.setOrientation(QPrinter::Portrait);
  //  printer.setPaperSize(QPrinter::A4);
  //  QPrintPreviewDialog preview(&printer, this, Qt::Window);
  //  preview.setModal(true);
  //  preview.setWindowTitle("Impressão de Orçamento");
  //  connect(&preview, &QPrintPreviewDialog::paintRequested, this, &Orcamento::print);
  //  preview.showMaximized();
  //  preview.exec();
}

QString Orcamento::itemData(const int row, const QString key) {
  return modelItem.data(modelItem.index(row, modelItem.fieldIndex(key))).toString();
}

void Orcamento::setupTables() {
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

  if (not modelItem.select()) {
    qDebug() << "erro modelItem: " << modelItem.lastError();
    return;
  }

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

  ui->tableProdutos->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

  ui->tableProdutos->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableProdutos->horizontalHeader()->setResizeContentsPrecision(0);
}

void Orcamento::print(const QPrinter *printer) { Q_UNUSED(printer); }

void Orcamento::adicionarItem(const bool isUpdate) {
  calcPrecoItemTotal();

  if (ui->itemBoxProduto->text().isEmpty()) {
    QMessageBox::warning(this, "Atenção!", "Item inválido!", QMessageBox::Ok);
    return;
  }

  if (ui->doubleSpinBoxQte->value() == 0) {
    QMessageBox::warning(this, "Atenção!", "Quantidade inválida!", QMessageBox::Ok, QMessageBox::NoButton);
    return;
  }

  const int row = (isUpdate) ? mapperItem.currentIndex() : modelItem.rowCount();

  if (not isUpdate) {
    modelItem.insertRow(row);
  }

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

void Orcamento::on_pushButtonAdicionarItem_clicked() { adicionarItem(); }

void Orcamento::on_pushButtonAtualizarItem_clicked() {
  adicionarItem(true);
  ui->tableProdutos->clearSelection();
}

void Orcamento::on_pushButtonGerarVenda_clicked() {
  silent = true;

  if (ui->lineEditOrcamento->text().isEmpty()) {
    if (not save()) {
      return;
    }
  } else {
    if (not update()) {
      return;
    }
  }

  const QDateTime time = ui->dateTimeEdit->dateTime();

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

  if (model.data(model.index(mapper.currentIndex(), model.fieldIndex("status"))).toString() == "CANCELADO") {
    QMessageBox::warning(this, "Aviso!", "Orçamento cancelado");
    // TODO: ask user if he wants to replicate orcamento
    return;
  }

  const int idCliente = ui->itemBoxCliente->value().toInt();

  QSqlQuery queryCadastro;
  queryCadastro.prepare("SELECT incompleto FROM Orcamento LEFT JOIN Cliente ON Orcamento.idCliente = Cliente.idCliente "
                        "WHERE Cliente.idCliente = :idCliente AND incompleto = TRUE");
  queryCadastro.bindValue(":idCliente", idCliente);

  if (not queryCadastro.exec()) {
    qDebug() << "Erro verificando se cadastro do cliente está completo: " << queryCadastro.lastError();
    return;
  }

  if (queryCadastro.next()) {
    QMessageBox::warning(this, "Aviso!", "Cadastro incompleto, deve terminar.");
    RegisterDialog *cadCliente = new CadastroCliente(this);
    cadCliente->viewRegisterById(idCliente);
    cadCliente->show();
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

void Orcamento::on_doubleSpinBoxTotal_valueChanged(const double) { calcPrecoGlobalTotal(); }

void Orcamento::on_spinBoxCaixas_valueChanged(const int caixas) {
  const double qte = caixas * ui->doubleSpinBoxQte->singleStep();
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
    ui->doubleSpinBoxPrecoTotal->clear();
    ui->lineEditPrecoUn->clear();
    ui->lineEditPrecoUn->setDisabled(true);
    ui->doubleSpinBoxPrecoTotal->setDisabled(true);
    ui->lineEditUn->clear();
    ui->lineEditObs->clear();
    return;
  }

  QSqlQuery query;
  query.prepare("SELECT * FROM Produto WHERE idProduto = :index");
  query.bindValue(":index", ui->itemBoxProduto->value().toInt());

  if (not query.exec()) {
    qDebug() << "Erro na busca do produto: " << query.lastError();
  }

  query.first();
  const QString un = query.value("un").toString();
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
  ui->doubleSpinBoxPrecoTotal->setEnabled(true);

  if (un.contains("M2") or un.contains("M²") or un.contains("ML")) {
    ui->doubleSpinBoxQte->setSingleStep(query.value("m2cx").toDouble());
  } else {
    ui->doubleSpinBoxQte->setSingleStep(query.value("pccx").toDouble());
  }

  ui->doubleSpinBoxQte->setValue(0);

  calcPrecoItemTotal();
}

void Orcamento::on_itemBoxCliente_textChanged(const QString &text) {
  Q_UNUSED(text);

  ui->itemBoxEndereco->searchDialog()->setFilter("idCliente = " + QString::number(ui->itemBoxCliente->value().toInt()) +
                                                 " AND desativado = FALSE OR idEndereco = 1");

  QSqlQuery queryCliente;
  queryCliente.prepare("SELECT idProfissionalRel FROM Cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", ui->itemBoxCliente->value());

  if (not queryCliente.exec() or not queryCliente.first()) {
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

void Orcamento::on_pushButtonLimparSelecao_clicked() { novoItem(); }

void Orcamento::on_checkBoxFreteManual_clicked(const bool checked) {
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
  Orcamento *orcamento = new Orcamento(parentWidget());
  orcamento->ui->pushButtonReplicar->hide();

  int row = mapper.currentIndex();

  // MODEL
  orcamento->ui->itemBoxCliente->setValue(model.data(model.index(row, model.fieldIndex("idCliente"))));
  orcamento->ui->itemBoxProfissional->setValue(model.data(model.index(row, model.fieldIndex("idProfissional"))));
  orcamento->ui->itemBoxVendedor->setValue(model.data(model.index(row, model.fieldIndex("idUsuario"))));
  orcamento->ui->itemBoxEndereco->setValue(model.data(model.index(row, model.fieldIndex("idEnderecoEntrega"))));
  orcamento->ui->spinBoxValidade->setValue(model.data(model.index(row, model.fieldIndex("validade"))).toInt());
  orcamento->ui->doubleSpinBoxDescontoGlobal->setValue(
      model.data(model.index(row, model.fieldIndex("descontoPorc"))).toDouble());
  orcamento->ui->doubleSpinBoxFrete->setValue(model.data(model.index(row, model.fieldIndex("frete"))).toDouble());
  orcamento->ui->doubleSpinBoxFinal->setValue(model.data(model.index(row, model.fieldIndex("total"))).toDouble());
  orcamento->ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

  // MODELITEM
  for (int i = 0; i < modelItem.rowCount(); ++i) {
    orcamento->ui->itemBoxProduto->setValue(modelItem.data(modelItem.index(i, modelItem.fieldIndex("idProduto"))));
    orcamento->ui->doubleSpinBoxQte->setValue(
        modelItem.data(modelItem.index(i, modelItem.fieldIndex("qte"))).toDouble());
    orcamento->adicionarItem();
  }

  orcamento->show();
}

void Orcamento::on_doubleSpinBoxPrecoTotal_editingFinished() {
  if (ui->itemBoxProduto->text().isEmpty()) {
    return;
  }

  double qte = ui->doubleSpinBoxQte->value();
  double prcUn = ui->lineEditPrecoUn->getValue();
  double itemBruto = qte * prcUn;
  double subTotalItem = ui->doubleSpinBoxPrecoTotal->value();
  double desconto = (itemBruto - subTotalItem) / itemBruto * 100;

  ui->doubleSpinBoxDesconto->setValue(desconto);
}
