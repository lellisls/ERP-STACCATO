#include <QSqlDriver>
#include <QSqlRecord>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <xlsxdocument.h>

#include "orcamento.h"
#include "ui_orcamento.h"
#include "qtrpt.h"
#include "cadastrocliente.h"
#include "usersession.h"
#include "venda.h"
#include "apagaorcamento.h"
#include "doubledelegate.h"

Orcamento::Orcamento(QWidget *parent) : RegisterDialog("orcamento", "idOrcamento", parent), ui(new Ui::Orcamento) {
  ui->setupUi(this);

  setupTables();

  ui->itemBoxCliente->setSearchDialog(SearchDialog::cliente(this));
  ui->itemBoxCliente->setRegisterDialog(new CadastroCliente(this));
  ui->itemBoxProduto->setSearchDialog(SearchDialog::produto(this));
  ui->itemBoxVendedor->setSearchDialog(SearchDialog::vendedor(this));
  ui->itemBoxProfissional->setSearchDialog(SearchDialog::profissional(this));
  ui->itemBoxEndereco->setSearchDialog(SearchDialog::enderecoCliente(this));

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
    ui->pushButtonAtualizarOrcamento->hide();
    ui->pushButtonReplicar->show();
  } else {
    ui->pushButtonGerarVenda->show();
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
  addMapping(ui->lineEditOrcamento, "idOrcamento");
  addMapping(ui->itemBoxCliente, "idCliente", "value");
  addMapping(ui->itemBoxProfissional, "idProfissional", "value");
  addMapping(ui->itemBoxVendedor, "idUsuario", "value");
  addMapping(ui->itemBoxEndereco, "idEnderecoEntrega", "value");
  addMapping(ui->spinBoxValidade, "validade");
  addMapping(ui->doubleSpinBoxDescontoGlobal, "descontoPorc");
  addMapping(ui->doubleSpinBoxFrete, "frete");
  addMapping(ui->doubleSpinBoxTotal, "total");
  addMapping(ui->dateTimeEdit, "data");
  addMapping(ui->spinBoxPrazoEntrega, "prazoEntrega");
  addMapping(ui->textEditObs, "observacao");

  mapperItem.setModel(&modelItem);
  mapperItem.setSubmitPolicy(QDataWidgetMapper::ManualSubmit);

  mapperItem.addMapping(ui->lineEditPrecoUn, modelItem.fieldIndex("prcUnitario"), "value");
  mapperItem.addMapping(ui->doubleSpinBoxPrecoTotal, modelItem.fieldIndex("parcialDesc"));
  mapperItem.addMapping(ui->lineEditOrcamento, modelItem.fieldIndex("idOrcamento"), "text");
  mapperItem.addMapping(ui->lineEditObs, modelItem.fieldIndex("obs"), "text");
  mapperItem.addMapping(ui->lineEditUn, modelItem.fieldIndex("un"), "text");
  mapperItem.addMapping(ui->lineEditCodComercial, modelItem.fieldIndex("codComercial"));
  mapperItem.addMapping(ui->lineEditFormComercial, modelItem.fieldIndex("formComercial"));
  mapperItem.addMapping(ui->itemBoxProduto, modelItem.fieldIndex("idProduto"), "value");
  mapperItem.addMapping(ui->doubleSpinBoxQte, modelItem.fieldIndex("quant"), "value");
  mapperItem.addMapping(ui->doubleSpinBoxDesconto, modelItem.fieldIndex("desconto"), "value");
}

void Orcamento::registerMode() {
  ui->pushButtonCadastrarOrcamento->show();
  ui->pushButtonAtualizarOrcamento->hide();
  ui->pushButtonReplicar->hide();

  ui->pushButtonApagarOrc->setDisabled(true);
  ui->pushButtonImprimir->setDisabled(true);
  ui->pushButtonGerarVenda->setEnabled(true);
  ui->itemBoxEndereco->setDisabled(true);
}

