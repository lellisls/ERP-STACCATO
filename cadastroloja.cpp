#include <QDebug>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "cadastroloja.h"
#include "ui_cadastroloja.h"
#include "searchdialog.h"
#include "usersession.h"

CadastroLoja::CadastroLoja(QWidget *parent)
  : RegisterDialog("Loja", "idLoja", parent), ui(new Ui::CadastroLoja) {
  ui->setupUi(this);

  modelAlcadas.setTable("Alcadas");
  modelAlcadas.setEditStrategy(QSqlRelationalTableModel::OnManualSubmit);
  modelAlcadas.setFilter("idLoja = " + QString::number(UserSession::getLoja()) + "");
  if (not modelAlcadas.select()) {
    qDebug() << "Erro carregando alçadas: " << modelAlcadas.lastError();
  }
  ui->tableAlcadas->setModel(&modelAlcadas);
  ui->tableAlcadas->hideColumn(modelAlcadas.fieldIndex("idAlcada"));
  ui->tableAlcadas->hideColumn(modelAlcadas.fieldIndex("idLoja"));
  ui->tableAlcadas->resizeColumnsToContents();

  ui->widgetEnd->setupUi(ui->lineEditTel, ui->pushButtonCadastrar);
  ui->widgetEnd->setTable("Loja_has_Endereco");

  ui->lineEditCNPJ->setInputMask("99.999.999/9999-99;_");
  ui->lineEditSIGLA->setInputMask(">AANN;_");

  setupMapper();
  newRegister();
}

CadastroLoja::~CadastroLoja() { delete ui; }

void CadastroLoja::clearFields() {
  ui->widgetEnd->clearFields();
  foreach (QLineEdit *line, this->findChildren<QLineEdit *>()) { line->clear(); }
}

bool CadastroLoja::verifyFields(int row) {
  Q_UNUSED(row);

  if (not RegisterDialog::verifyFields({ui->lineEditDescricao, ui->lineEditRazaoSocial,
                                       ui->lineEditNomeFantasia, ui->lineEditSIGLA, ui->lineEditCNPJ,
                                       ui->lineEditInscEstadual, ui->lineEditTel})) {
    return false;
  }
  if (ui->widgetEnd->isEnabled() and not ui->widgetEnd->verifyFields()) {
    return false;
  }

  return true;
}

bool CadastroLoja::savingProcedures(int row) {
  if (not ui->widgetEnd->cadastrar()) {
    return false;
  }
  setData(row, "descricao", ui->lineEditDescricao->text());
  setData(row, "razaoSocial", ui->lineEditRazaoSocial->text());
  setData(row, "sigla", ui->lineEditSIGLA->text());
  setData(row, "nomeFantasia", ui->lineEditNomeFantasia->text());
  setData(row, "cnpj", ui->lineEditCNPJ->text());
  setData(row, "inscEstadual", ui->lineEditInscEstadual->text());
  setData(row, "tel", ui->lineEditTel->text());
  setData(row, "idEndereco", ui->widgetEnd->getId());
  setData(row, "valorMinimoFrete", ui->doubleSpinBoxValorMinimoFrete->value());
  setData(row, "porcentagemFrete", ui->doubleSpinBoxPorcFrete->value());
  setData(row, "servidorACBr", ui->lineEditServidorACBr->text());
  setData(row, "portaACBr", ui->lineEditPortaACBr->text().toInt());
  setData(row, "pastaEntACBr", ui->lineEditPastaEntACBr->text());
  setData(row, "pastaSaiACBr", ui->lineEditPastaSaiACBr->text());
  return true;
}

void CadastroLoja::registerMode() {
  ui->pushButtonCadastrar->show();
  ui->pushButtonAtualizar->hide();
  ui->pushButtonRemover->hide();
}

void CadastroLoja::updateMode() {
  ui->pushButtonCadastrar->hide();
  ui->pushButtonAtualizar->show();
  ui->pushButtonRemover->show();
}

