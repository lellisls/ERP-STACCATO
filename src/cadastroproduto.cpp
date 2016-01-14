#include <QMessageBox>

#include "cadastrofornecedor.h"
#include "cadastroproduto.h"
#include "ui_cadastroproduto.h"
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

  ui->itemBoxFornecedor->setSearchDialog(SearchDialog::fornecedor(this));

  SearchDialog *sdProd = SearchDialog::produto(this);
  connect(sdProd, &SearchDialog::itemSelected, this, &CadastroProduto::viewRegisterById);
  connect(ui->pushButtonBuscar, &QAbstractButton::clicked, sdProd, &SearchDialog::show);

  ui->itemBoxFornecedor->setRegisterDialog(new CadastroFornecedor(this));

  if (UserSession::tipoUsuario() != "ADMINISTRADOR") ui->pushButtonRemover->setDisabled(true);
}

CadastroProduto::~CadastroProduto() { delete ui; }

void CadastroProduto::clearFields() {
  for (auto const &line : this->findChildren<QLineEdit *>()) {
    line->clear();
  }

  ui->radioButtonDesc->setChecked(false);
  ui->radioButtonLote->setChecked(false);

  ui->dateEditValidade->setDate(QDate(1900, 1, 1));
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

bool CadastroProduto::verifyFields(const bool &isUpdate) {
  if (not isUpdate) {
    QSqlQuery query;
    query.prepare("SELECT * FROM produto WHERE fornecedor = :fornecedor AND codComercial = :codComercial");
    query.bindValue(":fornecedor", ui->itemBoxFornecedor->text());
    query.bindValue(":codComercial", ui->lineEditCodComer->text());

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro verificando se produto já cadastrado!");
      return false;
    }

    if (query.first()) {
      QMessageBox::critical(this, "Erro!", "Código comercial já cadastrado!");
      return false;
    }
  }

  return true;
}

bool CadastroProduto::verifyFields() {
  for (auto const &line : ui->frame->findChildren<QLineEdit *>()) {
    if (not verifyRequiredField(line)) return false;
  }

  if (ui->comboBoxUn->currentText().isEmpty()) {
    ui->comboBoxUn->setFocus();
    QMessageBox::critical(this, "Erro!", "Faltou preencher unidade!");
    return false;
  }

  if (ui->dateEditValidade->date().toString("dd-MM-yyyy") == "01-01-1900") {
    ui->dateEditValidade->setFocus();
    QMessageBox::critical(this, "Erro!", "Faltou preencher validade!");
    return false;
  }

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
  addMapping(ui->dateEditValidade, "validade");
  addMapping(ui->doubleSpinBoxKgCx, "kgcx");
}

bool CadastroProduto::savingProcedures() {
  if (not setData("codBarras", ui->lineEditCodBarras->text())) return false;
  if (not setData("codComercial", ui->lineEditCodComer->text())) return false;
  if (not setData("colecao", ui->lineEditColecao->text())) return false;
  if (not setData("comissao", ui->doubleSpinBoxComissao->value())) return false;
  if (not setData("cst", ui->comboBoxCST->currentText())) return false;
  if (not setData("custo", ui->doubleSpinBoxCusto->value())) return false;
  if (not setData("descontinuado", ui->radioButtonDesc->isChecked())) return false;
  if (not setData("descricao", ui->lineEditDescricao->text())) return false;
  if (not setData("estoque", ui->doubleSpinBoxEstoque->value())) return false;
  if (not setData("formComercial", ui->lineEditFormComer->text())) return false;
  if (not setData("Fornecedor", ui->itemBoxFornecedor->text())) return false;
  if (not setData("icms", ui->lineEditICMS->text())) return false;
  if (not setData("idFornecedor", ui->itemBoxFornecedor->value())) return false;
  if (not setData("ipi", ui->doubleSpinBoxIPI->value())) return false;
  if (not setData("kgcx", ui->doubleSpinBoxKgCx->value())) return false;
  if (not setData("m2cx", ui->doubleSpinBoxM2Cx->value())) return false;
  if (not setData("markup", ui->doubleSpinBoxMarkup->value())) return false;
  if (not setData("ncm", ui->lineEditNCM->text())) return false;
  if (not setData("observacoes", ui->textEditObserv->toPlainText())) return false;
  if (not setData("origem", ui->comboBoxOrigem->currentData())) return false;
  if (not setData("pccx", ui->doubleSpinBoxPcCx->value())) return false;
  if (not setData("precoVenda", ui->doubleSpinBoxVenda->value())) return false;
  if (not setData("qtdPallet", ui->doubleSpinBoxQtePallet->value())) return false;
  if (not setData("st", ui->doubleSpinBoxST->value())) return false;
  if (not setData("temLote", ui->radioButtonLote->isChecked() ? "SIM" : "NÃO")) return false;
  if (not setData("ui", ui->lineEditUI->text().isEmpty() ? "0" : ui->lineEditUI->text())) return false;
  if (not setData("un", ui->comboBoxUn->currentText())) return false;
  if (not setData("validade", ui->dateEditValidade->date().toString("yyyy-MM-dd"))) return false;

  return true;
}

void CadastroProduto::on_pushButtonCadastrar_clicked() { save(); }

void CadastroProduto::on_pushButtonAtualizar_clicked() { update(); }

void CadastroProduto::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroProduto::on_pushButtonRemover_clicked() { remove(); }

void CadastroProduto::on_doubleSpinBoxVenda_valueChanged(const double &) { calcularMarkup(); }

void CadastroProduto::on_doubleSpinBoxCusto_valueChanged(const double &) { calcularMarkup(); }

void CadastroProduto::calcularMarkup() {
  double markup = ((ui->doubleSpinBoxVenda->value() / ui->doubleSpinBoxCusto->value()) - 1.) * 100.;
  ui->doubleSpinBoxMarkup->setValue(markup);
}

bool CadastroProduto::save(const bool &isUpdate) {
  verifyFields(isUpdate);

  return RegisterDialog::save(isUpdate);
}

// NOTE: alguma coisa deixando lento (aparentemente a validade)