void Orcamento::updateMode() {
  ui->pushButtonCadastrarOrcamento->hide();
  ui->pushButtonAtualizarOrcamento->show();
  ui->pushButtonReplicar->show();

  ui->pushButtonApagarOrc->setEnabled(true);
  ui->pushButtonImprimir->setEnabled(true);
  ui->pushButtonGerarVenda->setEnabled(true);
  ui->itemBoxEndereco->setVisible(true);
  ui->spinBoxValidade->setDisabled(true);
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
  query.prepare("SELECT * FROM orcamento WHERE idOrcamento = :idOrcamento");
  query.bindValue(":idOrcamento", ui->lineEditOrcamento->text());

  if (not query.exec()) {
    qDebug() << "Erro na query: " << query.lastError();
  }

  if (query.size() != 0) {
    return;
  }

  QString id = UserSession::getSiglaLoja() + "-" + QDate::currentDate().toString("yy");

  query.prepare("SELECT idOrcamento FROM orcamento WHERE idOrcamento LIKE :id UNION SELECT idVenda AS idOrcamento FROM "
                "venda WHERE idVenda LIKE :id ORDER BY idOrcamento ASC");
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
    if (not modelItem.setData(modelItem.index(row, modelItem.fieldIndex(primaryKey)), id)) {
      qDebug() << "Erro setando id nos itens: " << modelItem.lastError();
    }
  }
}

bool Orcamento::verifyFields() { return true; }

bool Orcamento::savingProcedures(const int row) {
  updateId();

  const QString idOrcamento = ui->lineEditOrcamento->text();

  if (model.data(model.index(row, model.fieldIndex("idOrcamento"))).toString() != idOrcamento) {
    if (not setData(row, "idOrcamento", idOrcamento)) {
      qDebug() << "erro setando idOrcamento";
      return false;
    }
  }

  if (not setData(row, "idLoja", UserSession::getLoja())) {
    qDebug() << "erro setando idLoja";
    return false;
  }

  if (not setData(row, "idCliente", ui->itemBoxCliente->value())) {
    qDebug() << "erro setando idCliente";
    return false;
  }

  if (not setData(row, "idEnderecoEntrega", ui->itemBoxEndereco->value())) {
    qDebug() << "erro setando idEnderecoEntrega";
    return false;
  }

  if (not setData(row, "idEnderecoFaturamento", ui->itemBoxEndereco->value())) {
    qDebug() << "erro setando idEnderecoFaturamento";
    return false;
  }

  if (not setData(row, "idUsuario", ui->itemBoxVendedor->value())) {
    qDebug() << "erro setando idUsuario";
    return false;
  }

  if (not setData(row, "idProfissional", ui->itemBoxProfissional->value())) {
    qDebug() << "erro setando idProfissional";
    return false;
  }

  if (not setData(row, "validade", ui->spinBoxValidade->value())) {
    qDebug() << "erro setando validade";
    return false;
  }

  if (not setData(row, "data", ui->dateTimeEdit->dateTime())) {
    qDebug() << "erro setando data";
    return false;
  }

  if (not setData(row, "prazoEntrega", ui->spinBoxPrazoEntrega->value())) {
    qDebug() << "erro setando prazoEntrega";
    return false;
  }

  if (not setData(row, "observacao", ui->textEditObs->toPlainText())) {
    qDebug() << "erro setando observacao";
    return false;
  }

  if (not setData(row, "subTotalBru", ui->doubleSpinBoxSubTotalBruto->value())) {
    qDebug() << "erro setando subTotalBru";
    return false;
  }

  if (not setData(row, "subTotalLiq", ui->doubleSpinBoxSubTotalLiq->value())) {
    qDebug() << "erro setando subTotalLiq";
    return false;
  }

  if (not setData(row, "frete", ui->doubleSpinBoxFrete->value())) {
    qDebug() << "erro setando frete";
    return false;
  }

  if (not setData(row, "descontoPorc", ui->doubleSpinBoxDescontoGlobal->value())) {
    qDebug() << "erro setando descontoPorc";
    return false;
  }

  if (not setData(row, "descontoReais",
                  ui->doubleSpinBoxSubTotalLiq->value() * ui->doubleSpinBoxDescontoGlobal->value() / 100)) {
    qDebug() << "erro setando descontoReais";
    return false;
  }

  if (not setData(row, "total", ui->doubleSpinBoxTotal->value())) {
    qDebug() << "erro setando total";
    return false;
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

  const double quant = ui->doubleSpinBoxQte->value();
  const double prcUn = ui->lineEditPrecoUn->getValue();
  const double desc = ui->doubleSpinBoxDesconto->value() / 100.0;
  const double itemBruto = quant * prcUn;
  const double subTotalItem = itemBruto * (1.0 - desc);

  if (not ui->itemBoxProduto->text().isEmpty()) {
    ui->doubleSpinBoxPrecoTotal->setValue(subTotalItem);
  }
}

void Orcamento::on_doubleSpinBoxDesconto_valueChanged(const double) { calcPrecoItemTotal(); }

void Orcamento::on_doubleSpinBoxQte_valueChanged(const double) {
  const int caixas = qRound(ui->doubleSpinBoxQte->value() / ui->doubleSpinBoxQte->singleStep());

  if (ui->spinBoxCaixas->value() != caixas) {
    ui->spinBoxCaixas->setValue(caixas);
  }
}

void Orcamento::on_pushButtonCadastrarOrcamento_clicked() { save(); }

void Orcamento::on_pushButtonAtualizarOrcamento_clicked() { update(); }

void Orcamento::calcPrecoGlobalTotal(const bool ajusteTotal) {
  double subTotal = 0.;
  double subTotalItens = 0.;
  double subTotalBruto = 0.;
  double minimoFrete = 0., porcFrete = 0.;

  QSqlQuery queryFrete;
  queryFrete.prepare("SELECT * FROM loja WHERE idLoja = :idLoja");
  queryFrete.bindValue(":idLoja", UserSession::getLoja());

  if (not queryFrete.exec() or not queryFrete.next()) {
    qDebug() << "Erro buscando parâmetros do frete: " << queryFrete.lastError();
  }

  minimoFrete = queryFrete.value("valorMinimoFrete").toDouble();
  porcFrete = queryFrete.value("porcentagemFrete").toDouble();

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    const double prcUnItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("prcUnitario"))).toDouble();
    const double qteItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("quant"))).toDouble();
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
    const double final = ui->doubleSpinBoxTotal->value();
    subTotal = final - frete;

    if (subTotalItens == 0.) {
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
  ui->doubleSpinBoxSubTotalLiq->setValue(subTotalItens);
  ui->doubleSpinBoxDescontoGlobal->setValue(descGlobal * 100);
  ui->doubleSpinBoxDescontoGlobal->setPrefix("- R$ " + QString::number(subTotalItens - subTotal, 'f', 2) + " - ");
  ui->doubleSpinBoxFrete->setValue(frete);
  ui->doubleSpinBoxTotal->setValue(subTotal + frete);
}

