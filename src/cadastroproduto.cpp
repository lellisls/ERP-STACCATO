#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "cadastrofornecedor.h"
#include "cadastroproduto.h"
#include "ui_cadastroproduto.h"
#include "usersession.h"

CadastroProduto::CadastroProduto(QWidget *parent)
  : RegisterDialog("Produto", "idProduto", parent), ui(new Ui::CadastroProduto) {
  ui->setupUi(this);

  // InputMasks
  ui->lineEditCodBarras->setInputMask("9999999999999;_");
  ui->lineEditNCM->setInputMask("99999999;_");

  ui->comboBoxOrigem->addItem("0 - Nacional", 0);
  ui->comboBoxOrigem->addItem("1 - Imp. Direta", 1);
  ui->comboBoxOrigem->addItem("2 - Merc. Interno", 2);

  setupMapper();
  newRegister();

  SearchDialog *sdFornecedor = SearchDialog::fornecedor(this);
  ui->itemBoxFornecedor->setSearchDialog(sdFornecedor);

  CadastroFornecedor *cadFornecedor = new CadastroFornecedor(this);
  ui->itemBoxFornecedor->setRegisterDialog(cadFornecedor);

  if (UserSession::getTipoUsuario() != "ADMINISTRADOR") {
    ui->pushButtonRemover->setDisabled(true);
  }
}

CadastroProduto::~CadastroProduto() { delete ui; }

void CadastroProduto::clearFields() {
  foreach (QLineEdit *line, this->findChildren<QLineEdit *>()) { line->clear(); }
  ui->radioButtonDesc->setChecked(false);
  ui->radioButtonLote->setChecked(false);
}

void CadastroProduto::updateMode() {
  ui->pushButtonCadastrar->hide();
  ui->pushButtonAtualizar->show();
  ui->pushButtonRemover->show();
}

void CadastroProduto::registerMode() {
  ui->pushButtonCadastrar->show();
  ui->pushButtonAtualizar->hide();
  ui->pushButtonRemover->hide();
}

bool CadastroProduto::verifyFields(int row) {
  Q_UNUSED(row);

  if (not RegisterDialog::verifyFields({ui->lineEditCodBarras, ui->lineEditCodComer, ui->lineEditCodInd,
                                       ui->lineEditColecao, ui->lineEditDescricao, ui->lineEditFormComer,
                                       ui->lineEditICMS, ui->lineEditNCM})) {
    return false;
  }

  if (ui->doubleSpinBoxCusto->value() == 0) {
    ui->doubleSpinBoxCusto->setFocus();
    QMessageBox::warning(this, "Atenção!", "Custo inválido!", QMessageBox::Ok, QMessageBox::NoButton);
  }

  if (ui->doubleSpinBoxVenda->value() == 0) {
    ui->doubleSpinBoxVenda->setFocus();
    QMessageBox::warning(this, "Atenção!", "Preço inválido!", QMessageBox::Ok, QMessageBox::NoButton);
  }

  if (ui->itemBoxFornecedor->value().isNull()) {
    ui->itemBoxFornecedor->setFocus();
    QMessageBox::warning(this, "Atenção!", "Você não preencheu um item obrigatório!", QMessageBox::Ok,
                         QMessageBox::NoButton);
    return false;
  }

  return true;
}

