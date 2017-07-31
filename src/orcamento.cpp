#include <QDebug>
#include <QDesktopServices>
#include <QMessageBox>
#include <QSqlError>
#include <QtMath>

#include "baixaorcamento.h"
#include "cadastrocliente.h"
#include "doubledelegate.h"
#include "excel.h"
#include "impressao.h"
#include "orcamento.h"
#include "porcentagemdelegate.h"
#include "reaisdelegate.h"
#include "searchdialogproxy.h"
#include "ui_orcamento.h"
#include "usersession.h"
#include "venda.h"

Orcamento::Orcamento(QWidget *parent) : RegisterDialog("orcamento", "idOrcamento", parent), ui(new Ui::Orcamento) {
  ui->setupUi(this);

  setupTables();

  for (auto const *line : findChildren<QLineEdit *>()) connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);

  ui->itemBoxCliente->setSearchDialog(SearchDialog::cliente(this));
  ui->itemBoxCliente->setRegisterDialog(new CadastroCliente(this));
  ui->itemBoxProduto->setSearchDialog(SearchDialog::produto(false, this));
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

  setupConnections();
}

Orcamento::~Orcamento() { delete ui; }

void Orcamento::setupConnections() {
  connect(ui->checkBoxFreteManual, &QCheckBox::clicked, this, &Orcamento::on_checkBoxFreteManual_clicked);
  connect(ui->checkBoxRepresentacao, &QCheckBox::toggled, this, &Orcamento::on_checkBoxRepresentacao_toggled);
  connect(ui->comboBoxLoja, &ComboBox::currentTextChanged, this, &Orcamento::on_comboBoxLoja_currentTextChanged);
  connect(ui->doubleSpinBoxCaixas, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxCaixas_valueChanged);
  connect(ui->doubleSpinBoxDesconto, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxDesconto_valueChanged);
  connect(ui->doubleSpinBoxDescontoGlobal, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxDescontoGlobal_valueChanged);
  connect(ui->doubleSpinBoxDescontoGlobalReais, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxDescontoGlobalReais_valueChanged);
  connect(ui->doubleSpinBoxFrete, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxFrete_valueChanged);
  connect(ui->doubleSpinBoxQuant, &QDoubleSpinBox::editingFinished, this, &Orcamento::on_doubleSpinBoxQuant_editingFinished);
  connect(ui->doubleSpinBoxQuant, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxQuant_valueChanged);
  connect(ui->doubleSpinBoxSubTotalBruto, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxSubTotalBruto_valueChanged);
  connect(ui->doubleSpinBoxSubTotalLiq, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxSubTotalLiq_valueChanged);
  connect(ui->doubleSpinBoxTotal, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxTotal_valueChanged);
  connect(ui->doubleSpinBoxTotalItem, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxTotalItem_valueChanged);
  connect(ui->itemBoxCliente, &ItemBox::textChanged, this, &Orcamento::on_itemBoxCliente_textChanged);
  connect(ui->itemBoxProduto, &ItemBox::textChanged, this, &Orcamento::on_itemBoxProduto_textChanged);
  connect(ui->itemBoxVendedor, &ItemBox::textChanged, this, &Orcamento::on_itemBoxVendedor_textChanged);
  connect(ui->pushButtonAdicionarItem, &QPushButton::clicked, this, &Orcamento::on_pushButtonAdicionarItem_clicked);
  connect(ui->pushButtonApagarOrc, &QPushButton::clicked, this, &Orcamento::on_pushButtonApagarOrc_clicked);
  connect(ui->pushButtonAtualizarItem, &QPushButton::clicked, this, &Orcamento::on_pushButtonAtualizarItem_clicked);
  connect(ui->pushButtonAtualizarOrcamento, &QPushButton::clicked, this, &Orcamento::on_pushButtonAtualizarOrcamento_clicked);
  connect(ui->pushButtonCadastrarOrcamento, &QPushButton::clicked, this, &Orcamento::on_pushButtonCadastrarOrcamento_clicked);
  connect(ui->pushButtonGerarExcel, &QPushButton::clicked, this, &Orcamento::on_pushButtonGerarExcel_clicked);
  connect(ui->pushButtonGerarVenda, &QPushButton::clicked, this, &Orcamento::on_pushButtonGerarVenda_clicked);
  connect(ui->pushButtonImprimir, &QPushButton::clicked, this, &Orcamento::on_pushButtonImprimir_clicked);
  connect(ui->pushButtonLimparSelecao, &QPushButton::clicked, this, &Orcamento::on_pushButtonLimparSelecao_clicked);
  connect(ui->pushButtonRemoverItem, &QPushButton::clicked, this, &Orcamento::on_pushButtonRemoverItem_clicked);
  connect(ui->pushButtonReplicar, &QPushButton::clicked, this, &Orcamento::on_pushButtonReplicar_clicked);
  connect(ui->tableProdutos, &TableView::clicked, this, &Orcamento::on_tableProdutos_clicked);
}

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