void Orcamento::on_doubleSpinBoxDescontoGlobal_valueChanged(const double) {
  calcPrecoItemTotal();
  calcPrecoGlobalTotal();
}

void Orcamento::on_doubleSpinBoxTotal_editingFinished() {
  if (modelItem.rowCount() == 0 or ui->doubleSpinBoxSubTotalLiq->value() == 0.) {
    calcPrecoGlobalTotal();
    return;
  }

  const double new_total = ui->doubleSpinBoxTotal->value();
  const double frete = ui->doubleSpinBoxFrete->value();
  const double new_subtotal = new_total - frete;

  if (new_subtotal >= ui->doubleSpinBoxSubTotalLiq->value()) {
    ui->doubleSpinBoxDescontoGlobal->setValue(0.);
    calcPrecoGlobalTotal();
  } else {
    calcPrecoGlobalTotal(true);
  }
}

void Orcamento::on_pushButtonImprimir_clicked() {
  QtRPT *report = new QtRPT(this);
  QFile file(qApp->applicationDirPath() + "/orcamento.xml");

  if (not file.exists()) {
    QMessageBox::warning(this, "Aviso!", "XML da impressão não encontrado!");
    return;
  }

  report->loadReport(file.fileName());
  report->recordCount << ui->tableProdutos->model()->rowCount();
  connect(report, &QtRPT::setValue, this, &Orcamento::setValue);
  report->printExec();
}

