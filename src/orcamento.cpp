#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlRecord>
#include <cmath>

#include "baixaorcamento.h"
#include "cadastrocliente.h"
#include "doubledelegate.h"
#include "excel.h"
#include "impressao.h"
#include "orcamento.h"
#include "orcamentoproxymodel.h"
#include "porcentagemdelegate.h"
#include "reaisdelegate.h"
#include "searchdialogproxy.h"
#include "ui_orcamento.h"
#include "usersession.h"
#include "venda.h"

Orcamento::Orcamento(QWidget *parent) : RegisterDialog("orcamento", "idOrcamento", parent), ui(new Ui::Orcamento) {
  ui->setupUi(this);

  setAttribute(Qt::WA_DeleteOnClose);

  proxy = new SearchDialogProxy(&modelItem, this);

  setupTables();

  //  for (auto const *line : findChildren<QLineEdit *>()) {
  //    connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);
  //  }

  ui->itemBoxCliente->setSearchDialog(SearchDialog::cliente(this));
  ui->itemBoxCliente->setRegisterDialog(new CadastroCliente(this));
  ui->itemBoxProduto->setSearchDialog(SearchDialog::produto(this));
  ui->itemBoxVendedor->setSearchDialog(SearchDialog::vendedor(this));
  ui->itemBoxVendedorIndicou->setSearchDialog(SearchDialog::vendedor(this));
  ui->itemBoxProfissional->setSearchDialog(SearchDialog::profissional(this));
  ui->itemBoxEndereco->setSearchDialog(SearchDialog::enderecoCliente(this));

  setupMapper();
  newRegister();

  if (UserSession::tipoUsuario() == "VENDEDOR ESPECIAL") {
    QSqlQuery query("SELECT descricao, idLoja FROM loja");

    while (query.next()) ui->comboBoxLoja->addItem(query.value("descricao").toString(), query.value("idLoja"));

    ui->comboBoxLoja->setCurrentValue(UserSession::idLoja());
  } else {
    ui->labelLoja->hide();
    ui->comboBoxLoja->hide();
    ui->labelVendedorIndicou->hide();
    ui->itemBoxVendedorIndicou->hide();
  }

  if (UserSession::tipoUsuario() == "ADMINISTRADOR") {
    ui->dateTimeEdit->setReadOnly(false);
    ui->dateTimeEdit->setCalendarPopup(true);
    ui->checkBoxFreteManual->show();
  }

  on_checkBoxRepresentacao_toggled(false);

  ui->labelBaixa->hide();
  ui->plainTextEditBaixa->hide();
  ui->spinBoxEstoquePromocao->hide();
}

Orcamento::~Orcamento() { delete ui; }

void Orcamento::show() {
  QDialog::show();
  ui->tableProdutos->resizeColumnsToContents();
}

void Orcamento::on_tableProdutos_clicked(const QModelIndex &index) {
  if (isReadOnly) return;

  ui->pushButtonAtualizarItem->show();
  ui->pushButtonAdicionarItem->hide();
  ui->pushButtonRemoverItem->show();
  mapperItem.setCurrentModelIndex(index);
  ui->tableProdutos->selectRow(index.row());
}