void Orcamento::unsetConnections() {
  disconnect(ui->checkBoxFreteManual, &QCheckBox::clicked, this, &Orcamento::on_checkBoxFreteManual_clicked);
  disconnect(ui->checkBoxRepresentacao, &QCheckBox::toggled, this, &Orcamento::on_checkBoxRepresentacao_toggled);
  disconnect(ui->comboBoxLoja, &ComboBox::currentTextChanged, this, &Orcamento::on_comboBoxLoja_currentTextChanged);
  disconnect(ui->doubleSpinBoxCaixas, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxCaixas_valueChanged);
  disconnect(ui->doubleSpinBoxDesconto, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxDesconto_valueChanged);
  disconnect(ui->doubleSpinBoxDescontoGlobal, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxDescontoGlobal_valueChanged);
  disconnect(ui->doubleSpinBoxDescontoGlobalReais, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxDescontoGlobalReais_valueChanged);
  disconnect(ui->doubleSpinBoxFrete, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxFrete_valueChanged);
  disconnect(ui->doubleSpinBoxQuant, &QDoubleSpinBox::editingFinished, this, &Orcamento::on_doubleSpinBoxQuant_editingFinished);
  disconnect(ui->doubleSpinBoxQuant, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxQuant_valueChanged);
  disconnect(ui->doubleSpinBoxSubTotalBruto, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxSubTotalBruto_valueChanged);
  disconnect(ui->doubleSpinBoxSubTotalLiq, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxSubTotalLiq_valueChanged);
  disconnect(ui->doubleSpinBoxTotal, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxTotal_valueChanged);
  disconnect(ui->doubleSpinBoxTotalItem, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxTotalItem_valueChanged);
  disconnect(ui->itemBoxCliente, &ItemBox::textChanged, this, &Orcamento::on_itemBoxCliente_textChanged);
  disconnect(ui->itemBoxProduto, &ItemBox::textChanged, this, &Orcamento::on_itemBoxProduto_textChanged);
  disconnect(ui->itemBoxVendedor, &ItemBox::textChanged, this, &Orcamento::on_itemBoxVendedor_textChanged);
  disconnect(ui->pushButtonAdicionarItem, &QPushButton::clicked, this, &Orcamento::on_pushButtonAdicionarItem_clicked);
  disconnect(ui->pushButtonApagarOrc, &QPushButton::clicked, this, &Orcamento::on_pushButtonApagarOrc_clicked);
  disconnect(ui->pushButtonAtualizarItem, &QPushButton::clicked, this, &Orcamento::on_pushButtonAtualizarItem_clicked);
  disconnect(ui->pushButtonAtualizarOrcamento, &QPushButton::clicked, this, &Orcamento::on_pushButtonAtualizarOrcamento_clicked);
  disconnect(ui->pushButtonCadastrarOrcamento, &QPushButton::clicked, this, &Orcamento::on_pushButtonCadastrarOrcamento_clicked);
  disconnect(ui->pushButtonGerarExcel, &QPushButton::clicked, this, &Orcamento::on_pushButtonGerarExcel_clicked);
  disconnect(ui->pushButtonGerarVenda, &QPushButton::clicked, this, &Orcamento::on_pushButtonGerarVenda_clicked);
  disconnect(ui->pushButtonImprimir, &QPushButton::clicked, this, &Orcamento::on_pushButtonImprimir_clicked);
  disconnect(ui->pushButtonLimparSelecao, &QPushButton::clicked, this, &Orcamento::on_pushButtonLimparSelecao_clicked);
  disconnect(ui->pushButtonRemoverItem, &QPushButton::clicked, this, &Orcamento::on_pushButtonRemoverItem_clicked);
  disconnect(ui->pushButtonReplicar, &QPushButton::clicked, this, &Orcamento::on_pushButtonReplicar_clicked);
  disconnect(ui->tableProdutos, &TableView::clicked, this, &Orcamento::on_tableProdutos_clicked);
}