void Orcamento::setValue(const int recNo, const QString paramName, QVariant &paramValue, const int reportPage) {
  Q_UNUSED(reportPage);

  QLocale locale;

  QSqlQuery queryCliente;
  queryCliente.prepare("SELECT * FROM cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", model.data(model.index(0, model.fieldIndex("idCliente"))));

  if (not queryCliente.exec() or not queryCliente.first()) {
    qDebug() << "Erro buscando cliente: " << model.fieldIndex("idCliente") << " - " << queryCliente.lastError();
  }

  QSqlQuery queryProfissional;
  queryProfissional.prepare("SELECT * FROM profissional WHERE idProfissional = :idProfissional");
  queryProfissional.bindValue(":idProfissional", model.data(model.index(0, model.fieldIndex("idProfissional"))));

  if (not queryProfissional.exec() or not queryProfissional.first()) {
    qDebug() << "Erro buscando profissional: " << model.fieldIndex("idProfissional") << " - "
             << queryProfissional.lastError();
  }

  QSqlQuery queryVendedor;
  queryVendedor.prepare("SELECT * FROM usuario WHERE idUsuario = :idUsuario");
  queryVendedor.bindValue(":idUsuario", model.data(model.index(0, model.fieldIndex("idUsuario"))));

  if (not queryVendedor.exec() or not queryVendedor.first()) {
    qDebug() << "Erro buscando vendedor: " << model.fieldIndex("idUsuario") << " - " << queryVendedor.lastError();
  }

  QSqlQuery queryProduto;
  queryProduto.prepare("SELECT * FROM produto WHERE idProduto = :idProduto");
  queryProduto.bindValue(":idProduto", modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("idProduto"))));

  if (not queryProduto.exec() or not queryProduto.first()) {
    qDebug() << "Erro buscando produto: " << modelItem.fieldIndex("idProduto") << " - " << queryProduto.lastError();
  }

  QSqlQuery queryLoja;
  queryLoja.prepare("SELECT * FROM loja WHERE idLoja = :idLoja");
  queryLoja.bindValue(":idLoja", UserSession::getLoja());

  if (not queryLoja.exec() or not queryLoja.first()) {
    qDebug() << "Erro buscando loja: " << modelItem.fieldIndex("idLoja") << " - " << queryLoja.lastError();
  }

  QSqlQuery queryLojaEnd;
  queryLojaEnd.prepare("SELECT * FROM loja_has_endereco WHERE idLoja = :idLoja");
  queryLojaEnd.bindValue(":idLoja", UserSession::getLoja());

  if (not queryLojaEnd.exec() or not queryLojaEnd.first()) {
    qDebug() << "Erro buscando loja end.: " << modelItem.fieldIndex("idLoja") << " - " << queryLojaEnd.lastError();
  }

  // REPORT TITLE
  if (paramName == "Unidade") {
    paramValue = "Unidade " + queryLoja.value("descricao").toString();
  }

  if (paramName == "Endereco") {
    paramValue = queryLojaEnd.value("logradouro").toString() + ", " + queryLojaEnd.value("numero").toString() + " - " +
                 queryLojaEnd.value("bairro").toString() + "\n" + queryLojaEnd.value("cidade").toString() + " - " +
                 queryLojaEnd.value("uf").toString() + " - CEP: " + queryLojaEnd.value("cep").toString() + "\n" +
                 queryLoja.value("tel").toString() + " - " + queryLoja.value("tel2").toString();
  }

  if (paramName == "orcamento") {
    paramValue = ui->lineEditOrcamento->text();
  }

  if (paramName == "data") {
    paramValue = ui->dateTimeEdit->dateTime().toString("hh:mm dd-MM-yyyy");
  }

  if (paramName == "validade") {
    paramValue = QString::number(ui->spinBoxValidade->value()) + " dias";
  }

  if (paramName == "cliente") {
    paramValue = ui->itemBoxCliente->text();
  }

  if (paramName == "cpfcnpj") {
    if (queryCliente.value("pfpj").toString() == "PF") {
      paramValue = queryCliente.value("cpf").toString();
    } else if (queryCliente.value("pfpj").toString() == "PJ") {
      paramValue = queryCliente.value("cnpj").toString();
    }
  }

  if (paramName == "email") {
    paramValue = queryCliente.value("email").toString();
  }

  if (paramName == "tel1") {
    paramValue = queryCliente.value("tel").toString();
  }

  if (paramName == "tel2") {
    paramValue = queryCliente.value("telCel").toString();
  }

  if (paramName == "profissional") {
    if (ui->itemBoxProfissional->text().isEmpty()) {
      paramValue = "Não há";
    } else {
      paramValue = ui->itemBoxProfissional->text();
    }
  }

  if (paramName == "telprofissional") {
    paramValue = queryProfissional.value("tel").toString();
  }

  if (paramName == "emailprofissional") {
    paramValue = queryProfissional.value("email").toString();
  }

  if (paramName == "vendedor") {
    paramValue = ui->itemBoxVendedor->text();
  }

  if (paramName == "emailvendedor") {
  }

  if (paramName == "estoque") {
  }

  if (paramName == "dataestoque") {
  }

  // MASTER BAND
  if (paramName == "Marca") {
    paramValue = modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("fornecedor"))).toString();
  }

  if (paramName == "Código") {
    paramValue = queryProduto.value("codComercial").toString();
  }

  if (paramName == "Nome do produto") {
    if (modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("formComercial"))).toString().isEmpty()) {
      paramValue = modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("produto"))).toString();
    } else {
      paramValue = modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("produto"))).toString() + " (" +
                   modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("formComercial"))).toString() + ")";
    }
  }

  if (paramName == "Ambiente") {
    paramValue = modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("obs"))).toString();
  }

  if (paramName == "Preço-R$") {
    double desconto = modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("desconto"))).toDouble();
    if (desconto == 0) {
      double value = modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("prcUnitario"))).toDouble();
      paramValue = "R$ " + locale.toString(value, 'f', 2);
    } else {
      double value = modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("prcUnitario"))).toDouble();
      paramValue = "(R$ " + locale.toString(value, 'f', 2) + " -" + locale.toString(desconto, 'f', 1) + "%)\n" + "R$ " +
                   locale.toString(value * (1 - (desconto / 100)), 'f', 2);
    }
  }

  if (paramName == "Quant.") {
    paramValue = modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("quant"))).toString();
  }

  if (paramName == "Unid.") {
    paramValue = modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("un"))).toString();
  }

  if (paramName == "TotalProd") {
    double parcial = modelItem.data(modelItem.index(recNo, modelItem.fieldIndex("parcialDesc"))).toDouble();
    paramValue = "R$ " + locale.toString(parcial, 'f', 2);
  }

  // REPORT SUMMARY
  if (paramName == "Soma") {
    double value = ui->doubleSpinBoxSubTotalLiq->value();
    paramValue = locale.toString(value, 'f', 2);
  }

  if (paramName == "Desconto") {
    double value = ui->doubleSpinBoxDescontoGlobal->value();
    paramValue = locale.toString(value, 'f', 2);
  }

  if (paramName == "Total") {
    double value = ui->doubleSpinBoxSubTotalLiq->value() -
                   (ui->doubleSpinBoxSubTotalLiq->value() * ui->doubleSpinBoxDescontoGlobal->value() / 100);
    paramValue = locale.toString(value, 'f', 2);
  }

  if (paramName == "Frete") {
    double value = ui->doubleSpinBoxFrete->value();
    paramValue = locale.toString(value, 'f', 2);
  }

  if (paramName == "TotalFinal") {
    double value = ui->doubleSpinBoxTotal->value();
    paramValue = locale.toString(value, 'f', 2);
  }

  if (paramName == "Observacao") {
    paramValue = ui->textEditObs->toPlainText();
  }
}

