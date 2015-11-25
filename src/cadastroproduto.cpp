#include <QMessageBox>

#include "cadastroproduto.h"
#include "ui_cadastroproduto.h"
#include "cadastrofornecedor.h"
#include "usersession.h"

CadastroProduto::CadastroProduto(QWidget *parent)
  : RegisterDialog("produto", "idProduto", parent), ui(new Ui::CadastroProduto) {
  ui->setupUi(this);

  for (const QLineEdit *line : findChildren<QLineEdit *>()) {
    connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);
  }

  ui->lineEditCodBarras->setInputMask("9999999999999;_");
  ui->lineEditNCM->setInputMask("99999999;_");

  ui->comboBoxOrigem->addItem("0 - Nacional", 0);
  ui->comboBoxOrigem->addItem("1 - Imp. Direta", 1);
  ui->comboBoxOrigem->addItem("2 - Merc. Interno", 2);

  setupMapper();
  newRegister();

  SearchDialog *sdFornecedor = SearchDialog::fornecedor(this);
  ui->itemBoxFornecedor->setSearchDialog(sdFornecedor);

  SearchDialog *sdProd = SearchDialog::produto(this);
  connect(sdProd, &SearchDialog::itemSelected, this, &CadastroProduto::viewRegisterById);
  connect(ui->pushButtonBuscar, &QAbstractButton::clicked, sdProd, &SearchDialog::show);

  CadastroFornecedor *cadFornecedor = new CadastroFornecedor(this);
  ui->itemBoxFornecedor->setRegisterDialog(cadFornecedor);

  if (UserSession::getTipoUsuario() != "ADMINISTRADOR") {
    ui->pushButtonRemover->setDisabled(true);
  }
}

CadastroProduto::~CadastroProduto() { delete ui; }

void CadastroProduto::clearFields() {
  for (auto const &line : this->findChildren<QLineEdit *>()) {
    line->clear();
  }

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

bool CadastroProduto::verifyFields() {
  if (ui->doubleSpinBoxCusto->value() == 0) {
    ui->doubleSpinBoxCusto->setFocus();
    QMessageBox::critical(this, "Erro!", "Custo inválido!");
    return false;
  }

  if (ui->doubleSpinBoxVenda->value() == 0) {
    ui->doubleSpinBoxVenda->setFocus();
    QMessageBox::critical(this, "Erro!", "Preço inválido!");
    return false;
  }

  if (ui->itemBoxFornecedor->value().isNull()) {
    ui->itemBoxFornecedor->setFocus();
    QMessageBox::critical(this, "Erro!", "Faltou preencher fornecedor!");
    return false;
  }

  if (ui->lineEditICMS->text().isEmpty()) {
    ui->lineEditICMS->setFocus();
    QMessageBox::critical(this, "Erro!", "Faltou preencher ICMS!");
    return false;
  }

  if (ui->lineEditCodComer->text().isEmpty()) {
    ui->lineEditCodComer->setFocus();
    QMessageBox::critical(this, "Erro!", "Faltou preencher Código comercial!");
    return false;
  }

  return true;
}

void CadastroProduto::setupMapper() {
  addMapping(ui->lineEditDescricao, "descricao");
  addMapping(ui->comboBoxUn, "un");
  addMapping(ui->lineEditColecao, "colecao");
  addMapping(ui->lineEditFormComer, "formComercial");
  addMapping(ui->lineEditCodComer, "codComercial");
  addMapping(ui->lineEditCodBarras, "codBarras");
  addMapping(ui->lineEditNCM, "ncm");
  addMapping(ui->lineEditICMS, "icms");
  addMapping(ui->lineEditUI, "ui");

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
  addMapping(ui->comboBoxCST, "cst");
  addMapping(ui->itemBoxFornecedor, "idFornecedor", "value");
  addMapping(ui->comboBoxOrigem, "origem");
  addMapping(ui->radioButtonDesc, "descontinuado");
  addMapping(ui->radioButtonLote, "temLote");
}

bool CadastroProduto::savingProcedures() {
  setData("codBarras", ui->lineEditCodBarras->text());
  setData("codComercial", ui->lineEditCodComer->text());
  setData("colecao", ui->lineEditColecao->text());
  setData("comissao", ui->doubleSpinBoxComissao->value());
  setData("custo", ui->doubleSpinBoxCusto->value());
  setData("descontinuado", ui->radioButtonDesc->isChecked());
  setData("descricao", ui->lineEditDescricao->text());
  setData("estoque", ui->doubleSpinBoxEstoque->value());
  setData("formComercial", ui->lineEditFormComer->text());
  setData("Fornecedor", ui->itemBoxFornecedor->text());
  setData("icms", ui->lineEditICMS->text());
  setData("idFornecedor", ui->itemBoxFornecedor->value());
  setData("ipi", ui->doubleSpinBoxIPI->value());
  setData("m2cx", ui->doubleSpinBoxM2Cx->value());
  setData("markup", ui->doubleSpinBoxMarkup->value());
  setData("ncm", ui->lineEditNCM->text());
  setData("observacoes", ui->textEditObserv->toPlainText());
  setData("origem", ui->comboBoxOrigem->currentData());
  setData("pccx", ui->doubleSpinBoxPcCx->value());
  setData("precoVenda", ui->doubleSpinBoxVenda->value());
  setData("qtdPallet", ui->doubleSpinBoxQtePallet->value());
  setData("cst", ui->comboBoxCST->currentText());
  setData("st", ui->doubleSpinBoxST->value());
  setData("temLote", ui->radioButtonLote->isChecked());
  setData("ui", ui->lineEditUI->text());
  setData("un", ui->comboBoxUn->currentText());

  return isOk;
}

void CadastroProduto::on_pushButtonCadastrar_clicked() { save(); }

void CadastroProduto::on_pushButtonAtualizar_clicked() { update(); }

void CadastroProduto::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroProduto::on_pushButtonRemover_clicked() { remove(); }

void CadastroProduto::on_pushButtonCancelar_clicked() { close(); }

void CadastroProduto::show() {
  QWidget::show();
  adjustSize();
}