bool Orcamento::viewRegister() {
  modelItem.setFilter("idOrcamento = '" + data(0, "idOrcamento").toString() + "'");

  if (not modelItem.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela orcamento_has_produto: " + modelItem.lastError().text());
    return false;
  }

  if (not RegisterDialog::viewRegister()) return false;

  novoItem();

  const QString status = data("status").toString();

  if (status == "PERDIDO" or status == "CANCELADO") {
    ui->labelBaixa->show();
    ui->plainTextEditBaixa->show();
  }

  if (status == "ATIVO") ui->pushButtonReplicar->hide();

  if (ui->dateTimeEdit->dateTime().addDays(data("validade").toInt()).date() < QDateTime::currentDateTime().date() or
      status != "ATIVO") {
    isReadOnly = true;

    ui->pushButtonGerarVenda->hide();
    ui->pushButtonAtualizarOrcamento->hide();
    ui->pushButtonReplicar->show();

    ui->pushButtonAdicionarItem->hide();
    ui->pushButtonAtualizarItem->hide();
    ui->pushButtonRemoverItem->hide();
    ui->pushButtonLimparSelecao->hide();

    ui->itemBoxCliente->setReadOnlyItemBox(true);
    ui->itemBoxEndereco->setReadOnlyItemBox(true);
    ui->itemBoxProduto->setReadOnlyItemBox(true);
    ui->itemBoxProfissional->setReadOnlyItemBox(true);
    ui->itemBoxVendedor->setReadOnlyItemBox(true);

    ui->spinBoxPrazoEntrega->setReadOnly(true);

    ui->doubleSpinBoxDesconto->setReadOnly(true);
    ui->doubleSpinBoxDescontoGlobal->setReadOnly(true);
    ui->doubleSpinBoxDescontoGlobalReais->setReadOnly(true);
    ui->doubleSpinBoxFrete->setReadOnly(true);
    ui->doubleSpinBoxTotalItem->setReadOnly(true);
    ui->doubleSpinBoxQuant->setReadOnly(true);
    ui->doubleSpinBoxSubTotalBruto->setReadOnly(true);
    ui->doubleSpinBoxSubTotalLiq->setReadOnly(true);
    ui->doubleSpinBoxTotal->setReadOnly(true);

    ui->lineEditCodComercial->hide();
    ui->lineEditEstoque->hide();
    ui->lineEditFormComercial->hide();
    ui->lineEditFornecedor->hide();
    ui->spinBoxMinimo->hide();
    ui->lineEditObs->hide();
    ui->lineEditPrecoUn->hide();
    ui->lineEditUn->hide();
    ui->itemBoxProduto->hide();
    ui->doubleSpinBoxQuant->hide();
    ui->doubleSpinBoxCaixas->hide();
    ui->labelCaixa->hide();
    ui->spinBoxUnCx->hide();
    ui->doubleSpinBoxDesconto->hide();
    ui->doubleSpinBoxTotalItem->hide();
    ui->labelCaixas->hide();
    ui->labelCodComercial->hide();
    ui->labelProduto->hide();
    ui->labelDesconto->hide();
    ui->labelTotalItem->hide();
    ui->labelUn->hide();
    ui->labelPrecoUn->hide();
    ui->labelFornecedor->hide();
    ui->labelQuant->hide();
    ui->labelEstoque->hide();
    ui->labelFormComercial->hide();
    ui->labelMinimo->hide();
    ui->labelObs->hide();

    ui->plainTextEditObs->setReadOnly(true);

    ui->checkBoxFreteManual->setDisabled(true);
  } else {
    ui->pushButtonGerarVenda->show();
  }

  ui->lineEditReplicaDe->setReadOnly(true);
  ui->lineEditReplicadoEm->setReadOnly(true);

  ui->tableProdutos->resizeColumnsToContents();

  ui->checkBoxRepresentacao->setDisabled(true);

  ui->plainTextEditObs->setPlainText(data("observacao").toString());

  ui->doubleSpinBoxFrete->setValue(data("frete").toDouble());

  return true;
}

void Orcamento::novoItem() {
  ui->pushButtonAdicionarItem->show();
  ui->pushButtonAtualizarItem->hide();
  ui->pushButtonRemoverItem->hide();
  ui->itemBoxProduto->clear();
  ui->tableProdutos->clearSelection();
  ui->tableProdutos->resizeColumnsToContents();
}

void Orcamento::setupMapper() {
  addMapping(ui->checkBoxRepresentacao, "representacao");
  addMapping(ui->dateTimeEdit, "data");
  addMapping(ui->doubleSpinBoxDescontoGlobal, "descontoPorc");
  addMapping(ui->doubleSpinBoxFrete, "frete");
  addMapping(ui->doubleSpinBoxSubTotalBruto, "subTotalBru");
  addMapping(ui->doubleSpinBoxSubTotalLiq, "subTotalLiq");
  addMapping(ui->doubleSpinBoxTotal, "total");
  addMapping(ui->itemBoxCliente, "idCliente", "value");
  addMapping(ui->itemBoxEndereco, "idEnderecoEntrega", "value");
  addMapping(ui->itemBoxProfissional, "idProfissional", "value");
  addMapping(ui->itemBoxVendedor, "idUsuario", "value");
  addMapping(ui->itemBoxVendedorIndicou, "idUsuarioIndicou", "value");
  addMapping(ui->lineEditOrcamento, "idOrcamento");
  addMapping(ui->lineEditReplicaDe, "replicadoDe");
  addMapping(ui->lineEditReplicadoEm, "replicadoEm");
  addMapping(ui->spinBoxPrazoEntrega, "prazoEntrega");
  addMapping(ui->spinBoxValidade, "validade");
  addMapping(ui->plainTextEditObs, "observacao");
  addMapping(ui->plainTextEditBaixa, "observacaoCancelamento");

  mapperItem.setModel(proxy);
  mapperItem.setSubmitPolicy(QDataWidgetMapper::ManualSubmit);

  mapperItem.addMapping(ui->itemBoxProduto, modelItem.fieldIndex("idProduto"), "value");
  mapperItem.addMapping(ui->lineEditCodComercial, modelItem.fieldIndex("codComercial"));
  mapperItem.addMapping(ui->lineEditFormComercial, modelItem.fieldIndex("formComercial"));
  mapperItem.addMapping(ui->lineEditObs, modelItem.fieldIndex("obs"), "text");
  mapperItem.addMapping(ui->lineEditPrecoUn, modelItem.fieldIndex("prcUnitario"), "value");
  mapperItem.addMapping(ui->lineEditUn, modelItem.fieldIndex("un"), "text");
  mapperItem.addMapping(ui->doubleSpinBoxQuant, modelItem.fieldIndex("quant"), "value");
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
  if (not RegisterDialog::newRegister()) return false;

  ui->lineEditOrcamento->setText("Auto gerado");
  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
  ui->spinBoxValidade->setValue(7);
  novoItem();

  return true;
}

