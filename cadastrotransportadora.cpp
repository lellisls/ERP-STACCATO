#include <QDebug>
#include <QDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "cadastrotransportadora.h"
#include "ui_cadastrotransportadora.h"
#include "searchdialog.h"

CadastroTransportadora::CadastroTransportadora(QWidget *parent):
  RegisterDialog("Transportadora", "idTransportadora", parent),
  ui(new Ui::CadastroTransportadora) {
  ui->setupUi(this);
  ui->widgetEnd->setupUi(ui->lineEditPlaca, ui->pushButtonCadastrar);

  ui->lineEditCNPJ->setInputMask("99.999.999/9999-99;_");
  ui->lineEditANTT->setInputMask("99999999;_");
  ui->lineEditPlaca->setInputMask("AAA-9999;_");

  setupMapper();
  newRegister();
}

CadastroTransportadora::~CadastroTransportadora() {
  delete ui;
}

void CadastroTransportadora::clearFields() {
  RegisterDialog::clearFields();
}

bool CadastroTransportadora::verifyFields() {
  //  if(!RegisterDialog::verifyFields({ui}))
  if (ui->widgetEnd->isEnabled() && !ui->widgetEnd->verifyFields()) {
    return false;
  }
  return true;
}

bool CadastroTransportadora::savingProcedures(int row) {
  if(!ui->widgetEnd->cadastrar()) {
    return false;
  }
  setData(row, "cnpj", ui->lineEditCNPJ->text());
  setData(row, "razaoSocial", ui->lineEditRazaoSocial->text());
  setData(row, "nomeFantasia", ui->lineEditNomeFantasia->text());
  setData(row, "inscEstadual", ui->lineEditInscEstadual->text());
  setData(row, "tel", ui->lineEditTel->text());
  setData(row, "antt", ui->lineEditANTT->text());
  setData(row, "placaVeiculo", ui->lineEditPlaca->text());
  setData(row, "idEndereco", ui->widgetEnd->getId());
  return true;
}

void CadastroTransportadora::setupMapper() {
  mapper.addMapping(ui->lineEditCNPJ, model.fieldIndex("cnpj"));
  mapper.addMapping(ui->lineEditRazaoSocial, model.fieldIndex("razaoSocial"));
  mapper.addMapping(ui->lineEditNomeFantasia, model.fieldIndex("nomeFantasia"));
  mapper.addMapping(ui->lineEditInscEstadual, model.fieldIndex("inscEstadual"));
  mapper.addMapping(ui->lineEditTel, model.fieldIndex("tel"));
  mapper.addMapping(ui->lineEditANTT, model.fieldIndex("antt"));
  mapper.addMapping(ui->lineEditPlaca, model.fieldIndex("placaVeiculo"));
}

void CadastroTransportadora::registerMode() {
  ui->pushButtonCadastrar->show();
  ui->pushButtonAtualizar->hide();
  ui->pushButtonRemover->hide();
}

void CadastroTransportadora::updateMode() {
  ui->pushButtonCadastrar->hide();
  ui->pushButtonAtualizar->show();
  ui->pushButtonRemover->show();
}

bool CadastroTransportadora::viewRegister(QModelIndex idx) {
  if(!RegisterDialog::viewRegister(idx)) {
    return false;
  }
  bool ok = false;
  int idEnd = model.data(model.index(idx.row(), model.fieldIndex("idEndereco"))).toInt(&ok);
  if (ok) ui->widgetEnd->viewCadastro(idEnd);
  mapper.setCurrentModelIndex(idx);
  return true;
}

void CadastroTransportadora::on_pushButtonCadastrar_clicked() {
  save();
}

void CadastroTransportadora::on_pushButtonAtualizar_clicked() {
  save();
}

void CadastroTransportadora::on_pushButtonNovoCad_clicked() {
  newRegister();
}

void CadastroTransportadora::on_pushButtonRemover_clicked() {
  remove();
}

void CadastroTransportadora::on_pushButtonCancelar_clicked() {
  cancel();
}

void CadastroTransportadora::on_pushButtonBuscar_clicked() {
  SearchDialog *sdTransportadora = SearchDialog::transportadora(this);
  sdTransportadora->show();
}

void CadastroTransportadora::validaCNPJ(QString text) {
  if(text.size() == 14) {

    int digito1;
    int digito2;

    QString sub = text.left(12);

    QVector<int> sub2;
    for(int i = 0; i < sub.size(); ++i) {
      sub2.push_back(sub.at(i).digitValue());
    }

    QVector<int> multiplicadores = {5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2};

    int soma = 0;
    for(int i = 0; i < 12; ++i) {
      soma += sub2.at(i) * multiplicadores.at(i);
    }

    int resto = soma % 11;

    if(resto < 2) {
      digito1 = 0;
    } else {
      digito1 = 11 - resto;
    }

    sub2.push_back(digito1);

    QVector<int> multiplicadores2 = {6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2};
    soma = 0;

    for(int i = 0; i < 13; ++i) {
      soma += sub2.at(i) * multiplicadores2.at(i);
    }

    resto = soma % 11;

    if(resto < 2) {
      digito2 = 0;
    } else {
      digito2 = 11 - resto;
    }

    if(digito1 == text.at(12).digitValue() and digito2 == text.at(13).digitValue()) {
      qDebug() << "Válido!";
    } else {
      QMessageBox::warning(this, "Aviso!", "CNPJ inválido!");
      return;
    }
  }
}

void CadastroTransportadora::remove() {
  QMessageBox msgBox(QMessageBox::Warning, "Atenção!", "Tem certeza que deseja remover?", QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Sim");
  msgBox.setButtonText(QMessageBox::No, "Não");
  if(msgBox.exec() == QMessageBox::Yes) {
    qDebug() << "Yes!";
    if (model.removeRow(mapper.currentIndex()) && model.submitAll()) {
      qDebug() << "REMOVING " << mapper.currentIndex();
      model.select();
      newRegister();
    } else {
      QMessageBox::warning(this, "Atenção!", "Não foi possível remover este item.", QMessageBox::Ok,
                           QMessageBox::NoButton);
      qDebug() << model.lastError();
    }
  }
}

bool CadastroTransportadora::newRegister() {
  if(!RegisterDialog::newRegister()) {
    return false;
  }
  novoItem();
  return true;
}

void CadastroTransportadora::novoItem() {
  ui->pushButtonAtualizar->hide();
  ui->pushButtonRemover->hide();
}

void CadastroTransportadora::on_lineEditCNPJ_textEdited(const QString &) {
  QString text = ui->lineEditCNPJ->text().remove(".").remove("/").remove("-");
  validaCNPJ(text);
}
