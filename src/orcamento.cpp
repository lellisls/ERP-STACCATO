#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDir>
#include <QFileDialog>
#include <QDesktopServices>

#include "orcamento.h"
#include "ui_orcamento.h"
#include "qtrpt.h"
#include "cadastrocliente.h"
#include "usersession.h"
#include "venda.h"
#include "apagaorcamento.h"
#include "doubledelegate.h"
#include "xlsxdocument.h"

Orcamento::Orcamento(QWidget *parent) : RegisterDialog("orcamento", "idOrcamento", parent), ui(new Ui::Orcamento) {
  ui->setupUi(this);

  setupTables();

  for (const auto *line : findChildren<QLineEdit *>()) {
    connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);
  }

  ui->itemBoxCliente->setSearchDialog(SearchDialog::cliente(this));
  ui->itemBoxCliente->setRegisterDialog(new CadastroCliente(this));
  ui->itemBoxProduto->setSearchDialog(SearchDialog::produto(this));
  ui->itemBoxVendedor->setSearchDialog(SearchDialog::vendedor(this));
  ui->itemBoxProfissional->setSearchDialog(SearchDialog::profissional(this));
  ui->itemBoxEndereco->setSearchDialog(SearchDialog::enderecoCliente(this));

  setupMapper();
  newRegister();

  if (UserSession::getTipoUsuario() == "ADMINISTRADOR") {
    ui->dateTimeEdit->setReadOnly(false);
    ui->dateTimeEdit->setCalendarPopup(true);
    ui->checkBoxFreteManual->show();
  } else {
    //    ui->checkBoxFreteManual->hide();
    // NOTE: remove this later
    ui->dateTimeEdit->setReadOnly(false);
    ui->dateTimeEdit->setCalendarPopup(true);
  }

  on_checkBoxRepresentacao_toggled(false);
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
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela orcamento_has_produto: " + modelItem.lastError().text());
    return false;
  }

  novoItem();

  calcPrecoGlobalTotal();

  ui->doubleSpinBoxSubTotalBruto->setValue(model.data(mapper.currentIndex(), "subTotalBru").toDouble());
  ui->doubleSpinBoxSubTotalLiq->setValue(model.data(mapper.currentIndex(), "subTotalLiq").toDouble());
  ui->doubleSpinBoxDescontoGlobal->setValue(model.data(mapper.currentIndex(), "descontoPorc").toDouble());
  ui->doubleSpinBoxDescontoGlobal->setPrefix(
        "- R$ " + QString::number(model.data(mapper.currentIndex(), "descontoReais").toDouble(), 'f', 2) + " - ");
  ui->doubleSpinBoxFrete->setValue(model.data(mapper.currentIndex(), "frete").toDouble());
  ui->doubleSpinBoxTotal->setValue(model.data(mapper.currentIndex(), "total").toDouble());

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

  ui->checkBoxRepresentacao->setDisabled(true);

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
  addMapping(ui->doubleSpinBoxSubTotalBruto, "subTotalBru");
  addMapping(ui->doubleSpinBoxSubTotalLiq, "subTotalLiq");
  addMapping(ui->checkBoxRepresentacao, "representacao");

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
  ui->pushButtonGerarExcel->setDisabled(true);
  ui->pushButtonImprimir->setDisabled(true);
  ui->pushButtonGerarVenda->setEnabled(true);
  ui->itemBoxEndereco->setDisabled(true);
}

void Orcamento::updateMode() {
  ui->pushButtonCadastrarOrcamento->hide();
  ui->pushButtonAtualizarOrcamento->show();
  ui->pushButtonReplicar->show();

  ui->pushButtonApagarOrc->setEnabled(true);
  ui->pushButtonGerarExcel->setEnabled(true);
  ui->pushButtonImprimir->setEnabled(true);
  ui->pushButtonGerarVenda->setEnabled(true);
  ui->itemBoxEndereco->setVisible(true);
  ui->spinBoxValidade->setDisabled(true);
}

bool Orcamento::newRegister() {
  if (not RegisterDialog::newRegister()) {
    return false;
  }

  ui->lineEditOrcamento->setText("Auto gerado");
  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
  ui->spinBoxValidade->setValue(7);
  novoItem();

  return true;
}

void Orcamento::removeItem() {
  modelItem.removeRow(ui->tableProdutos->currentIndex().row());
  calcPrecoGlobalTotal();

  if (modelItem.rowCount() == 0) {
    ui->checkBoxRepresentacao->setEnabled(true);
  }
}