void Orcamento::removeItem() {
  if (not modelItem.removeRow(ui->tableProdutos->currentIndex().row())) {
    QMessageBox::critical(this, "Erro!", "Erro removendo linha: " + modelItem.lastError().text());
    return;
  }

  if (ui->lineEditOrcamento->text() != "Auto gerado") {
    if (not modelItem.submitAll()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando remoção: " + modelItem.lastError().text());
      return;
    }

    if (not calcPrecoGlobalTotal()) return;

    update();
  }

  if (modelItem.rowCount() == 0) {
    if (ui->lineEditOrcamento->text() == "Auto gerado") ui->checkBoxRepresentacao->setEnabled(true);
    ui->itemBoxProduto->searchDialog()->setFornecedorRep("");
  }

  novoItem();
}

bool Orcamento::generateId() {
  QString id = UserSession::fromLoja("sigla", UserSession::tipoUsuario() == "VENDEDOR ESPECIAL" and
                                                      not ui->itemBoxVendedorIndicou->text().isEmpty()
                                                  ? ui->itemBoxVendedorIndicou->text()
                                                  : ui->itemBoxVendedor->text()) +
               "-" + QDate::currentDate().toString("yy");

  QSqlQuery query;
  query.prepare("SELECT MAX(idOrcamento) AS idOrcamento FROM orcamento WHERE idOrcamento LIKE :id");
  query.bindValue(":id", id + "%");

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando próximo id disponível: " + query.lastError().text());
    return false;
  }

  const int last = query.first() ? query.value("idOrcamento").toString().remove(id).left(4).toInt() : 0;

  id += QString("%1").arg(last + 1, 4, 10, QChar('0'));
  id += ui->checkBoxRepresentacao->isChecked() ? "R" : "";
  id += "O";

  if (id.size() != 12) {
    QMessageBox::critical(this, "Erro!", "Ocorreu algum erro ao gerar id: " + id);
    return false;
  }

  const QString replica = ui->lineEditReplicaDe->text();

  if (not replica.isEmpty()) {
    query.prepare(
        "SELECT COALESCE(MAX(RIGHT(idOrcamento, 1)) + 1, 1) AS revisao FROM orcamento WHERE LENGTH(idOrcamento) = 17 "
        "AND idOrcamento LIKE :idOrcamento");
    query.bindValue(":idOrcamento", replica.left(16) + "%");

    if (not query.exec() or not query.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando próxima revisão disponível: " + query.lastError().text());
      return false;
    }

    id = replica.left(replica.indexOf("-Rev")) + "-Rev" + query.value("revisao").toString();
  }

  ui->lineEditOrcamento->setText(id);

  return true;
}

bool Orcamento::verifyFields() {
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
  if (not setData("data", ui->dateTimeEdit->dateTime())) return false;
  if (not setData("descontoPorc", ui->doubleSpinBoxDescontoGlobal->value())) return false;
  const double desconto = ui->doubleSpinBoxSubTotalLiq->value() * ui->doubleSpinBoxDescontoGlobal->value() / 100.;
  if (not setData("descontoReais", desconto)) return false;
  if (not setData("frete", ui->doubleSpinBoxFrete->value())) return false;
  if (not setData("idCliente", ui->itemBoxCliente->value())) return false;
  if (not setData("idEnderecoEntrega", ui->itemBoxEndereco->value())) return false;
  if (not setData("idLoja", UserSession::fromLoja("usuario.idLoja", ui->itemBoxVendedor->text()))) return false;
  if (not setData("idOrcamento", ui->lineEditOrcamento->text())) return false;
  if (not setData("idProfissional", ui->itemBoxProfissional->value())) return false;
  if (not setData("idUsuario", ui->itemBoxVendedor->value())) return false;
  if (not setData("idUsuarioIndicou", ui->itemBoxVendedorIndicou->value())) return false;
  if (not setData("observacao", ui->plainTextEditObs->toPlainText())) return false;
  if (not setData("prazoEntrega", ui->spinBoxPrazoEntrega->value())) return false;
  if (not setData("replicadoDe", ui->lineEditReplicaDe->text())) return false;
  if (not setData("representacao", ui->checkBoxRepresentacao->isChecked())) return false;
  if (not setData("subTotalBru", ui->doubleSpinBoxSubTotalBruto->value())) return false;
  if (not setData("subTotalLiq", ui->doubleSpinBoxSubTotalLiq->value())) return false;
  if (not setData("total", ui->doubleSpinBoxTotal->value())) return false;
  if (not setData("validade", ui->spinBoxValidade->value())) return false;
  if (not setData("freteManual", ui->checkBoxFreteManual->isChecked())) return false;

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    if (not modelItem.setData(row, "idOrcamento", ui->lineEditOrcamento->text())) return false;
    if (not modelItem.setData(row, "idLoja", model.data(this->row, "idLoja"))) return false;
    const double prcUnitario = modelItem.data(row, "prcUnitario").toDouble();
    const double desconto = modelItem.data(row, "desconto").toDouble() / 100.;
    if (not modelItem.setData(row, "descUnitario", prcUnitario - (prcUnitario * desconto))) return false;
  }

  if (not atualizaReplica()) return false;

  return true;
}