void CadastroLoja::setupMapper() {
  addMapping(ui->lineEditDescricao, "descricao");
  addMapping(ui->lineEditCNPJ, "cnpj");
  addMapping(ui->lineEditRazaoSocial, "razaoSocial");
  addMapping(ui->lineEditNomeFantasia, "nomeFantasia");
  addMapping(ui->lineEditInscEstadual, "inscEstadual");
  addMapping(ui->lineEditTel, "tel");
  addMapping(ui->lineEditSIGLA, "sigla");
  addMapping(ui->doubleSpinBoxValorMinimoFrete, "valorMinimoFrete");
  addMapping(ui->doubleSpinBoxPorcFrete, "porcentagemFrete");
  addMapping(ui->lineEditServidorACBr, "servidorACBr");
  addMapping(ui->lineEditPortaACBr, "portaACBr");
  addMapping(ui->lineEditPastaEntACBr, "pastaEntACBr");
  addMapping(ui->lineEditPastaSaiACBr, "pastaSaiACBr");
}

bool CadastroLoja::viewRegister(QModelIndex idx) {
  if (not RegisterDialog::viewRegister(idx)) {
    return false;
  }
  bool ok = false;
  int idEnd = model.data(model.index(idx.row(), model.fieldIndex("idEndereco"))).toInt(&ok);
  if (ok) {
    ui->widgetEnd->viewCadastro(idEnd);
  }
  mapper.setCurrentModelIndex(idx);
  return true;
}

void CadastroLoja::on_pushButtonCadastrar_clicked() { save(); }

void CadastroLoja::on_pushButtonAtualizar_clicked() { save(); }

void CadastroLoja::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroLoja::on_pushButtonRemover_clicked() { remove(); }

void CadastroLoja::on_pushButtonCancelar_clicked() { close(); }

void CadastroLoja::on_pushButtonBuscar_clicked() {
  SearchDialog *sdLoja = SearchDialog::loja(this);
  connect(sdLoja, &SearchDialog::itemSelected, this, &CadastroLoja::changeItem);
  sdLoja->show();
}

void CadastroLoja::changeItem(QVariant value, QString text) {
  Q_UNUSED(text)
  viewRegisterById(value);
}

void CadastroLoja::on_lineEditCNPJ_textEdited(const QString &) {
  QString text = ui->lineEditCNPJ->text().remove(".").remove("/").remove("-");
  validaCNPJ(text);
}

void CadastroLoja::validaCNPJ(QString text) {
  if (text.size() == 14) {

    int digito1;
    int digito2;

    QString sub = text.left(12);

    QVector<int> sub2;
    for (int i = 0; i < sub.size(); ++i) {
      sub2.push_back(sub.at(i).digitValue());
    }

    QVector<int> multiplicadores = {5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2};

    int soma = 0;
    for (int i = 0; i < 12; ++i) {
      soma += sub2.at(i) * multiplicadores.at(i);
    }

    int resto = soma % 11;

    if (resto < 2) {
      digito1 = 0;
    } else {
      digito1 = 11 - resto;
    }

    sub2.push_back(digito1);

    QVector<int> multiplicadores2 = {6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2};
    soma = 0;

    for (int i = 0; i < 13; ++i) {
      soma += sub2.at(i) * multiplicadores2.at(i);
    }

    resto = soma % 11;

    if (resto < 2) {
      digito2 = 0;
    } else {
      digito2 = 11 - resto;
    }

    if (digito1 == text.at(12).digitValue() and digito2 == text.at(13).digitValue()) {
      qDebug() << "Válido!";
    } else {
      QMessageBox::warning(this, "Aviso!", "CNPJ inválido!");
      return;
    }
  }
}

void CadastroLoja::on_pushButtonEntradaNFe_clicked() {
  QString dir = QFileDialog::getExistingDirectory(this, "Pasta entrada NFe", QDir::currentPath());

  if (not dir.isEmpty()) {
    ui->lineEditPastaEntACBr->setText(dir);
  }
}

void CadastroLoja::on_pushButtonSaidaNFe_clicked() {
  QString dir = QFileDialog::getExistingDirectory(this, "Pasta saída NFe", QDir::currentPath());

  if (not dir.isEmpty()) {
    ui->lineEditPastaSaiACBr->setText(dir);
  }
}