QString Orcamento::itemData(const int row, const QString key) {
  return modelItem.data(modelItem.index(row, modelItem.fieldIndex(key))).toString();
}

void Orcamento::setupTables() {
  modelItem.setTable("orcamento_has_produto");
  modelItem.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelItem.setHeaderData(modelItem.fieldIndex("produto"), Qt::Horizontal, "Produto");
  modelItem.setHeaderData(modelItem.fieldIndex("fornecedor"), Qt::Horizontal, "Fornecedor");
  modelItem.setHeaderData(modelItem.fieldIndex("obs"), Qt::Horizontal, "Obs.");
  modelItem.setHeaderData(modelItem.fieldIndex("prcUnitario"), Qt::Horizontal, "Preço/Un.");
  modelItem.setHeaderData(modelItem.fieldIndex("caixas"), Qt::Horizontal, "Caixas");
  modelItem.setHeaderData(modelItem.fieldIndex("quant"), Qt::Horizontal, "Quant.");
  modelItem.setHeaderData(modelItem.fieldIndex("un"), Qt::Horizontal, "Un.");
  modelItem.setHeaderData(modelItem.fieldIndex("codComercial"), Qt::Horizontal, "Código");
  modelItem.setHeaderData(modelItem.fieldIndex("formComercial"), Qt::Horizontal, "Formato");
  modelItem.setHeaderData(modelItem.fieldIndex("unCaixa"), Qt::Horizontal, "Un./Caixa");
  modelItem.setHeaderData(modelItem.fieldIndex("parcial"), Qt::Horizontal, "Subtotal");
  modelItem.setHeaderData(modelItem.fieldIndex("desconto"), Qt::Horizontal, "Desc. %");
  modelItem.setHeaderData(modelItem.fieldIndex("parcialDesc"), Qt::Horizontal, "Total");

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

  ui->tableProdutos->setItemDelegate(new DoubleDelegate(this));

  ui->tableProdutos->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableProdutos->horizontalHeader()->setResizeContentsPrecision(0);
}