bool Orcamento::viewRegister() {
  unsetConnections();

  modelItem.setFilter("idOrcamento = '" + data(0, "idOrcamento").toString() + "'");

  if (not modelItem.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela orcamento_has_produto: " + modelItem.lastError().text());
    return false;
  }

  if (not RegisterDialog::viewRegister()) return false;

  if (not buscarParametrosFrete()) return false;

  novoItem();

  const QString status = data("status").toString();

  if (status == "PERDIDO" or status == "CANCELADO") {
    ui->labelBaixa->show();
    ui->plainTextEditBaixa->show();
  }

  if (status == "ATIVO") ui->pushButtonReplicar->hide();

  if (ui->dateTimeEdit->dateTime().addDays(data("validade").toInt()).date() < QDateTime::currentDateTime().date() or status != "ATIVO") {
    isReadOnly = true;

    ui->pushButtonGerarVenda->hide();
    ui->pushButtonAtualizarOrcamento->hide();
    ui->pushButtonReplicar->show();

    ui->pushButtonAdicionarItem->hide();
    ui->pushButtonAtualizarItem->hide();
    ui->pushButtonRemoverItem->hide();
    ui->pushButtonLimparSelecao->hide();
    ui->pushButtonCalculadora->hide();

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

  ui->doubleSpinBoxDescontoGlobalReais->setMaximum(ui->doubleSpinBoxSubTotalLiq->value());

  setupConnections();

  return true;
}

void Orcamento::novoItem() {
  ui->pushButtonAdicionarItem->show();
  ui->pushButtonAtualizarItem->hide();
  ui->pushButtonRemoverItem->hide();
  ui->itemBoxProduto->clear();
  ui->tableProdutos->clearSelection();
  ui->tableProdutos->resizeColumnsToContents();

  // -----------------------

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
}

void Orcamento::setupMapper() {
  addMapping(ui->checkBoxRepresentacao, "representacao");
  addMapping(ui->dateTimeEdit, "data");
  addMapping(ui->doubleSpinBoxDescontoGlobal, "descontoPorc");
  addMapping(ui->doubleSpinBoxDescontoGlobalReais, "descontoReais");
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

  mapperItem.setModel(ui->tableProdutos->model());
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

bool Orcamento::save() { return save(false); }

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

    calcPrecoGlobalTotal();

    save();
  }

  if (modelItem.rowCount() == 0) {
    if (ui->lineEditOrcamento->text() == "Auto gerado") ui->checkBoxRepresentacao->setEnabled(true);
    ui->itemBoxProduto->getSearchDialog()->setFornecedorRep("");
  }

  novoItem();
}