bool Orcamento::atualizaReplica() {
  if (not ui->lineEditReplicaDe->text().isEmpty()) {
    QSqlQuery query;
    query.prepare("UPDATE orcamento SET status = 'REPLICADO', replicadoEm = "
                  ":idReplica WHERE idOrcamento = :idOrcamento");
    query.bindValue(":idReplica", ui->lineEditOrcamento->text());
    query.bindValue(":idOrcamento", ui->lineEditReplicaDe->text());

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando replicadoEm: " + query.lastError().text());
      return false;
    }
  }

  return true;
}

void Orcamento::clearFields() {
  RegisterDialog::clearFields();

  if (UserSession::tipoUsuario() == "VENDEDOR" or UserSession::tipoUsuario() == "VENDEDOR ESPECIAL") {
    ui->itemBoxVendedor->setValue(UserSession::idUsuario());
  }

  ui->itemBoxEndereco->setEnabled(false);
}

void Orcamento::on_pushButtonRemoverItem_clicked() { removeItem(); }

void Orcamento::calcPrecoItemTotal() {
  if (ui->itemBoxProduto->text().isEmpty() or isBlockedTotalItem) return;

  const double quant = ui->doubleSpinBoxQuant->value();
  const double prcUn = ui->lineEditPrecoUn->getValue();
  const double desc = ui->doubleSpinBoxDesconto->value() / 100.;
  const double itemBruto = quant * prcUn;
  const double subTotalItem = itemBruto * (1. - desc);

  ui->doubleSpinBoxTotalItem->setValue(subTotalItem);
}

void Orcamento::on_doubleSpinBoxQuant_valueChanged(const double) {
  const double caixas = ui->doubleSpinBoxQuant->value() / ui->spinBoxUnCx->value();

  if (ui->doubleSpinBoxCaixas->value() != caixas) ui->doubleSpinBoxCaixas->setValue(caixas);
}

void Orcamento::on_doubleSpinBoxQuant_editingFinished() {
  ui->doubleSpinBoxQuant->setValue(ui->doubleSpinBoxCaixas->value() * ui->doubleSpinBoxQuant->singleStep());
}

void Orcamento::on_pushButtonCadastrarOrcamento_clicked() { save(); }

void Orcamento::on_pushButtonAtualizarOrcamento_clicked() { update(); }

bool Orcamento::calcPrecoGlobalTotal() {
  double subTotalItens = 0.;
  double subTotalBruto = 0.;

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    const double itemBruto = modelItem.data(row, "quant").toDouble() * modelItem.data(row, "prcUnitario").toDouble();
    const double descItem = modelItem.data(row, "desconto").toDouble() / 100.;
    const double stItem = itemBruto * (1. - descItem);
    subTotalBruto += itemBruto;
    subTotalItens += stItem;
    if (not modelItem.setData(row, "parcial", itemBruto)) return false;
    if (not modelItem.setData(row, "parcialDesc", stItem)) return false;
  }

  const double descGlobal = ui->doubleSpinBoxDescontoGlobal->value() / 100.;
  const double subTotal = subTotalItens * (1. - descGlobal);

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    const double parcialDesc = modelItem.data(row, "parcialDesc").toDouble();

    if (not modelItem.setData(row, "descGlobal", descGlobal * 100.)) return false;
    if (not modelItem.setData(row, "total", parcialDesc * (1. - descGlobal))) return false;
  }

  ui->doubleSpinBoxSubTotalBruto->setValue(subTotalBruto);
  ui->doubleSpinBoxSubTotalLiq->setValue(subTotalItens);
  if (not isBlockedReais) ui->doubleSpinBoxDescontoGlobalReais->setValue(subTotalItens - subTotal);
  ui->doubleSpinBoxDescontoGlobalReais->setMaximum(ui->doubleSpinBoxSubTotalLiq->value());
  if (not isBlockedTotal) ui->doubleSpinBoxTotal->setValue(subTotal + ui->doubleSpinBoxFrete->value());
  ui->doubleSpinBoxTotal->setMinimum(ui->doubleSpinBoxFrete->value());

  return true;
}

void Orcamento::on_pushButtonImprimir_clicked() {
  Impressao impressao(data("idOrcamento").toString());
  impressao.print();
}