void Orcamento::updateId() {
  QSqlQuery query;
  query.prepare("SELECT * FROM orcamento WHERE idOrcamento = :idOrcamento");
  query.bindValue(":idOrcamento", ui->lineEditOrcamento->text());

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro na query: " + query.lastError().text());
    return;
  }

  if (query.size() != 0) {
    return;
  }

  query.prepare(
        "SELECT loja.sigla FROM usuario LEFT JOIN loja ON usuario.idLoja = loja.idLoja WHERE usuario.nome = :nome");
  query.bindValue(":nome", ui->itemBoxVendedor->text());

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro na query: " + query.lastError().text());
    return;
  }

  QString siglaLoja = query.value(0).toString();

  QString id = siglaLoja + "-" + QDate::currentDate().toString("yy");

  query.prepare("SELECT idOrcamento FROM orcamento WHERE idOrcamento LIKE :id UNION SELECT idVenda AS idOrcamento FROM "
                "venda WHERE idVenda LIKE :id ORDER BY idOrcamento ASC");
  query.bindValue(":id", id + "%");

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro na query: " + query.lastError().text());
    return;
  }

  int last = 0;

  if (query.size() > 0) {
    query.last();
    QString temp = query.value("idOrcamento").toString().mid(id.size());

    if (temp.endsWith("R")) {
      temp = temp.remove("R");
    }

    last = temp.toInt();
  }

  id += QString("%1").arg(last + 1, 4, 10, QChar('0'));
  ui->lineEditOrcamento->setText(ui->checkBoxRepresentacao->isChecked() ? id + "R" : id);

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    if (not modelItem.setData(row, primaryKey, id)) {
      QMessageBox::critical(this, "Erro!", "Erro guardando id nos itens: " + modelItem.lastError().text());
      return;
    }
  }
}

bool Orcamento::verifyFields() {
  if (UserSession::getTipoUsuario() == "ADMINISTRADOR" and UserSession::getNome() == ui->itemBoxVendedor->text()) {
    QMessageBox::critical(this, "Erro!", "Administrador não pode cadastrar, escolha outro vendedor.");
    return false;
  }

  if (ui->itemBoxCliente->text().isEmpty()) {
    ui->itemBoxCliente->setFocus();
    QMessageBox::critical(this, "Erro!", "Cliente inválido!");
    return false;
  }

  if (ui->itemBoxVendedor->text().isEmpty()) {
    ui->itemBoxVendedor->setFocus();
    QMessageBox::critical(this, "Erro!", "Vendedor inválido!");
    return false;
  }

  if (ui->itemBoxProfissional->text().isEmpty()) {
    ui->itemBoxProfissional->setFocus();
    QMessageBox::critical(this, "Erro!", "Profissional inválido!");
    return false;
  }

  if (ui->itemBoxEndereco->text().isEmpty()) {
    ui->itemBoxEndereco->setFocus();
    QMessageBox::critical(this, "Erro!", "Endereço inválido! Se não possui endereço, escolha \"Não há\".");
    return false;
  }

  if (modelItem.rowCount() == 0) {
    ui->itemBoxProduto->setFocus();
    QMessageBox::critical(this, "Erro!", "Não pode cadastrar um orçamento sem itens!");
    return false;
  }

  return true;
}

bool Orcamento::savingProcedures() {
  updateId();

  const QString idOrcamento = ui->lineEditOrcamento->text();

  setData("idOrcamento", idOrcamento);
  setData("idLoja", UserSession::getLoja());
  setData("idCliente", ui->itemBoxCliente->value());
  setData("idEnderecoEntrega", ui->itemBoxEndereco->value());
  setData("idEnderecoFaturamento", ui->itemBoxEndereco->value());
  setData("idUsuario", ui->itemBoxVendedor->value());
  setData("idProfissional", ui->itemBoxProfissional->value());
  setData("validade", ui->spinBoxValidade->value());
  setData("data", ui->dateTimeEdit->dateTime());
  setData("prazoEntrega", ui->spinBoxPrazoEntrega->value());
  setData("observacao", ui->textEditObs->toPlainText());
  setData("subTotalBru", ui->doubleSpinBoxSubTotalBruto->value());
  setData("subTotalLiq", ui->doubleSpinBoxSubTotalLiq->value());
  setData("frete", ui->doubleSpinBoxFrete->value());
  setData("descontoPorc", ui->doubleSpinBoxDescontoGlobal->value());
  setData("descontoReais", ui->doubleSpinBoxSubTotalLiq->value() * ui->doubleSpinBoxDescontoGlobal->value() / 100.);
  setData("total", ui->doubleSpinBoxTotal->value());
  setData("representacao", ui->checkBoxRepresentacao->isChecked());

  return isOk;
}