bool Orcamento::generateId() {
  QString id = UserSession::fromLoja("sigla", UserSession::tipoUsuario() == "VENDEDOR ESPECIAL" and not ui->itemBoxVendedorIndicou->text().isEmpty() ? ui->itemBoxVendedorIndicou->text()
                                                                                                                                                     : ui->itemBoxVendedor->text()) +
               "-" + QDate::currentDate().toString("yy");

  const QString replica = ui->lineEditReplicaDe->text();

  if (replica.isEmpty()) {
    QSqlQuery query;
    query.prepare("SELECT MAX(idOrcamento) AS idOrcamento FROM orcamento WHERE idOrcamento LIKE :id");
    query.bindValue(":id", id + "%");

    if (not query.exec()) {
      error = "Erro buscando próximo id disponível: " + query.lastError().text();
      return false;
    }

    const int last = query.first() ? query.value("idOrcamento").toString().remove(id).left(4).toInt() : 0;

    id += QString("%1").arg(last + 1, 4, 10, QChar('0'));
    id += ui->checkBoxRepresentacao->isChecked() ? "R" : "";
    id += "O";

    if (id.size() != 12 and id.size() != 13) {
      error = "Ocorreu algum erro ao gerar id: " + id;
      return false;
    }
  } else {
    QSqlQuery query;
    query.prepare("SELECT COALESCE(MAX(CAST(REPLACE(idOrcamento, LEFT(idOrcamento, LOCATE('Rev', idOrcamento) + 2), "
                  "'') AS UNSIGNED)) + 1, 1) AS revisao FROM orcamento WHERE LENGTH(idOrcamento) > 16 AND idOrcamento "
                  "LIKE :idOrcamento");
    query.bindValue(":idOrcamento", replica.left(16) + "%");

    if (not query.exec() or not query.first()) {
      error = "Erro buscando próxima revisão disponível: " + query.lastError().text();
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
    QMessageBox::critical(this, "Erro!", R"(Endereço inválido! Se não possui endereço, escolha "Não há".)");
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
  if (not setData("descontoReais", ui->doubleSpinBoxSubTotalLiq->value() * ui->doubleSpinBoxDescontoGlobal->value() / 100.)) return false;
  if (not setData("frete", ui->doubleSpinBoxFrete->value())) return false;
  if (not setData("idCliente", ui->itemBoxCliente->getValue())) return false;
  if (not setData("idEnderecoEntrega", ui->itemBoxEndereco->getValue())) return false;
  if (not setData("idLoja", UserSession::fromLoja("usuario.idLoja", ui->itemBoxVendedor->text()))) return false;
  if (not setData("idOrcamento", ui->lineEditOrcamento->text())) return false;
  if (not setData("idProfissional", ui->itemBoxProfissional->getValue())) return false;
  if (not setData("idUsuario", ui->itemBoxVendedor->getValue())) return false;
  if (not setData("idUsuarioIndicou", ui->itemBoxVendedorIndicou->getValue())) return false;
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
    if (not modelItem.setData(row, "idLoja", model.data(currentRow, "idLoja"))) return false;
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
    query.prepare("UPDATE orcamento SET status = 'REPLICADO', replicadoEm = :idReplica WHERE idOrcamento = :idOrcamento");
    query.bindValue(":idReplica", ui->lineEditOrcamento->text());
    query.bindValue(":idOrcamento", ui->lineEditReplicaDe->text());

    if (not query.exec()) {
      error = "Erro salvando replicadoEm: " + query.lastError().text();
      return false;
    }
  }

  return true;
}

void Orcamento::clearFields() {
  RegisterDialog::clearFields();

  if (UserSession::tipoUsuario() == "VENDEDOR" or UserSession::tipoUsuario() == "VENDEDOR ESPECIAL") ui->itemBoxVendedor->setValue(UserSession::idUsuario());

  ui->itemBoxEndereco->setEnabled(false);
}

void Orcamento::on_pushButtonRemoverItem_clicked() { removeItem(); }

void Orcamento::on_doubleSpinBoxQuant_valueChanged(const double) {
  const double caixas = ui->doubleSpinBoxQuant->value() / ui->spinBoxUnCx->value();

  if (ui->doubleSpinBoxCaixas->value() != caixas) ui->doubleSpinBoxCaixas->setValue(caixas);
}

void Orcamento::on_doubleSpinBoxQuant_editingFinished() { ui->doubleSpinBoxQuant->setValue(ui->doubleSpinBoxCaixas->value() * ui->doubleSpinBoxQuant->singleStep()); }

void Orcamento::on_pushButtonCadastrarOrcamento_clicked() { save(); }

void Orcamento::on_pushButtonAtualizarOrcamento_clicked() { save(); }

void Orcamento::calcPrecoGlobalTotal() {
  double subTotalBruto = 0.;
  double subTotalItens = 0.;

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    const double itemBruto = modelItem.data(row, "quant").toDouble() * modelItem.data(row, "prcUnitario").toDouble();
    const double descItem = modelItem.data(row, "desconto").toDouble() / 100.;
    const double stItem = itemBruto * (1. - descItem);
    subTotalBruto += itemBruto;
    subTotalItens += stItem;
  }

  ui->doubleSpinBoxSubTotalBruto->setValue(subTotalBruto);
  ui->doubleSpinBoxSubTotalLiq->setValue(subTotalItens);
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

  ui->tableProdutos->setModel(new SearchDialogProxy(&modelItem, this));
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

  if (not modelItem.setData(row, "idProduto", ui->itemBoxProduto->getValue().toInt())) return;
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
  if (not modelItem.setData(row, "parcial", modelItem.data(row, "quant").toDouble() * modelItem.data(row, "prcUnitario").toDouble())) return;
  if (not modelItem.setData(row, "parcialDesc", ui->doubleSpinBoxTotalItem->value())) return;
  if (not modelItem.setData(row, "descGlobal", ui->doubleSpinBoxDescontoGlobal->value())) return;
  if (not modelItem.setData(row, "total", ui->doubleSpinBoxTotalItem->value() * (1 - (ui->doubleSpinBoxDescontoGlobal->value() / 100)))) return;

  calcPrecoGlobalTotal();

  if (ui->lineEditOrcamento->text() != "Auto gerado") save(true);

  if (modelItem.rowCount() == 1 and ui->checkBoxRepresentacao->isChecked()) ui->itemBoxProduto->getSearchDialog()->setFornecedorRep(modelItem.data(row, "fornecedor").toString());

  isDirty = true;
}