void Orcamento::setupTables() {
  modelItem.setTable("orcamento_has_produto");
  modelItem.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelItem.setHeaderData("produto", "Produto");
  modelItem.setHeaderData("fornecedor", "Fornecedor");
  modelItem.setHeaderData("obs", "Obs.");
  modelItem.setHeaderData("prcUnitario", "Preço/Un.");
  modelItem.setHeaderData("caixas", "Caixas");
  modelItem.setHeaderData("quant", "Quant.");
  modelItem.setHeaderData("un", "Un.");
  modelItem.setHeaderData("codComercial", "Código");
  modelItem.setHeaderData("formComercial", "Formato");
  modelItem.setHeaderData("unCaixa", "Un./Caixa");
  modelItem.setHeaderData("parcial", "Subtotal");
  modelItem.setHeaderData("desconto", "Desc. %");
  modelItem.setHeaderData("parcialDesc", "Total");

  modelItem.setFilter("0");

  if (not modelItem.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela orcamento_has_produto: " + modelItem.lastError().text());
    return;
  }

  ui->tableProdutos->setModel(proxy);
  ui->tableProdutos->hideColumn("idOrcamentoProduto");
  ui->tableProdutos->hideColumn("idProduto");
  ui->tableProdutos->hideColumn("idOrcamento");
  ui->tableProdutos->hideColumn("idLoja");
  ui->tableProdutos->hideColumn("unCaixa");
  ui->tableProdutos->hideColumn("descUnitario");
  ui->tableProdutos->hideColumn("descGlobal");
  ui->tableProdutos->hideColumn("total");
  ui->tableProdutos->hideColumn("estoque_promocao");

  ui->tableProdutos->setItemDelegate(new DoubleDelegate(this));
  ui->tableProdutos->setItemDelegateForColumn(modelItem.fieldIndex("quant"), new DoubleDelegate(this, 4));
  ui->tableProdutos->setItemDelegateForColumn("prcUnitario", new ReaisDelegate(this));
  ui->tableProdutos->setItemDelegateForColumn("parcial", new ReaisDelegate(this));
  ui->tableProdutos->setItemDelegateForColumn("parcialDesc", new ReaisDelegate(this));
  ui->tableProdutos->setItemDelegateForColumn("desconto", new PorcentagemDelegate(this));
}

void Orcamento::atualizarItem() { adicionarItem(true); }

void Orcamento::adicionarItem(const bool isUpdate) {
  ui->checkBoxRepresentacao->setDisabled(true);

  calcPrecoItemTotal();

  if (ui->itemBoxProduto->text().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Item inválido!");
    return;
  }

  if (ui->doubleSpinBoxQuant->value() == 0.) {
    QMessageBox::critical(this, "Erro!", "Quantidade inválida!");
    return;
  }

  const int row = isUpdate ? mapperItem.currentIndex() : modelItem.rowCount();

  if (row == -1) {
    QMessageBox::critical(this, "Erro!", "Erro linha - 1 adicionarItem");
    return;
  }

  if (not isUpdate) modelItem.insertRow(row);

  if (not modelItem.setData(row, "idProduto", ui->itemBoxProduto->value().toInt())) return;
  if (not modelItem.setData(row, "fornecedor", ui->lineEditFornecedor->text())) return;
  if (not modelItem.setData(row, "produto", ui->itemBoxProduto->text())) return;
  if (not modelItem.setData(row, "obs", ui->lineEditObs->text())) return;
  if (not modelItem.setData(row, "prcUnitario", ui->lineEditPrecoUn->getValue())) return;
  if (not modelItem.setData(row, "caixas", ui->doubleSpinBoxCaixas->value())) return;
  if (not modelItem.setData(row, "quant", ui->doubleSpinBoxQuant->value())) return;
  if (not modelItem.setData(row, "unCaixa", ui->doubleSpinBoxQuant->singleStep())) return;
  if (not modelItem.setData(row, "un", ui->lineEditUn->text())) return;
  if (not modelItem.setData(row, "codComercial", ui->lineEditCodComercial->text())) return;
  if (not modelItem.setData(row, "formComercial", ui->lineEditFormComercial->text())) return;
  if (not modelItem.setData(row, "desconto", ui->doubleSpinBoxDesconto->value())) return;
  if (not modelItem.setData(row, "estoque_promocao", ui->spinBoxEstoquePromocao->value())) return;

  calcPrecoGlobalTotal();

  if (not modelItem.setData(row, "parcialDesc", ui->doubleSpinBoxTotalItem->value())) return;

  //

  if (ui->lineEditOrcamento->text() != "Auto gerado") {
    silent = true;
    update();
    silent = false;
  }

  //

  if (modelItem.rowCount() == 1 and ui->checkBoxRepresentacao->isChecked()) {
    ui->itemBoxProduto->searchDialog()->setFornecedorRep(modelItem.data(row, "fornecedor").toString());
  }

  //

  novoItem();
}

void Orcamento::on_pushButtonAdicionarItem_clicked() { adicionarItem(); }

void Orcamento::on_pushButtonAtualizarItem_clicked() { atualizarItem(); }

void Orcamento::on_pushButtonGerarVenda_clicked() {
  silent = true;

  if (not(ui->lineEditOrcamento->text() == "Auto gerado" ? save() : update())) return;

  silent = false;

  const QDateTime time = ui->dateTimeEdit->dateTime();

  if (not time.isValid()) return;

  if (time.addDays(data("validade").toInt()).date() < QDateTime::currentDateTime().date()) {
    QMessageBox::critical(this, "Erro!", "Orçamento vencido!");
    return;
  }

  if (ui->itemBoxEndereco->text().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Deve selecionar endereço!");
    ui->itemBoxEndereco->setFocus();
    return;
  }

  if (not verificaCadastroCliente()) return;

  Venda *venda = new Venda(parentWidget());
  venda->prepararVenda(ui->lineEditOrcamento->text());

  close();
}