bool Orcamento::verificaCampos() {
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
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("quant")), ui->doubleSpinBoxQte->value());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("unCaixa")), ui->doubleSpinBoxQte->singleStep());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("un")), ui->lineEditUn->text());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("codComercial")), ui->lineEditCodComercial->text());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("formComercial")), ui->lineEditFormComercial->text());
  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("desconto")), ui->doubleSpinBoxDesconto->value());

  calcPrecoGlobalTotal();

  modelItem.setData(modelItem.index(row, modelItem.fieldIndex("parcialDesc")), ui->doubleSpinBoxPrecoTotal->value());

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
  queryCadastro.prepare("SELECT incompleto FROM orcamento LEFT JOIN cliente ON orcamento.idCliente = cliente.idCliente "
                        "WHERE cliente.idCliente = :idCliente AND incompleto = TRUE");
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

void Orcamento::on_doubleSpinBoxSubTotalLiq_valueChanged(const double) { calcPrecoGlobalTotal(); }

void Orcamento::on_spinBoxCaixas_valueChanged(const int caixas) {
  const double quant = caixas * ui->doubleSpinBoxQte->singleStep();

  if (ui->doubleSpinBoxQte->value() != quant) {
    ui->doubleSpinBoxQte->setValue(quant);
  }

  calcPrecoItemTotal();
}

void Orcamento::on_pushButtonApagarOrc_clicked() {
  ApagaOrcamento *apaga = new ApagaOrcamento(this);
  apaga->apagar(mapper.currentIndex());
}

void Orcamento::on_itemBoxProduto_textChanged(const QString &text) {
  Q_UNUSED(text);

  ui->doubleSpinBoxQte->setValue(0.);
  ui->spinBoxCaixas->setValue(0.);
  ui->doubleSpinBoxDesconto->setValue(0.);

  if (ui->itemBoxProduto->text().isEmpty()) {
    ui->spinBoxCaixas->setDisabled(true);
    ui->doubleSpinBoxQte->setDisabled(true);
    ui->doubleSpinBoxDesconto->setDisabled(true);
    ui->doubleSpinBoxQte->setSingleStep(1.0);
    ui->doubleSpinBoxPrecoTotal->clear();
    ui->doubleSpinBoxPrecoTotal->setDisabled(true);
    ui->lineEditFornecedor->clear();
    ui->lineEditPrecoUn->clear();
    ui->lineEditPrecoUn->setDisabled(true);
    ui->lineEditUn->clear();
    ui->lineEditObs->clear();
    ui->lineEditCodComercial->clear();
    ui->lineEditFormComercial->clear();
    ui->lineEditEstoque->clear();
    return;
  }

  QSqlQuery query;
  query.prepare("SELECT * FROM produto WHERE idProduto = :index");
  query.bindValue(":index", ui->itemBoxProduto->value().toInt());

  if (not query.exec() or not query.first()) {
    qDebug() << "Erro na busca do produto: " << query.lastError();
  }

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
  queryCliente.prepare("SELECT idProfissionalRel FROM cliente WHERE idCliente = :idCliente");
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
  orcamento->ui->doubleSpinBoxTotal->setValue(model.data(model.index(row, model.fieldIndex("total"))).toDouble());
  orcamento->ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

  // MODELITEM
  for (int i = 0; i < modelItem.rowCount(); ++i) {
    orcamento->ui->itemBoxProduto->setValue(modelItem.data(modelItem.index(i, modelItem.fieldIndex("idProduto"))));
    orcamento->ui->doubleSpinBoxQte->setValue(
          modelItem.data(modelItem.index(i, modelItem.fieldIndex("quant"))).toDouble());
    orcamento->adicionarItem();
  }

  orcamento->show();
}

void Orcamento::on_doubleSpinBoxPrecoTotal_editingFinished() {
  if (ui->itemBoxProduto->text().isEmpty()) {
    return;
  }

  double quant = ui->doubleSpinBoxQte->value();
  double prcUn = ui->lineEditPrecoUn->getValue();
  double itemBruto = quant * prcUn;
  double subTotalItem = ui->doubleSpinBoxPrecoTotal->value();
  double desconto = (itemBruto - subTotalItem) / itemBruto * 100;

  ui->doubleSpinBoxDesconto->setValue(desconto);
}