void Orcamento::on_pushButtonAdicionarItem_clicked() { adicionarItem(); }

void Orcamento::on_pushButtonAtualizarItem_clicked() { atualizarItem(); }

void Orcamento::on_pushButtonGerarVenda_clicked() {
  if (not save(true)) return;

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

  auto *venda = new Venda(parentWidget());
  venda->prepararVenda(ui->lineEditOrcamento->text());

  close();
}

void Orcamento::on_doubleSpinBoxSubTotalLiq_valueChanged(const double) {
  const double subTotalLiq = ui->doubleSpinBoxSubTotalLiq->value();
  const double descReais = ui->doubleSpinBoxDescontoGlobalReais->value();
  const double frete = ui->doubleSpinBoxFrete->value();

  ui->doubleSpinBoxDescontoGlobalReais->setMaximum(subTotalLiq);

  unsetConnections();

  ui->doubleSpinBoxTotal->setValue(subTotalLiq - descReais + frete);

  setupConnections();
}

void Orcamento::on_doubleSpinBoxCaixas_valueChanged(const double caixas) {
  const double caixas2 = fmod(caixas, ui->doubleSpinBoxCaixas->singleStep()) != 0. ? ceil(caixas) : caixas;
  const double quant = caixas2 * ui->spinBoxUnCx->value();
  const double prcUn = ui->lineEditPrecoUn->getValue();
  const double desc = ui->doubleSpinBoxDesconto->value() / 100.;
  const double itemBruto = quant * prcUn;

  unsetConnections();

  ui->doubleSpinBoxQuant->setValue(quant);
  ui->doubleSpinBoxTotalItem->setValue(itemBruto * (1. - desc));

  setupConnections();
}

void Orcamento::on_pushButtonApagarOrc_clicked() {
  auto *baixa = new BaixaOrcamento(data("idOrcamento").toString(), this);
  baixa->show();
}