void Orcamento::on_doubleSpinBoxSubTotalLiq_valueChanged(const double) { calcPrecoGlobalTotal(); }

void Orcamento::on_doubleSpinBoxCaixas_valueChanged(const double caixas) {
  const double caixas2 = fmod(caixas, ui->doubleSpinBoxCaixas->singleStep()) != 0. ? ceil(caixas) : caixas;

  const double quant = caixas2 * ui->spinBoxUnCx->value();

  if (ui->doubleSpinBoxQuant->value() != quant) ui->doubleSpinBoxQuant->setValue(quant);

  isBlockedDesconto = true;
  calcPrecoItemTotal();
  isBlockedDesconto = false;
}

void Orcamento::on_pushButtonApagarOrc_clicked() {
  BaixaOrcamento *baixa = new BaixaOrcamento(data("idOrcamento").toString(), this);
  baixa->show();
}

void Orcamento::on_itemBoxProduto_textChanged(const QString &) {
  if (ui->itemBoxProduto->text().isEmpty()) {
    ui->doubleSpinBoxCaixas->setDisabled(true);
    ui->doubleSpinBoxCaixas->setSingleStep(1.);
    ui->doubleSpinBoxCaixas->clear();
    ui->doubleSpinBoxDesconto->setDisabled(true);
    ui->doubleSpinBoxDesconto->clear();
    ui->doubleSpinBoxQuant->setDisabled(true);
    ui->doubleSpinBoxQuant->setSingleStep(1.);
    ui->doubleSpinBoxQuant->clear();
    ui->doubleSpinBoxTotalItem->clear();
    ui->doubleSpinBoxTotalItem->setDisabled(true);
    ui->lineEditCodComercial->clear();
    ui->lineEditEstoque->clear();
    ui->lineEditFormComercial->clear();
    ui->lineEditFornecedor->clear();
    ui->lineEditObs->clear();
    ui->lineEditPrecoUn->clear();
    ui->lineEditPrecoUn->setDisabled(true);
    ui->lineEditUn->clear();
    ui->lineEditUn->setDisabled(true);
    ui->spinBoxMinimo->clear();
    ui->spinBoxMinimo->setDisabled(true);
    ui->spinBoxUnCx->clear();
    ui->spinBoxUnCx->setDisabled(true);
    return;
  }

  QSqlQuery query;
  query.prepare("SELECT un, precoVenda, estoque, fornecedor, codComercial, "
                "formComercial, m2cx, pccx, minimo, multiplo, estoque_promocao "
                "FROM produto WHERE idProduto = :index");
  query.bindValue(":index", ui->itemBoxProduto->value());

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

  const QString uncx = un.contains("M2") or un.contains("M²") or un.contains("ML") ? "m2cx" : "pccx";

  ui->spinBoxUnCx->setValue(query.value(uncx).toDouble());

  ui->spinBoxMinimo->setValue(query.value("minimo").toDouble());
  ui->doubleSpinBoxQuant->setMinimum(query.value("minimo").toDouble());
  ui->doubleSpinBoxCaixas->setMinimum(query.value("minimo").toDouble() / (query.value(uncx).toDouble()));

  ui->spinBoxEstoquePromocao->setValue(query.value("estoque_promocao").toInt());

  ui->doubleSpinBoxCaixas->setEnabled(true);
  ui->doubleSpinBoxCaixas->setEnabled(true);
  ui->doubleSpinBoxDesconto->setEnabled(true);
  ui->doubleSpinBoxQuant->setEnabled(true);
  ui->doubleSpinBoxTotalItem->setEnabled(true);
  ui->lineEditPrecoUn->setEnabled(true);
  ui->lineEditUn->setEnabled(true);
  ui->spinBoxMinimo->setEnabled(true);
  ui->spinBoxUnCx->setEnabled(true);

  ui->doubleSpinBoxCaixas->setSingleStep(1.);
  ui->doubleSpinBoxQuant->setSingleStep(query.value(uncx).toDouble());

  if (query.value("multiplo").toDouble() != 0.) {
    if (query.value("minimo").toDouble() != 0.) {
      const double step = query.value("multiplo").toDouble() / query.value("minimo").toDouble();
      ui->doubleSpinBoxCaixas->setSingleStep(step);
    }

    ui->doubleSpinBoxQuant->setSingleStep(query.value("multiplo").toDouble());
  }

  ui->doubleSpinBoxQuant->setValue(0.);
  ui->doubleSpinBoxCaixas->setValue(0.);
  ui->doubleSpinBoxDesconto->setValue(0.);

  ui->tableProdutos->clearSelection();
}