#ifdef TEST
void Orcamento::testaOrcamento() {
  ui->itemBoxCliente->setValue(17);
  ui->itemBoxEndereco->setValue(31);
  ui->itemBoxProfissional->setValue(5);
  ui->itemBoxProduto->setValue(4000);
  ui->doubleSpinBoxQte->setValue(10);
  adicionarItem();
  silent = true;
  save();
  close();
}
#endif

bool Orcamento::save(const bool isUpdate) {
  if (not verificaCampos()) {
    return false;
  }

  QSqlQuery("SET SESSION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  const int row = (isUpdate) ? mapper.currentIndex() : model.rowCount();

  if (row == -1) {
    qDebug() << "Something went very wrong!";
    return false;
  }

  if (not isUpdate) {
    model.insertRow(row);
  }

  if (not savingProcedures(row)) {
    errorMessage();
    QSqlQuery("ROLLBACK").exec();
    return false;
  }

  if (not model.submitAll()) {
    qDebug() << objectName() << " : " << model.lastError();
    qDebug() << "Last query: "
             << model.database().driver()->sqlStatement(QSqlDriver::InsertStatement, model.tableName(),
                                                        model.record(row), false);
    errorMessage();
    QSqlQuery("ROLLBACK").exec();
    return false;
  }

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    modelItem.setData(model.index(row, modelItem.fieldIndex(primaryKey)), ui->lineEditOrcamento->text());
    modelItem.setData(model.index(row, modelItem.fieldIndex("idLoja")), UserSession::getLoja());
  }

  if (not modelItem.submitAll()) {
    qDebug() << "Failed to add item! : " << modelItem.lastError().text();
    qDebug() << "Last query: "
             << modelItem.database().driver()->sqlStatement(QSqlDriver::InsertStatement, modelItem.tableName(),
                                                            modelItem.record(row), false);
    QMessageBox::warning(this, "Atenção!", "Erro ao adicionar um item ao orçamento.", QMessageBox::Ok,
                         QMessageBox::NoButton);
    return false;
  }

  QSqlQuery("COMMIT").exec();
  isDirty = false;

  viewRegister(model.index(row, 0));
  sendUpdateMessage();

  if (not silent) {
    successMessage();
  }

  return true;
}