void Orcamento::on_itemBoxProduto_textChanged(const QString &) {
  if (ui->itemBoxProduto->text().isEmpty()) return;

  QSqlQuery query;
  query.prepare("SELECT un, precoVenda, estoque, fornecedor, codComercial, formComercial, m2cx, pccx, minimo, estoque_promocao FROM produto WHERE idProduto = :idProduto");
  query.bindValue(":idProduto", ui->itemBoxProduto->getValue());

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro na busca do produto: " + query.lastError().text());
    return;
  }

  const QString un = query.value("un").toString().toUpper();

  ui->lineEditUn->setText(un);
  ui->lineEditPrecoUn->setValue(query.value("precoVenda").toDouble());
  ui->lineEditEstoque->setValue(query.value("estoque").toInt());
  ui->lineEditFornecedor->setText(query.value("fornecedor").toString());
  ui->lineEditCodComercial->setText(query.value("codComercial").toString());
  ui->lineEditFormComercial->setText(query.value("formComercial").toString());

  const QString uncxString = un.contains("M2") or un.contains("M²") or un.contains("ML") ? "m2cx" : "pccx";

  ui->spinBoxUnCx->setValue(query.value(uncxString).toDouble());

  const double minimo = query.value("minimo").toDouble();
  const double uncx = query.value(uncxString).toDouble();

  ui->spinBoxMinimo->setValue(minimo);
  ui->doubleSpinBoxQuant->setMinimum(minimo);
  ui->doubleSpinBoxCaixas->setMinimum(minimo / uncx);

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
  ui->doubleSpinBoxQuant->setSingleStep(uncx);

  ui->doubleSpinBoxQuant->setValue(0.);
  ui->doubleSpinBoxCaixas->setValue(0.);
  ui->doubleSpinBoxDesconto->setValue(0.);

  ui->tableProdutos->clearSelection();
}

void Orcamento::on_itemBoxCliente_textChanged(const QString &) {
  ui->itemBoxEndereco->getSearchDialog()->setFilter("idCliente = " + QString::number(ui->itemBoxCliente->getValue().toInt()) + " AND desativado = FALSE OR idEndereco = 1");

  QSqlQuery queryCliente;
  queryCliente.prepare("SELECT idProfissionalRel FROM cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", ui->itemBoxCliente->getValue());

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

  ui->doubleSpinBoxFrete->setValue(ui->checkBoxFreteManual->isChecked() ? ui->doubleSpinBoxFrete->value() : qMax(ui->doubleSpinBoxSubTotalBruto->value() * porcFrete / 100., minimoFrete));
}

void Orcamento::on_pushButtonReplicar_clicked() {
  auto *replica = new Orcamento(parentWidget());

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

  replica->calcPrecoGlobalTotal();

  replica->show();
}

bool Orcamento::cadastrar() {
  currentRow = isUpdate ? mapper.currentIndex() : model.rowCount();

  if (currentRow == -1) {
    error = "Erro linha -1 Orçamento: " + QString::number(isUpdate) + "\nMapper: " + QString::number(mapper.currentIndex()) + "\nModel: " + QString::number(model.rowCount());
    return false;
  }

  if (not isUpdate) {
    if (not generateId() or not model.insertRow(currentRow)) {
      error = "Erro inserindo linha na tabela";
      return false;
    }
  }

  if (not savingProcedures()) return false;

  if (not model.submitAll()) {
    error = "Erro ao cadastrar: " + model.lastError().text();
    return false;
  }

  primaryId = ui->lineEditOrcamento->text();

  if (primaryId.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "primaryId está vazio!");
    return false;
  }

  if (not modelItem.submitAll()) {
    error = "Erro ao adicionar um item ao orçamento: " + modelItem.lastError().text();
    return false;
  }

  return true;
}

bool Orcamento::save(const bool silent) {
  if (not verifyFields()) return false;

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not cadastrar()) {
    QSqlQuery("ROLLBACK").exec();
    if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
    return false;
  }

  QSqlQuery("COMMIT").exec();

  isDirty = false;

  viewRegisterById(primaryId);

  if (not silent) successMessage();

  return true;
}