void Orcamento::clearFields() {
  RegisterDialog::clearFields();
  ui->itemBoxVendedor->setValue(UserSession::getIdUsuario());
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
  QSqlQuery queryFrete;
  queryFrete.prepare("SELECT * FROM loja WHERE idLoja = :idLoja");
  queryFrete.bindValue(":idLoja", UserSession::getLoja());

  if (not queryFrete.exec() or not queryFrete.next()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando parâmetros do frete: " + queryFrete.lastError().text());
    return;
  }

  const double minimoFrete = queryFrete.value("valorMinimoFrete").toDouble();
  const double porcFrete = queryFrete.value("porcentagemFrete").toDouble();

  double subTotalItens = 0.;
  double subTotalBruto = 0.;

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    const double itemBruto = modelItem.data(row, "quant").toDouble() * modelItem.data(row, "prcUnitario").toDouble();
    const double descItem = modelItem.data(row, "desconto").toDouble() / 100.;
    const double stItem = itemBruto * (1. - descItem);
    subTotalBruto += itemBruto;
    subTotalItens += stItem;
    modelItem.setData(row, "parcial", itemBruto);
    modelItem.setData(row, "parcialDesc", stItem);
  }

  const double frete = ui->checkBoxFreteManual->isChecked() ? ui->doubleSpinBoxFrete->value()
                                                            : qMax(subTotalBruto * porcFrete / 100., minimoFrete);

  double descGlobal = ui->doubleSpinBoxDescontoGlobal->value() / 100.;
  double subTotal = subTotalItens * (1. - descGlobal);

  if (ajusteTotal) {
    subTotal = ui->doubleSpinBoxTotal->value() - frete;
    descGlobal = subTotalItens == 0. ? 0. : 1. - (subTotal / subTotalItens);
  }

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    modelItem.setData(row, "descGlobal", descGlobal * 100.);
    modelItem.setData(row, "total", modelItem.data(row, "parcialDesc").toDouble() * (1. - descGlobal));
  }

  ui->doubleSpinBoxSubTotalBruto->setValue(subTotalBruto);
  ui->doubleSpinBoxSubTotalLiq->setValue(subTotalItens);
  ui->doubleSpinBoxDescontoGlobal->setValue(descGlobal * 100.);
  ui->doubleSpinBoxDescontoGlobal->setPrefix("- R$ " + QString::number(subTotalItens - subTotal, 'f', 2) + " - ");

  if (ui->doubleSpinBoxFrete->value() == 0.) {
    ui->doubleSpinBoxFrete->setValue(frete);
  }

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
  if (settings("User/userFolder").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Não há uma pasta definida para salvar PDF/Excel. Por favor escolha uma.");
    setSettings("User/userFolder", QFileDialog::getExistingDirectory(this, "Pasta PDF/Excel"));
    return;
  }

  if (settings("User/userFolder").toString().isEmpty()) {
    return;
  }

  QString path = settings("User/userFolder").toString();

  QDir dir(path);

  if (not dir.exists()) {
    dir.mkdir(path);
  }

  QFile file(qApp->applicationDirPath() + "/orcamento.xml");

  if (not file.exists()) {
    QMessageBox::critical(this, "Erro!", "XML da impressão não encontrado!");
    return;
  }

  QtRPT *report = new QtRPT(this);

  queryCliente.prepare("SELECT * FROM cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", data("idCliente"));

  if (not queryCliente.exec() or not queryCliente.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando cliente: " + queryCliente.lastError().text());
    return;
  }

  queryProfissional.prepare("SELECT * FROM profissional WHERE idProfissional = :idProfissional");
  queryProfissional.bindValue(":idProfissional", data("idProfissional"));

  if (not queryProfissional.exec() or not queryProfissional.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando profissional: " + queryProfissional.lastError().text());
    return;
  }

  queryVendedor.prepare("SELECT * FROM usuario WHERE idUsuario = :idUsuario");
  queryVendedor.bindValue(":idUsuario", data("idUsuario"));

  if (not queryVendedor.exec() or not queryVendedor.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando vendedor: " + queryVendedor.lastError().text());
    return;
  }

  queryLoja.prepare("SELECT * FROM loja WHERE idLoja = :idLoja");
  queryLoja.bindValue(":idLoja", data("idLoja"));

  if (not queryLoja.exec() or not queryLoja.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando loja: " + queryLoja.lastError().text());
    return;
  }

  queryLojaEnd.prepare("SELECT * FROM loja_has_endereco WHERE idLoja = :idLoja");
  queryLojaEnd.bindValue(":idLoja", data("idLoja"));

  if (not queryLojaEnd.exec() or not queryLojaEnd.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando endereço loja: " + queryLojaEnd.lastError().text());
    return;
  }

  report->loadReport(file.fileName());
  report->recordCount << ui->tableProdutos->model()->rowCount();
  connect(report, &QtRPT::setValue, this, &Orcamento::setValue);

  report->printPDF(path + "/" + ui->lineEditOrcamento->text() + ".pdf");
}