void Orcamento::on_itemBoxCliente_textChanged(const QString &) {
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

void Orcamento::on_pushButtonLimparSelecao_clicked() { novoItem(); }

void Orcamento::on_checkBoxFreteManual_clicked(const bool checked) {
  ui->doubleSpinBoxFrete->setFrame(checked);
  ui->doubleSpinBoxFrete->setReadOnly(not checked);
  ui->doubleSpinBoxFrete->setButtonSymbols(checked ? QDoubleSpinBox::UpDownArrows : QDoubleSpinBox::NoButtons);

  ui->doubleSpinBoxFrete->setValue(ui->checkBoxFreteManual->isChecked()
                                       ? ui->doubleSpinBoxFrete->value()
                                       : qMax(ui->doubleSpinBoxSubTotalBruto->value() * porcFrete / 100., minimoFrete));
}

void Orcamento::on_pushButtonReplicar_clicked() {
  Orcamento *replica = new Orcamento(parentWidget());
  replica->ui->pushButtonReplicar->hide();

  replica->ui->itemBoxCliente->setValue(data("idCliente"));
  replica->ui->itemBoxProfissional->setValue(data("idProfissional"));
  replica->ui->itemBoxVendedor->setValue(data("idUsuario"));
  replica->ui->itemBoxEndereco->setValue(data("idEnderecoEntrega"));
  replica->ui->spinBoxValidade->setValue(data("validade").toInt());
  replica->ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
  replica->ui->checkBoxRepresentacao->setChecked(ui->checkBoxRepresentacao->isChecked());
  replica->ui->lineEditReplicaDe->setText(data("idOrcamento").toString());
  replica->ui->plainTextEditObs->setPlainText(data("observacao").toString());

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    replica->ui->itemBoxProduto->setValue(modelItem.data(row, "idProduto"));
    replica->ui->doubleSpinBoxQuant->setValue(modelItem.data(row, "quant").toDouble());
    replica->ui->doubleSpinBoxDesconto->setValue(modelItem.data(row, "desconto").toDouble());
    replica->ui->lineEditObs->setText(modelItem.data(row, "obs").toString());
    replica->adicionarItem();
  }

  replica->show();
}

bool Orcamento::cadastrar() {
  if (not verifyFields()) return false;

  row = isUpdate ? mapper.currentIndex() : model.rowCount();

  if (row == -1) {
    QMessageBox::critical(this, "Erro!", "Erro linha -1 Orçamento: " + QString::number(isUpdate) + "\nMapper: " +
                                             QString::number(mapper.currentIndex()) + "\nModel: " +
                                             QString::number(model.rowCount()));
    return false;
  }

  if (not isUpdate) {
    if (not generateId() or not model.insertRow(row)) {
      QMessageBox::critical(this, "Erro!", "Erro inserindo linha na tabela");
      return false;
    }
  }

  if (not savingProcedures()) return false;

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro ao cadastrar: " + model.lastError().text());
    return false;
  }

  primaryId = ui->lineEditOrcamento->text();

  if (not modelItem.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro ao adicionar um item ao orçamento: " + modelItem.lastError().text());
    return false;
  }

  return true;
}

bool Orcamento::save() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not cadastrar()) {
    QSqlQuery("ROLLBACK").exec();
    return false;
  }

  QSqlQuery("COMMIT").exec();

  isDirty = false;

  viewRegisterById(primaryId);

  if (not silent) successMessage();

  return true;
}

bool Orcamento::verificaCadastroCliente() {
  const int idCliente = ui->itemBoxCliente->value().toInt();

  QSqlQuery queryCliente;
  queryCliente.prepare("SELECT cpf, cnpj FROM cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", idCliente);

  if (not queryCliente.exec() or not queryCliente.first()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro verificando se cliente possui CPF/CNPJ: " + queryCliente.lastError().text());
    return false;
  }

  if (queryCliente.value("cpf").toString().isEmpty() and queryCliente.value("cnpj").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Cliente não possui CPF/CNPJ cadastrado!");
    CadastroCliente *cadCliente = new CadastroCliente(this);
    cadCliente->viewRegisterById(idCliente);
    cadCliente->show();
    return false;
  }

  QSqlQuery queryCadastro;
  queryCadastro.prepare("SELECT idCliente FROM cliente_has_endereco WHERE "
                        "idCliente = :idCliente");
  queryCadastro.bindValue(":idCliente", idCliente);

  if (not queryCadastro.exec()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro verificando se cliente possui endereço: " + queryCadastro.lastError().text());
    return false;
  }

  if (not queryCadastro.first()) {
    QMessageBox::critical(this, "Erro!", "Cliente não possui endereço cadastrado!");
    CadastroCliente *cadCliente = new CadastroCliente(this);
    cadCliente->viewRegisterById(idCliente);
    cadCliente->show();
    return false;
  }

  queryCadastro.prepare("SELECT incompleto FROM orcamento LEFT JOIN cliente ON "
                        "orcamento.idCliente = cliente.idCliente "
                        "WHERE cliente.idCliente = :idCliente AND incompleto = TRUE");
  queryCadastro.bindValue(":idCliente", idCliente);

  if (not queryCadastro.exec()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro verificando se cadastro do cliente está completo: " + queryCadastro.lastError().text());
    return false;
  }

  if (queryCadastro.first()) {
    QMessageBox::critical(this, "Erro!", "Cadastro incompleto, deve terminar!");
    CadastroCliente *cadCliente = new CadastroCliente(this);
    cadCliente->viewRegisterById(idCliente);
    cadCliente->show();
    return false;
  }

  return true;
}