void CadastroProduto::setupMapper() {
  mapper.setModel(&model);

  addMapping(ui->lineEditDescricao, "descricao");
  addMapping(ui->comboBoxUn, "un");
  addMapping(ui->lineEditColecao, "colecao");
  addMapping(ui->lineEditFormComer, "formComercial");
  addMapping(ui->lineEditCodComer, "codComercial");
  addMapping(ui->lineEditCodInd, "codIndustrial");
  addMapping(ui->lineEditCodBarras, "codBarras");
  addMapping(ui->lineEditNCM, "ncm");
  addMapping(ui->lineEditICMS, "icms");

  addMapping(ui->doubleSpinBoxPcCx, "pccx");
  addMapping(ui->doubleSpinBoxM2Cx, "m2cx", "value");
  addMapping(ui->doubleSpinBoxQtePallet, "qtdPallet", "value");
  addMapping(ui->doubleSpinBoxCusto, "custo", "value");
  addMapping(ui->doubleSpinBoxIPI, "ipi", "value");
  addMapping(ui->doubleSpinBoxST, "st", "value");
  addMapping(ui->doubleSpinBoxMarkup, "markup", "value");
  addMapping(ui->doubleSpinBoxVenda, "precoVenda", "value");
  addMapping(ui->doubleSpinBoxComissao, "comissao", "value");
  addMapping(ui->doubleSpinBoxEstoque, "estoque");

  addMapping(ui->textEditObserv, "observacoes", "plainText");
  addMapping(ui->comboBoxSitTrib, "situacaoTributaria");
  addMapping(ui->itemBoxFornecedor, "idFornecedor", "value");
  addMapping(ui->comboBoxOrigem, "origem");
  addMapping(ui->radioButtonDesc, "descontinuado");
  addMapping(ui->radioButtonLote, "temLote");
}

bool CadastroProduto::savingProcedures(int row) {
  setData(row, "Fornecedor", ui->itemBoxFornecedor->text());
  setData(row, "idFornecedor", ui->itemBoxFornecedor->value());

  setData(row, "temLote", ui->radioButtonLote->isChecked());
  setData(row, "descontinuado", ui->radioButtonDesc->isChecked());
  setData(row, "origem", ui->comboBoxOrigem->currentData());
  setData(row, "sitTrib", ui->comboBoxSitTrib->currentText());

  setData(row, "descricao", ui->lineEditDescricao->text());
  setData(row, "un", ui->comboBoxUn->currentText());
  setData(row, "colecao", ui->lineEditColecao->text());
  setData(row, "formComercial", ui->lineEditFormComer->text());
  setData(row, "codComercial", ui->lineEditCodComer->text());
  setData(row, "codIndustrial", ui->lineEditCodInd->text());
  setData(row, "codBarras", ui->lineEditCodBarras->text());
  setData(row, "ncm", ui->lineEditNCM->text());
  setData(row, "icms", ui->lineEditICMS->text());

  setData(row, "pccx", ui->doubleSpinBoxPcCx->value());
  setData(row, "m2cx", ui->doubleSpinBoxM2Cx->value());
  setData(row, "qtdPallet", ui->doubleSpinBoxQtePallet->value());
  setData(row, "custo", ui->doubleSpinBoxCusto->value());
  setData(row, "ipi", ui->doubleSpinBoxIPI->value());
  setData(row, "st", ui->doubleSpinBoxST->value());
  setData(row, "markup", ui->doubleSpinBoxMarkup->value());
  setData(row, "precoVenda", ui->doubleSpinBoxVenda->value());
  setData(row, "comissao", ui->doubleSpinBoxComissao->value());
  setData(row, "estoque", ui->doubleSpinBoxEstoque->value());

  setData(row, "obsercacoes", ui->textEditObserv->toPlainText());
  setData(row, "situacaoTributaria", ui->comboBoxSitTrib->currentText());
  setData(row, "origem", ui->comboBoxOrigem->currentData());
  setData(row, "descontinuado", ui->radioButtonDesc->isChecked());
  setData(row, "temLote", ui->radioButtonLote->isChecked());

  return true;
}

void CadastroProduto::on_pushButtonCadastrar_clicked() { save(); }

void CadastroProduto::on_pushButtonAtualizar_clicked() { save(); }

void CadastroProduto::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroProduto::on_pushButtonRemover_clicked() { remove(); }

void CadastroProduto::on_pushButtonCancelar_clicked() { close(); }

void CadastroProduto::on_pushButtonBuscar_clicked() {
  SearchDialog *sdProd = SearchDialog::produto(this);
  sdProd->showMaximized();
  connect(sdProd, &SearchDialog::itemSelected, this, &CadastroProduto::changeItem);
}

void CadastroProduto::changeItem(QVariant value, QString text) {
  Q_UNUSED(text);

  viewRegisterById(value);
}

void CadastroProduto::show() {
  QWidget::show();
  adjustSize();
}