void Orcamento::setValue(const int recNo, const QString paramName, QVariant &paramValue, const int reportPage) {
  Q_UNUSED(reportPage);

  QLocale locale;

  if (modelItem.data(recNo, "idProduto") != queryProduto.boundValue(":idProduto")) {
    queryProduto.prepare("SELECT * FROM produto WHERE idProduto = :idProduto");
    queryProduto.bindValue(":idProduto", modelItem.data(recNo, "idProduto"));

    if (not queryProduto.exec() or not queryProduto.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando produto: " + queryProduto.lastError().text());
      return;
    }
  }

  // REPORT TITLE
  if (paramName == "Loja") {
    paramValue = queryLoja.value("descricao").toString();
    return;
  }

  if (paramName == "Endereco") {
    paramValue = queryLojaEnd.value("logradouro").toString() + ", " + queryLojaEnd.value("numero").toString() + " - " +
                 queryLojaEnd.value("bairro").toString() + "\n" + queryLojaEnd.value("cidade").toString() + " - " +
                 queryLojaEnd.value("uf").toString() + " - CEP: " + queryLojaEnd.value("cep").toString() + "\n" +
                 queryLoja.value("tel").toString() + " - " + queryLoja.value("tel2").toString();
    return;
  }

  if (paramName == "orcamento") {
    paramValue = ui->lineEditOrcamento->text();
    return;
  }

  if (paramName == "data") {
    paramValue = ui->dateTimeEdit->dateTime().toString("hh:mm dd-MM-yyyy");
    return;
  }

  if (paramName == "validade") {
    paramValue = QString::number(ui->spinBoxValidade->value()) + " dias";
    return;
  }

  if (paramName == "cliente") {
    paramValue = ui->itemBoxCliente->text().left(30);
    return;
  }

  if (paramName == "cpfcnpj") {
    paramValue = queryCliente.value(queryCliente.value("pfpj").toString() == "PF" ? "cpf" : "cnpj").toString();
    return;
  }

  if (paramName == "email") {
    paramValue = queryCliente.value("email").toString();
    return;
  }

  if (paramName == "tel1") {
    paramValue = queryCliente.value("tel").toString();
    return;
  }

  if (paramName == "tel2") {
    paramValue = queryCliente.value("telCel").toString();
    return;
  }

  if (paramName == "profissional") {
    QString profissional = ui->itemBoxProfissional->text();

    paramValue = profissional.isEmpty() ? "Não há" : profissional;
    return;
  }

  if (paramName == "telprofissional") {
    paramValue = queryProfissional.value("tel").toString();
    return;
  }

  if (paramName == "emailprofissional") {
    paramValue = queryProfissional.value("email").toString();
    return;
  }

  if (paramName == "vendedor") {
    paramValue = ui->itemBoxVendedor->text();
    return;
  }

  if (paramName == "emailvendedor") {
    return;
  }

  if (paramName == "estoque") {
    return;
  }

  if (paramName == "dataestoque") {
    return;
  }

  // MASTER BAND
  if (paramName == "Marca") {
    paramValue = modelItem.data(recNo, "fornecedor").toString();
    return;
  }

  if (paramName == "Código") {
    paramValue = queryProduto.value("codComercial").toString();
    return;
  }

  if (paramName == "Nome do produto") {
    QString formComercial = modelItem.data(recNo, "formComercial").toString();

    paramValue =
        modelItem.data(recNo, "produto").toString() + (formComercial.isEmpty() ? "" : " (" + formComercial + ")");
    return;
  }

  if (paramName == "Ambiente") {
    paramValue = modelItem.data(recNo, "obs").toString();
    return;
  }

  if (paramName == "Preço-R$") {
    double desconto = modelItem.data(recNo, "desconto").toDouble();
    double value = modelItem.data(recNo, "prcUnitario").toDouble();

    if (desconto > 0) {
      paramValue = "(R$ " + locale.toString(value, 'f', 2) + " -" + locale.toString(desconto, 'f', 1) + "%)\n" + "R$ " +
                   locale.toString(value * (1 - (desconto / 100)), 'f', 2);
      return;
    }

    paramValue = "R$ " + locale.toString(value, 'f', 2);
    return;
  }

  if (paramName == "Quant.") {
    paramValue = modelItem.data(recNo, "quant").toString();
    return;
  }

  if (paramName == "Unid.") {
    paramValue = modelItem.data(recNo, "un").toString();
    return;
  }

  if (paramName == "TotalProd") {
    double parcial = modelItem.data(recNo, "parcialDesc").toDouble();
    paramValue = "R$ " + locale.toString(parcial, 'f', 2);
    return;
  }

  // REPORT SUMMARY
  if (paramName == "Soma") {
    double value = ui->doubleSpinBoxSubTotalLiq->value();
    paramValue = locale.toString(value, 'f', 2);
    return;
  }

  if (paramName == "Desconto") {
    double value = ui->doubleSpinBoxDescontoGlobal->value();
    paramValue = locale.toString(value, 'f', 2);
    return;
  }

  if (paramName == "Total") {
    double value = ui->doubleSpinBoxSubTotalLiq->value() -
                   (ui->doubleSpinBoxSubTotalLiq->value() * ui->doubleSpinBoxDescontoGlobal->value() / 100);
    paramValue = locale.toString(value, 'f', 2);
    return;
  }

  if (paramName == "Frete") {
    double value = ui->doubleSpinBoxFrete->value();
    paramValue = locale.toString(value, 'f', 2);
    return;
  }

  if (paramName == "TotalFinal") {
    double value = ui->doubleSpinBoxTotal->value();
    paramValue = locale.toString(value, 'f', 2);
    return;
  }

  if (paramName == "Observacao") {
    paramValue = ui->textEditObs->toPlainText();
    return;
  }

  if (paramName == "Disclaimer") {
    paramValue =
        "1- O prazo de entrega deve ser consultado no momento da compra;\n2- Não aceitamos devolução de produtos "
        "calculados com percetual de perda, cortes/rodapés de porcelanatos ou mosaicos especiais;\n3- Produtos com "
        "classificação comercial \"C\" podem apresentar algum tipo de defeito, tendo valor especial por este motivo,  "
        "e devoluções não serão aceitas.";
    return;
  }
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
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela orcamento_has_produto: " + modelItem.lastError().text());
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

  DoubleDelegate *delegate = new DoubleDelegate(this);

  for (int col = 0; col < modelItem.columnCount(); ++col) {
    if (col != modelItem.fieldIndex("desconto")) {
      ui->tableProdutos->setItemDelegateForColumn(col, delegate);
    }
  }

  ui->tableProdutos->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableProdutos->horizontalHeader()->setResizeContentsPrecision(0);
}