bool Orcamento::verificaCadastroCliente() {
  const int idCliente = ui->itemBoxCliente->getValue().toInt();

  QSqlQuery queryCliente;
  queryCliente.prepare("SELECT cpf, cnpj FROM cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", idCliente);

  if (not queryCliente.exec() or not queryCliente.first()) {
    QMessageBox::critical(this, "Erro!", "Erro verificando se cliente possui CPF/CNPJ: " + queryCliente.lastError().text());
    return false;
  }

  if (queryCliente.value("cpf").toString().isEmpty() and queryCliente.value("cnpj").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Cliente não possui CPF/CNPJ cadastrado!");
    auto *cadCliente = new CadastroCliente(this);
    cadCliente->viewRegisterById(idCliente);
    cadCliente->show();
    return false;
  }

  QSqlQuery queryCadastro;
  queryCadastro.prepare("SELECT idCliente FROM cliente_has_endereco WHERE "
                        "idCliente = :idCliente");
  queryCadastro.bindValue(":idCliente", idCliente);

  if (not queryCadastro.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro verificando se cliente possui endereço: " + queryCadastro.lastError().text());
    return false;
  }

  if (not queryCadastro.first()) {
    QMessageBox::critical(this, "Erro!", "Cliente não possui endereço cadastrado!");
    auto *cadCliente = new CadastroCliente(this);
    cadCliente->viewRegisterById(idCliente);
    cadCliente->show();
    return false;
  }

  queryCadastro.prepare("SELECT incompleto FROM orcamento LEFT JOIN cliente ON orcamento.idCliente = cliente.idCliente WHERE cliente.idCliente = :idCliente AND incompleto = TRUE");
  queryCadastro.bindValue(":idCliente", idCliente);

  if (not queryCadastro.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro verificando se cadastro do cliente está completo: " + queryCadastro.lastError().text());
    return false;
  }

  if (queryCadastro.first()) {
    QMessageBox::critical(this, "Erro!", "Cadastro incompleto, deve terminar!");
    auto *cadCliente = new CadastroCliente(this);
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

void Orcamento::on_checkBoxRepresentacao_toggled(const bool checked) { ui->itemBoxProduto->getSearchDialog()->setRepresentacao(" AND representacao = " + QString(checked ? "TRUE" : "FALSE")); }

void Orcamento::on_doubleSpinBoxDesconto_valueChanged(const double desconto) {
  const double caixas = ui->doubleSpinBoxCaixas->value();
  const double caixas2 = fmod(caixas, ui->doubleSpinBoxCaixas->singleStep()) != 0. ? ceil(caixas) : caixas;
  const double quant = caixas2 * ui->spinBoxUnCx->value();

  unsetConnections();

  const double prcUn = ui->lineEditPrecoUn->getValue();
  const double itemBruto = quant * prcUn;

  ui->doubleSpinBoxTotalItem->setValue(itemBruto * (1. - (desconto / 100)));

  setupConnections();
}

void Orcamento::on_doubleSpinBoxDescontoGlobalReais_valueChanged(const double desconto) {
  const double liq = ui->doubleSpinBoxSubTotalLiq->value();
  const double frete = ui->doubleSpinBoxFrete->value();

  unsetConnections();

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    if (not modelItem.setData(row, "descGlobal", desconto / liq * 100)) return;

    const double parcialDesc = modelItem.data(row, "parcialDesc").toDouble();
    if (not modelItem.setData(row, "total", parcialDesc * (1 - (desconto / liq)))) return;
  }

  calcPrecoGlobalTotal();

  ui->doubleSpinBoxDescontoGlobal->setValue(desconto / liq * 100);
  ui->doubleSpinBoxTotal->setValue(liq - desconto + frete);

  setupConnections();
}

void Orcamento::on_doubleSpinBoxFrete_valueChanged(const double frete) {
  const double subTotalLiq = ui->doubleSpinBoxSubTotalLiq->value();
  const double desconto = ui->doubleSpinBoxDescontoGlobalReais->value();

  unsetConnections();

  ui->doubleSpinBoxTotal->setValue(subTotalLiq - desconto + frete);

  setupConnections();
}

void Orcamento::on_itemBoxVendedor_textChanged(const QString &) {
  if (ui->itemBoxVendedor->text().isEmpty()) return;
  if (data("freteManual").toBool()) return;

  if (not buscarParametrosFrete()) return;

  ui->doubleSpinBoxFrete->setValue(ui->checkBoxFreteManual->isChecked() ? ui->doubleSpinBoxFrete->value() : qMax(ui->doubleSpinBoxSubTotalBruto->value() * porcFrete / 100., minimoFrete));
}

bool Orcamento::buscarParametrosFrete() {
  QSqlQuery queryFrete;
  queryFrete.prepare("SELECT valorMinimoFrete, porcentagemFrete FROM loja WHERE idLoja = :idLoja");
  queryFrete.bindValue(":idLoja", UserSession::fromLoja("usuario.idLoja", ui->itemBoxVendedor->text()));

  if (not queryFrete.exec() or not queryFrete.next()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando parâmetros do frete: " + queryFrete.lastError().text());
    return false;
  }

  minimoFrete = queryFrete.value("valorMinimoFrete").toDouble();
  porcFrete = queryFrete.value("porcentagemFrete").toDouble();

  return true;
}

void Orcamento::on_doubleSpinBoxDescontoGlobal_valueChanged(const double desconto) {
  const double liq = ui->doubleSpinBoxSubTotalLiq->value();
  const double frete = ui->doubleSpinBoxFrete->value();

  unsetConnections();

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    if (not modelItem.setData(row, "descGlobal", desconto)) return;

    const double parcialDesc = modelItem.data(row, "parcialDesc").toDouble();
    if (not modelItem.setData(row, "total", parcialDesc * (1 - (desconto / 100)))) return;
  }

  calcPrecoGlobalTotal();

  ui->doubleSpinBoxDescontoGlobalReais->setValue(liq * desconto / 100);
  ui->doubleSpinBoxTotal->setValue((liq * (1 - (desconto / 100)) + frete));

  setupConnections();
}