void Orcamento::on_pushButtonGerarExcel_clicked() {
  QXlsx::Document xlsx("modelo.xlsx");

  QString idOrcamento = model.data(model.index(mapper.currentIndex(), model.fieldIndex("idOrcamento"))).toString();

  QSqlQuery queryOrc;
  queryOrc.prepare("SELECT * FROM orcamento WHERE idOrcamento = :idOrcamento");
  queryOrc.bindValue(":idOrcamento", idOrcamento);

  if (not queryOrc.exec() or not queryOrc.first()) {
    qDebug() << "Erro buscando dados do orcamento:" << queryOrc.lastError();
  }

  QSqlQuery queryLoja;
  queryLoja.prepare(
        "SELECT * FROM loja WHERE idLoja = (SELECT idLoja FROM orcamento WHERE idOrcamento = :idOrcamento)");
  queryLoja.bindValue(":idOrcamento", idOrcamento);

  if (not queryLoja.exec() or not queryLoja.first()) {
    qDebug() << "Erro buscando dados da loja:" << queryLoja.lastError();
  }

  QSqlQuery queryUsuario;
  queryUsuario.prepare(
        "SELECT * FROM usuario WHERE idUsuario = (SELECT idUsuario FROM orcamento WHERE idOrcamento = :idOrcamento)");
  queryUsuario.bindValue(":idOrcamento", idOrcamento);

  if (not queryUsuario.exec() or not queryUsuario.first()) {
    qDebug() << "Erro buscando dados do usuario: " << queryUsuario.lastError();
  }

  QSqlQuery queryCliente;
  queryCliente.prepare(
        "SELECT * FROM cliente WHERE idCliente = (SELECT idCliente FROM orcamento WHERE idOrcamento = :idOrcamento)");
  queryCliente.bindValue(":idOrcamento", idOrcamento);

  if (not queryCliente.exec() or not queryCliente.first()) {
    qDebug() << "Erro buscando dados do cliente: " << queryCliente.lastError();
  }

  QSqlQuery queryEndEnt;
  queryEndEnt.prepare("SELECT * FROM cliente_has_endereco WHERE idEndereco = (SELECT idEnderecoEntrega FROM orcamento "
                      "WHERE idOrcamento = :idOrcamento)");
  queryEndEnt.bindValue(":idOrcamento", idOrcamento);

  if (not queryEndEnt.exec() or not queryEndEnt.first()) {
    qDebug() << "Erro buscando dados do endereco: " << queryEndEnt.lastError();
  }

  QSqlQuery queryEndFat;
  queryEndFat.prepare(
        "SELECT * FROM cliente_has_endereco WHERE idEndereco = (SELECT idEnderecoFaturamento FROM orcamento "
        "WHERE idOrcamento = :idOrcamento)");
  queryEndFat.bindValue(":idOrcamento", idOrcamento);

  if (not queryEndFat.exec() or not queryEndFat.first()) {
    qDebug() << "Erro buscando dados do endereco: " << queryEndFat.lastError();
  }

  QSqlQuery queryProf;
  queryProf.prepare("SELECT * FROM profissional WHERE idProfissional = (SELECT idProfissional FROM orcamento WHERE "
                    "idOrcamento = :idOrcamento)");
  queryProf.bindValue(":idOrcamento", idOrcamento);

  if (not queryProf.exec() or not queryProf.first()) {
    qDebug() << "Erro buscando dados do profissional: " << queryProf.lastError();
  }

  xlsx.write("D2", queryOrc.value("idOrcamento"));
  xlsx.write("D3", queryCliente.value("nome_razao"));
  xlsx.write("D4", queryCliente.value("email"));
  xlsx.write("D5", queryEndFat.value("logradouro").toString() + " " + queryEndFat.value("numero").toString() + " - " +
             queryEndFat.value("bairro").toString() + ", " + queryEndFat.value("cidade").toString());
  xlsx.write("D6", queryEndEnt.value("logradouro").toString() + " " + queryEndEnt.value("numero").toString() + " - " +
             queryEndEnt.value("bairro").toString() + ", " + queryEndEnt.value("cidade").toString());
  xlsx.write("D7", queryProf.value("nome_razao").toString());
  xlsx.write("D8", queryUsuario.value("nome").toString());
  xlsx.write("F8", queryUsuario.value("email").toString());
  xlsx.write("M2", queryOrc.value("data").toDateTime().toString("dd/MM/yyyy hh:mm"));
  xlsx.write("M3", queryCliente.value("pfpj").toString() == "PF" ? queryCliente.value("cpf").toString()
                                                                 : queryCliente.value("cnpj").toString());
  xlsx.write("M4", queryCliente.value("tel").toString());
  xlsx.write("M5", queryEndFat.value("cep").toString());
  xlsx.write("M6", queryEndEnt.value("cep").toString());
  xlsx.write("H7", queryProf.value("tel").toString());
  xlsx.write("K7", queryProf.value("email").toString());

  for (int i = 0; i < modelItem.rowCount(); ++i) {
    xlsx.write("A" + QString::number(12 + i),
               modelItem.data(modelItem.index(i, modelItem.fieldIndex("fornecedor"))).toString());
    xlsx.write("B" + QString::number(12 + i),
               modelItem.data(modelItem.index(i, modelItem.fieldIndex("codComercial"))).toString());
    xlsx.write("C" + QString::number(12 + i),
               modelItem.data(modelItem.index(i, modelItem.fieldIndex("produto"))).toString());
    xlsx.write("H" + QString::number(12 + i),
               modelItem.data(modelItem.index(i, modelItem.fieldIndex("obs"))).toString());
    xlsx.write("K" + QString::number(12 + i),
               modelItem.data(modelItem.index(i, modelItem.fieldIndex("prcUnitario"))).toDouble());
    xlsx.write("L" + QString::number(12 + i),
               modelItem.data(modelItem.index(i, modelItem.fieldIndex("quant"))).toDouble());
    xlsx.write("M" + QString::number(12 + i),
               modelItem.data(modelItem.index(i, modelItem.fieldIndex("un"))).toString());
  }

  if (xlsx.saveAs(model.data(model.index(mapper.currentIndex(), model.fieldIndex("idOrcamento"))).toString() +
                  ".xlsx")) {
    QMessageBox::information(
          this, "Ok!", "Arquivo salvo como " +
          model.data(model.index(mapper.currentIndex(), model.fieldIndex("idOrcamento"))).toString() +
          ".xlsx");
  } else {
    QMessageBox::warning(this, "Aviso!", "Ocorreu algum erro ao salvar o arquivo.");
  }
}