void Orcamento::adicionarItem(const bool isUpdate) {
  ui->checkBoxRepresentacao->setDisabled(true);

  calcPrecoItemTotal();

  if (ui->itemBoxProduto->text().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Item inválido!");
    return;
  }

  if (ui->doubleSpinBoxQte->value() == 0) {
    QMessageBox::critical(this, "Erro!", "Quantidade inválida!");
    return;
  }

  const int row = (isUpdate) ? mapperItem.currentIndex() : modelItem.rowCount();

  if (not isUpdate) {
    modelItem.insertRow(row);
  }

  // TODO: put second model on registerDialog for tables with auxiliary tables?
  // TODO: check setData's (AND them all in a bool?)
  modelItem.setData(row, "idOrcamento", ui->lineEditOrcamento->text());
  modelItem.setData(row, "idLoja", UserSession::getLoja());
  modelItem.setData(row, "idProduto", ui->itemBoxProduto->value().toInt());
  modelItem.setData(row, "item", row);
  modelItem.setData(row, "fornecedor", ui->lineEditFornecedor->text());
  modelItem.setData(row, "produto", ui->itemBoxProduto->text());
  modelItem.setData(row, "obs", ui->lineEditObs->text());
  modelItem.setData(row, "prcUnitario", ui->lineEditPrecoUn->getValue());
  modelItem.setData(row, "caixas", ui->spinBoxCaixas->value());
  modelItem.setData(row, "quant", ui->doubleSpinBoxQte->value());
  modelItem.setData(row, "unCaixa", ui->doubleSpinBoxQte->singleStep());
  modelItem.setData(row, "un", ui->lineEditUn->text());
  modelItem.setData(row, "codComercial", ui->lineEditCodComercial->text());
  modelItem.setData(row, "formComercial", ui->lineEditFormComercial->text());
  modelItem.setData(row, "desconto", ui->doubleSpinBoxDesconto->value());

  calcPrecoGlobalTotal();

  modelItem.setData(row, "parcialDesc", ui->doubleSpinBoxPrecoTotal->value());

  novoItem();
}

void Orcamento::on_pushButtonAdicionarItem_clicked() { adicionarItem(); }

void Orcamento::on_pushButtonAtualizarItem_clicked() {
  adicionarItem(true);
  ui->tableProdutos->clearSelection();
}