void Orcamento::on_doubleSpinBoxTotal_valueChanged(const double total) {
  const double liq = ui->doubleSpinBoxSubTotalLiq->value();
  const double frete = ui->doubleSpinBoxFrete->value();

  unsetConnections();

  ui->doubleSpinBoxDescontoGlobal->setValue((total - frete) / liq * 100);
  ui->doubleSpinBoxDescontoGlobalReais->setValue(liq - (total - frete));

  setupConnections();
}

void Orcamento::on_doubleSpinBoxTotalItem_valueChanged(const double) {
  if (ui->itemBoxProduto->text().isEmpty()) return;

  const double quant = ui->doubleSpinBoxQuant->value();
  const double prcUn = ui->lineEditPrecoUn->getValue();
  const double itemBruto = quant * prcUn;
  const double subTotalItem = ui->doubleSpinBoxTotalItem->value();
  const double desconto = (itemBruto - subTotalItem) / itemBruto * 100.;

  if (itemBruto == 0.) return;

  unsetConnections();

  ui->doubleSpinBoxDesconto->setValue(desconto);

  setupConnections();
}

void Orcamento::on_doubleSpinBoxSubTotalBruto_valueChanged(const double) {
  if (data("freteManual").toBool()) return;

  ui->doubleSpinBoxFrete->setValue(ui->checkBoxFreteManual->isChecked() ? ui->doubleSpinBoxFrete->value() : qMax(ui->doubleSpinBoxSubTotalBruto->value() * porcFrete / 100., minimoFrete));
}

void Orcamento::successMessage() { QMessageBox::information(this, "Atenção!", isUpdate ? "Cadastro atualizado!" : "Orçamento cadastrado com sucesso!"); }

void Orcamento::on_comboBoxLoja_currentTextChanged(const QString &) {
  qDebug() << "a";
  ui->itemBoxVendedorIndicou->clear();
  ui->itemBoxVendedorIndicou->getSearchDialog()->setFilter("idLoja = " + ui->comboBoxLoja->getCurrentValue().toString() + " AND tipo = 'VENDEDOR'");
}

void Orcamento::on_pushButtonCalculadora_clicked() { QDesktopServices::openUrl(QUrl::fromLocalFile(R"(C:\Windows\System32\calc.exe)")); }

// NOTE: model.submitAll faz mapper voltar para -1, select tambem (talvez porque
// submitAll chama select)
// NOTE: se produto for estoque permitir vender por peça
// TODO: 2orcamento de reposicao nao pode ter profissional associado (bloquear)
// TODO: 'ax06' nao esta mudando caixa (verificar se o minimo de 3 é caixas ou pecas [numa caixa vem 10 pecas, porque
// minimo 3?])
// TODO: quando cadastrar cliente no itemBox mudar para o id dele
// TODO: permitir que o usuario digite um valor e o sistema faça o calculo na linha?