void Orcamento::on_pushButtonGerarExcel_clicked() {
  Excel excel(ui->lineEditOrcamento->text());
  excel.gerarExcel();
}

void Orcamento::on_checkBoxRepresentacao_toggled(const bool checked) {
  ui->itemBoxProduto->searchDialog()->setRepresentacao(" AND representacao = " + QString(checked ? "TRUE" : "FALSE"));
}

void Orcamento::on_doubleSpinBoxDesconto_valueChanged(const double) {
  isBlockedDesconto = true;
  calcPrecoItemTotal();
  isBlockedDesconto = false;
}

void Orcamento::on_doubleSpinBoxDescontoGlobalReais_valueChanged(const double) {
  if (isBlockedGlobal) return;

  const double liq = ui->doubleSpinBoxSubTotalLiq->value();
  const double desc = ui->doubleSpinBoxDescontoGlobalReais->value();

  if (liq == 0.) return;

  isBlockedReais = true;
  ui->doubleSpinBoxDescontoGlobal->setValue(100 * (1 - ((liq - desc) / liq)));
  isBlockedReais = false;
}

void Orcamento::on_doubleSpinBoxFrete_valueChanged(const double) { calcPrecoGlobalTotal(); }

void Orcamento::on_itemBoxVendedor_textChanged(const QString &) {
  if (ui->itemBoxVendedor->text().isEmpty()) return;
  if (data("freteManual").toBool()) return;

  QSqlQuery queryFrete;
  queryFrete.prepare("SELECT valorMinimoFrete, porcentagemFrete FROM loja WHERE idLoja = :idLoja");
  queryFrete.bindValue(":idLoja", UserSession::fromLoja("usuario.idLoja", ui->itemBoxVendedor->text()));

  if (not queryFrete.exec() or not queryFrete.next()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando parâmetros do frete: " + queryFrete.lastError().text());
    return;
  }

  minimoFrete = queryFrete.value("valorMinimoFrete").toDouble();
  porcFrete = queryFrete.value("porcentagemFrete").toDouble();

  ui->doubleSpinBoxFrete->setValue(ui->checkBoxFreteManual->isChecked()
                                       ? ui->doubleSpinBoxFrete->value()
                                       : qMax(ui->doubleSpinBoxSubTotalBruto->value() * porcFrete / 100., minimoFrete));
}

void Orcamento::on_doubleSpinBoxDescontoGlobal_valueChanged(const double) {
  if (isBlockedGlobal) return;

  isBlockedGlobal = true;
  calcPrecoItemTotal();
  calcPrecoGlobalTotal();
  isBlockedGlobal = false;
}

void Orcamento::on_doubleSpinBoxTotal_valueChanged(const double) {
  if (isBlockedGlobal) return;

  const double liq = ui->doubleSpinBoxSubTotalLiq->value();
  const double frete = ui->doubleSpinBoxFrete->value();
  const double total = ui->doubleSpinBoxTotal->value();
  const double value = 100. * (1. - ((total - frete) / liq));

  if (liq == 0.) return;

  isBlockedTotal = true;
  ui->doubleSpinBoxDescontoGlobal->setValue(value);
  isBlockedTotal = false;
}

void Orcamento::on_doubleSpinBoxTotalItem_valueChanged(const double) {
  if (ui->itemBoxProduto->text().isEmpty() or isBlockedDesconto) return;

  const double quant = ui->doubleSpinBoxQuant->value();
  const double prcUn = ui->lineEditPrecoUn->getValue();
  const double itemBruto = quant * prcUn;
  const double subTotalItem = ui->doubleSpinBoxTotalItem->value();
  const double desconto = (itemBruto - subTotalItem) / itemBruto * 100.;

  if (itemBruto == 0.) return;

  isBlockedTotalItem = true;
  ui->doubleSpinBoxDesconto->setValue(desconto);
  isBlockedTotalItem = false;
}

void Orcamento::on_doubleSpinBoxSubTotalBruto_valueChanged(const double) {
  if (data("freteManual").toBool()) return;

  ui->doubleSpinBoxFrete->setValue(ui->checkBoxFreteManual->isChecked()
                                       ? ui->doubleSpinBoxFrete->value()
                                       : qMax(ui->doubleSpinBoxSubTotalBruto->value() * porcFrete / 100., minimoFrete));
}

void Orcamento::successMessage() {
  QMessageBox::information(this, "Atenção!", isUpdate ? "Cadastro atualizado!" : "Orçamento cadastrado com sucesso!");
}

void Orcamento::on_comboBoxLoja_currentTextChanged(const QString &) {
  ui->itemBoxVendedorIndicou->clear();
  ui->itemBoxVendedorIndicou->searchDialog()->setFilter("idLoja = " + ui->comboBoxLoja->getCurrentValue().toString() +
                                                        " AND tipo = 'VENDEDOR'");
}

// NOTE: model.submitAll faz mapper voltar para -1, select tambem (talvez porque
// submitAll chama select)
// NOTE: se produto for estoque permitir vender por peça