void Orcamento::on_pushButtonGerarVenda_clicked() {
  silent = true;

  bool ok = ui->lineEditOrcamento->text() == "Auto gerado" ? save() : update();

  if (not ok) {
    return;
  }

  const QDateTime time = ui->dateTimeEdit->dateTime();

  if (not time.isValid()) {
    return;
  }

  if (time.addDays(data("validade").toInt()).date() < QDateTime::currentDateTime().date()) {
    QMessageBox::critical(this, "Erro!", "Orçamento vencido!");
    return;
  }

  if (ui->itemBoxEndereco->text().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Deve selecionar endereço!");
    ui->itemBoxEndereco->setFocus();
    return;
  }

  if (model.data(mapper.currentIndex(), "status").toString() == "CANCELADO") {
    QMessageBox::critical(this, "Erro!", "Orçamento cancelado!");
    // TODO: ask user if he wants to replicate orcamento
    return;
  }

  // TODO: if status expirado ask user if he wants to replicate

  const int idCliente = ui->itemBoxCliente->value().toInt();

  QSqlQuery queryCliente;
  queryCliente.prepare("SELECT * FROM cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", idCliente);

  if (not queryCliente.exec() or not queryCliente.first()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro verificando se cliente possui CPF/CNPJ: " + queryCliente.lastError().text());
    return;
  }

  if (queryCliente.value("cpf").toString().isEmpty() and queryCliente.value("cnpj").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Cliente não possui CPF/CNPJ cadastrado!");
    CadastroCliente *cadCliente = new CadastroCliente(this);
    cadCliente->viewRegisterById(idCliente);
    cadCliente->show();
    return;
  }

  QSqlQuery queryCadastro;
  queryCadastro.prepare("SELECT * FROM cliente_has_endereco WHERE idCliente = :idCliente");
  queryCadastro.bindValue(":idCliente", idCliente);

  if (not queryCadastro.exec()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro verificando se cliente possui endereço: " + queryCadastro.lastError().text());
    return;
  }

  if (not queryCadastro.first()) {
    QMessageBox::critical(this, "Erro!", "Cliente não possui endereço cadastrado!");
    CadastroCliente *cadCliente = new CadastroCliente(this);
    cadCliente->viewRegisterById(idCliente);
    cadCliente->show();
    return;
  }

  queryCadastro.prepare("SELECT incompleto FROM orcamento LEFT JOIN cliente ON orcamento.idCliente = cliente.idCliente "
                        "WHERE cliente.idCliente = :idCliente AND incompleto = TRUE");
  queryCadastro.bindValue(":idCliente", idCliente);

  if (not queryCadastro.exec()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro verificando se cadastro do cliente está completo: " + queryCadastro.lastError().text());
    return;
  }

  if (queryCadastro.first()) {
    QMessageBox::critical(this, "Erro!", "Cadastro incompleto, deve terminar!");
    CadastroCliente *cadCliente = new CadastroCliente(this);
    cadCliente->viewRegisterById(idCliente);
    cadCliente->show();
    return;
  }

  if (ui->itemBoxEndereco->text().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Deve escolher um endereço!");
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
    ui->doubleSpinBoxQte->setSingleStep(1.);
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
    QMessageBox::critical(this, "Erro!", "Erro na busca do produto: " + query.lastError().text());
    return;
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

  ui->doubleSpinBoxQte->setSingleStep(
        query.value((un.contains("M2") or un.contains("M²") or un.contains("ML")) ? "m2cx" : "pccx").toDouble());

  ui->doubleSpinBoxQte->setValue(0);
}

void Orcamento::on_itemBoxCliente_textChanged(const QString &text) {
  Q_UNUSED(text);

  ui->itemBoxEndereco->searchDialog()->setFilter("idCliente = " + QString::number(ui->itemBoxCliente->value().toInt()) +
                                                 " AND desativado = FALSE OR idEndereco = 1");

  QSqlQuery queryCliente;
  queryCliente.prepare("SELECT idProfissionalRel FROM cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", ui->itemBoxCliente->value());

  if (not queryCliente.exec() or not queryCliente.first()) {
    QMessageBox::critical(this, "Erro!", "Erro ao buscar cliente: " + queryCliente.lastError().text());
    return;
  }

  ui->itemBoxProfissional->setValue(queryCliente.value("idProfissionalRel"));
  ui->itemBoxEndereco->setEnabled(true);
  ui->itemBoxEndereco->clear();
}

void Orcamento::successMessage() { QMessageBox::information(this, "Atenção!", "Orçamento atualizado com sucesso!"); }

void Orcamento::on_pushButtonLimparSelecao_clicked() { novoItem(); }

void Orcamento::on_checkBoxFreteManual_clicked(const bool checked) {
  ui->doubleSpinBoxFrete->setFrame(checked);
  ui->doubleSpinBoxFrete->setReadOnly(not checked);
  ui->doubleSpinBoxFrete->setButtonSymbols(checked ? QDoubleSpinBox::UpDownArrows : QDoubleSpinBox::NoButtons);

  calcPrecoGlobalTotal();
}

void Orcamento::on_pushButtonReplicar_clicked() {
  // TODO: setar parametros de representacao
  Orcamento *replica = new Orcamento(parentWidget());
  replica->ui->pushButtonReplicar->hide();

  int row = mapper.currentIndex();

  replica->ui->itemBoxCliente->setValue(model.data(row, "idCliente"));
  replica->ui->itemBoxProfissional->setValue(model.data(row, "idProfissional"));
  replica->ui->itemBoxVendedor->setValue(model.data(row, "idUsuario"));
  replica->ui->itemBoxEndereco->setValue(model.data(row, "idEnderecoEntrega"));
  replica->ui->spinBoxValidade->setValue(model.data(row, "validade").toInt());
  replica->ui->doubleSpinBoxDescontoGlobal->setValue(model.data(row, "descontoPorc").toDouble());
  replica->ui->doubleSpinBoxFrete->setValue(model.data(row, "frete").toDouble());
  replica->ui->doubleSpinBoxTotal->setValue(model.data(row, "total").toDouble());
  replica->ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
  replica->ui->checkBoxRepresentacao->setChecked(ui->checkBoxRepresentacao->isChecked());

  for (int i = 0; i < modelItem.rowCount(); ++i) {
    replica->ui->itemBoxProduto->setValue(modelItem.data(i, "idProduto"));
    replica->ui->doubleSpinBoxQte->setValue(modelItem.data(i, "quant").toDouble());
    replica->adicionarItem();
  }

  replica->show();
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

bool Orcamento::save(const bool isUpdate) {
  if (not verifyFields()) {
    return false;
  }

  QSqlQuery("SET SESSION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  row = (isUpdate) ? mapper.currentIndex() : model.rowCount();

  if (row == -1) {
    QMessageBox::critical(this, "Erro!", "Erro linha - 1");
    QSqlQuery("ROLLBACK").exec();
    return false;
  }

  if (not isUpdate) {
    model.insertRow(row);
  }

  if (not savingProcedures()) {
    errorMessage();
    QSqlQuery("ROLLBACK").exec();
    return false;
  }

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro ao cadastrar: " + model.lastError().text());
    QSqlQuery("ROLLBACK").exec();
    return false;
  }

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    modelItem.setData(row, primaryKey, ui->lineEditOrcamento->text());
    modelItem.setData(row, "idLoja", UserSession::getLoja());
  }

  if (not modelItem.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro ao adicionar um item ao orçamento: " + modelItem.lastError().text());
    QSqlQuery("ROLLBACK").exec();
    return false;
  }

  QSqlQuery("COMMIT").exec();
  isDirty = false;
  isOk = true;

  QSqlQuery queryOrc;
  queryOrc.prepare("SELECT * FROM view_orcamento WHERE Vendedor = :Vendedor AND Cliente = :Cliente AND Total = :Total "
                   "AND `Data de emissão` LIKE :Data");
  queryOrc.bindValue(":Vendedor", ui->itemBoxVendedor->text());
  queryOrc.bindValue(":Cliente", ui->itemBoxCliente->text());
  queryOrc.bindValue(":Total", ui->doubleSpinBoxTotal->value());
  queryOrc.bindValue(":Data", ui->dateTimeEdit->dateTime().toString("yyyy-MM-dd hh:mm%"));

  if (not queryOrc.exec() or not queryOrc.first()) {
    QMessageBox::critical(this, "Erro!", "Não encontrou orçamento!");
    return false;
  }

  viewRegisterById(queryOrc.value(0));

  sendUpdateMessage();

  if (not silent) {
    successMessage();
  }

  return true;
}

void Orcamento::on_pushButtonGerarExcel_clicked() {
  // TODO: endereco ficando como "-;"
  if (settings("User/userFolder").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Não há uma pasta definida para salvar PDF/Excel. Por favor escolha uma.");
    setSettings("User/userFolder", QFileDialog::getExistingDirectory(this, "Pasta PDF/Excel"));
    return;
  }

  // TODO: add this elsewhere
  if (settings("User/userFolder").toString().isEmpty()) {
    return;
  }

  QString path = settings("User/userFolder").toString();

  QDir dir(path);

  if (not dir.exists()) {
    dir.mkdir(path);
  }

  QFile modelo(QDir::currentPath() + "/modelo.xlsx");

  if (not modelo.exists()) {
    QMessageBox::critical(this, "Erro!", "Não encontrou o modelo do Excel!");
    return;
  }

  if (modelItem.rowCount() > 17) {
    QMessageBox::critical(this, "Erro!", "Mais itens do que cabe no modelo!");
    return;
  }

  QXlsx::Document xlsx("modelo.xlsx");

  QString idOrcamento = model.data(mapper.currentIndex(), "idOrcamento").toString();

  QSqlQuery queryOrc;
  queryOrc.prepare("SELECT * FROM orcamento WHERE idOrcamento = :idOrcamento");
  queryOrc.bindValue(":idOrcamento", idOrcamento);

  if (not queryOrc.exec() or not queryOrc.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados do orçamento: " + queryOrc.lastError().text());
    return;
  }

  QSqlQuery queryLoja;
  queryLoja.prepare(
        "SELECT * FROM loja WHERE idLoja = (SELECT idLoja FROM orcamento WHERE idOrcamento = :idOrcamento)");
  queryLoja.bindValue(":idOrcamento", idOrcamento);

  if (not queryLoja.exec() or not queryLoja.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados da loja: " + queryLoja.lastError().text());
    return;
  }

  QSqlQuery queryUsuario;
  queryUsuario.prepare("SELECT * FROM usuario WHERE idUsuario = :idUsuario");
  queryUsuario.bindValue(":idUsuario", ui->itemBoxVendedor->value());

  if (not queryUsuario.exec() or not queryUsuario.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados do usuário: " + queryUsuario.lastError().text());
    return;
  }

  QSqlQuery queryCliente;
  queryCliente.prepare("SELECT * FROM cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", ui->itemBoxCliente->value());

  if (not queryCliente.exec() or not queryCliente.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados do cliente: " + queryCliente.lastError().text());
    return;
  }

  QSqlQuery queryEndEnt;
  queryEndEnt.prepare("SELECT * FROM cliente_has_endereco WHERE idEndereco = :idEndereco");
  queryEndEnt.bindValue(":idEndereco", ui->itemBoxEndereco->value());

  if (not queryEndEnt.exec() or not queryEndEnt.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados do endereço entrega: " + queryEndEnt.lastError().text());
    return;
  }

  QSqlQuery queryEndFat;
  queryEndFat.prepare("SELECT * FROM cliente_has_endereco WHERE idEndereco = :idEndereco");
  queryEndFat.bindValue(":idEndereco", ui->itemBoxEndereco->value());

  if (not queryEndFat.exec() or not queryEndFat.first()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro buscando dados do endereço faturamento: " + queryEndFat.lastError().text());
    return;
  }

  QSqlQuery queryProf;
  queryProf.prepare("SELECT * FROM profissional WHERE idProfissional = :idProfissional");
  queryProf.bindValue(":idProfissional", ui->itemBoxProfissional->value());

  if (not queryProf.exec() or not queryProf.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados do profissional: " + queryProf.lastError().text());
    return;
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
  xlsx.write("M3", queryCliente.value(queryCliente.value("pfpj").toString() == "PF" ? "cpf" : "cnpj").toString());
  xlsx.write("M4", queryCliente.value("tel").toString());
  xlsx.write("M5", queryEndFat.value("cep").toString());
  xlsx.write("M6", queryEndEnt.value("cep").toString());
  xlsx.write("H7", queryProf.value("tel").toString());
  xlsx.write("K7", queryProf.value("email").toString());

  xlsx.write("N29", "R$ " + QString::number(queryOrc.value("subTotalLiq").toDouble(), 'f', 2)); // soma
  xlsx.write("N30", QString::number(queryOrc.value("descontoPorc").toDouble(), 'f', 2) + "%");  // desconto
  xlsx.write("N31", "R$ " + QString::number(queryOrc.value("subTotalLiq").toDouble() -
                                            (ui->doubleSpinBoxDescontoGlobal->value() / 100 *
                                             queryOrc.value("subTotalLiq").toDouble()),
                                            'f', 2));                                     // total
  xlsx.write("N32", "R$ " + QString::number(queryOrc.value("frete").toDouble(), 'f', 2)); // frete
  xlsx.write("N33", "R$ " + QString::number(queryOrc.value("total").toDouble(), 'f', 2)); // total final

  for (int i = 0; i < modelItem.rowCount(); ++i) {
    xlsx.write("A" + QString::number(12 + i), modelItem.data(i, "fornecedor").toString());
    xlsx.write("B" + QString::number(12 + i), modelItem.data(i, "codComercial").toString());
    xlsx.write("C" + QString::number(12 + i), modelItem.data(i, "produto").toString());
    xlsx.write("H" + QString::number(12 + i), modelItem.data(i, "obs").toString());
    xlsx.write("K" + QString::number(12 + i), modelItem.data(i, "prcUnitario").toDouble());
    xlsx.write("L" + QString::number(12 + i), modelItem.data(i, "quant").toDouble());
    xlsx.write("M" + QString::number(12 + i), modelItem.data(i, "un").toString());
    xlsx.write("N" + QString::number(12 + i), "R$ " + QString::number(modelItem.data(i, "total").toDouble(), 'f', 2));
  }

  if (not xlsx.saveAs(path + "/" + ui->lineEditOrcamento->text() + ".xlsx")) {
    QMessageBox::critical(this, "Erro!", "Ocorreu algum erro ao salvar o arquivo.");
    return;
  }

  QMessageBox::information(this, "Ok!", "Arquivo salvo como " + path + "/" + ui->lineEditOrcamento->text() + ".xlsx");
  QDesktopServices::openUrl(QUrl::fromLocalFile(path + "/" + ui->lineEditOrcamento->text() + ".xlsx"));
}

void Orcamento::on_checkBoxRepresentacao_toggled(bool checked) {
  ui->itemBoxProduto->searchDialog()->setRepresentacao(" AND representacao = " + QString(checked ? "TRUE" : "FALSE"));
}

QVariant Orcamento::settings(QString key) const { return UserSession::getSettings(key); }

void Orcamento::setSettings(QString key, QVariant value) const { UserSession::setSettings(key, value); }

// TODO: mudar status do orcamento para expirado se validade vencida
// TODO: cadastrar endereco nao aparece (apenas apos fechar e abrir de novo) dar um refresh no model
