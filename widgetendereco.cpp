#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include "cepcompleter.h"
#include "widgetendereco.h"
#include "ui_widgetendereco.h"

WidgetEndereco::WidgetEndereco(QWidget *parent)
  : QWidget(parent), ui(new Ui::WidgetEndereco), id(-1) {
  ui->setupUi(this);
  ui->lineEditCEP->setInputMask("99999-999;_");
  ui->lineEditUF->setInputMask(">AA;_");
}

WidgetEndereco::~WidgetEndereco() { delete ui; }

bool WidgetEndereco::verifyField(QLineEdit *lineEdit, bool silent) {
  if (lineEdit->text().isEmpty()) {
    if (not silent) {
      lineEdit->setFocus();
      QMessageBox::warning(this, "Atenção!", "Você não preencheu um campo obrigatório!", QMessageBox::Ok,
                           QMessageBox::NoButton);
    }
    return false;
  }

  return true;
}

bool WidgetEndereco::verifyFields(bool silent) {
  if (not isEnabled()) {
    return true;
  }

  if (not ui->lineEditCEP->isValid()) {
    if (not silent) {
      ui->lineEditCEP->setFocus();
      QMessageBox::warning(this, "Atenção!", "CEP inválido!", QMessageBox::Ok, QMessageBox::NoButton);
    }
    return false;
  }

  foreach (QLineEdit *line, this->findChildren<QLineEdit *>()) {
    if (line->styleSheet() == requiredStyle()) {
      if (not verifyField(line)) {
        return false;
      }
    }
  }

  return true;
}

void WidgetEndereco::novoCadastro() {
  clearFields();
  setEnabled(true);
}

void WidgetEndereco::clearFields() {
  id = -1;
  ui->lineEditBairro->clear();
  ui->lineEditCEP->clear();
  ui->lineEditCidade->clear();
  ui->lineEditComp->clear();
  ui->lineEditDescricao->clear();
  ui->lineEditEndereco->clear();
  ui->lineEditNro->clear();
  ui->lineEditUF->clear();
}

bool WidgetEndereco::cadastrar() {
  if (not isEnabled()) {
    return true;
  }
  if (not verifyFields()) {
    return false;
  }
  if (table.isEmpty()) {
    qDebug() << "a tabela do widget endereco não foi setada!";
    return false;
  }

  QString str;
  if (id == -1) {
    str = "INSERT INTO " + table + " (descricao, CEP, logradouro, numero, complemento, bairro, cidade, uf)"
                                   "VALUES ( ?, ?, ?, ?, ?, ?, ?, ?);";
  } else {
    str = "UPDATE " + table +
          " SET descricao = ?, CEP = ?, logradouro = ?, numero = ?, complemento = ?, bairro = "
          "?, cidade = ?, uf = ? WHERE idEndereco = " +
          QString::number(id) + ";";
  }
  qDebug() << "qry end: " << str;
  QSqlQuery qry(str);
  qry.addBindValue(ui->lineEditDescricao->text());
  qry.addBindValue(ui->lineEditCEP->text());
  qry.addBindValue(ui->lineEditEndereco->text());
  qry.addBindValue(ui->lineEditNro->text());
  qry.addBindValue(ui->lineEditComp->text());
  qry.addBindValue(ui->lineEditBairro->text());
  qry.addBindValue(ui->lineEditCidade->text());
  qry.addBindValue(ui->lineEditUF->text());
  bool ret = qry.exec();
  if (ret) {
    if (id == -1)
      id = qry.lastInsertId().toInt();
  } else {
    qDebug() << objectName() << " : " << qry.lastError();
    QMessageBox::warning(this, "Atenção!", "Não foi possível cadastrar este endereço!.", QMessageBox::Ok,
                         QMessageBox::NoButton);
  }
  return ret;
}

void WidgetEndereco::remove(int id) {
  QString str = "DELETE FROM " + table + " WHERE idEndereco = '" + QString::number(id) + "';";
  QSqlQuery qry(str);
  qry.exec();
}

bool WidgetEndereco::viewCadastro(int id) {
  setEnabled(true);
  clearFields();
  this->id = id;
  QString str = "SELECT * FROM " + table + " WHERE idEndereco = '" + QString::number(id) + "'";
  QSqlQuery qry(str);
  if (not qry.exec() or not qry.next()) {
    return false;
  }
  ui->lineEditDescricao->setText(qry.value("descricao").toString());
  ui->lineEditCEP->setText(qry.value("CEP").toString());
  ui->lineEditEndereco->setText(qry.value("logradouro").toString());
  ui->lineEditNro->setText(qry.value("numero").toString());
  ui->lineEditComp->setText(qry.value("complemento").toString());
  ui->lineEditBairro->setText(qry.value("bairro").toString());
  ui->lineEditCidade->setText(qry.value("cidade").toString());
  ui->lineEditUF->setText(qry.value("uf").toString());
  return true;
}

int WidgetEndereco::getId() const { return id; }

void WidgetEndereco::setId(int value) { viewCadastro(value); }

void WidgetEndereco::setupUi(QWidget *first, QWidget *last) {
  QWidget::setTabOrder(first, ui->lineEditDescricao);
  QWidget::setTabOrder(ui->lineEditDescricao, ui->lineEditCEP);
  QWidget::setTabOrder(ui->lineEditCEP, ui->lineEditEndereco);
  QWidget::setTabOrder(ui->lineEditEndereco, ui->lineEditNro);
  QWidget::setTabOrder(ui->lineEditNro, ui->lineEditComp);
  QWidget::setTabOrder(ui->lineEditComp, ui->lineEditBairro);
  QWidget::setTabOrder(ui->lineEditBairro, ui->lineEditCidade);
  QWidget::setTabOrder(ui->lineEditCidade, ui->lineEditUF);
  QWidget::setTabOrder(ui->lineEditUF, last);
}

QString WidgetEndereco::requiredStyle() { return (QString("background-color: rgb(255, 255, 127);")); }

void WidgetEndereco::on_lineEditCEP_textEdited(const QString &cep) {
  if (not ui->lineEditCEP->isValid()) {
    return;
  }
  CepCompleter cc;
  if (cc.buscaCEP(cep)) {
    ui->lineEditUF->setText(cc.getUf());
    ui->lineEditCidade->setText(cc.getCidade());
    ui->lineEditEndereco->setText(cc.getEndereco());
    ui->lineEditBairro->setText(cc.getBairro());
  } else {
    QMessageBox::warning(this, "Aviso!", "CEP não encontrado!", QMessageBox::Ok);
  }
}

QString WidgetEndereco::getTable() const { return table; }

void WidgetEndereco::setTable(const QString &value) { table = value; }

bool WidgetEndereco::isEmpty() {
  if (ui->lineEditBairro->text().isEmpty() and ui->lineEditCEP->text().remove("-").isEmpty() and
      ui->lineEditCidade->text().isEmpty() and ui->lineEditComp->text().isEmpty() and
      ui->lineEditDescricao->text().isEmpty() and ui->lineEditEndereco->text().isEmpty() and
      ui->lineEditNro->text().isEmpty() and ui->lineEditUF->text().isEmpty()) {
    return true;
  } else {
    return false;
  }
  // qDebug() << "Bairro: " << ui->lineEditBairro->text();
  // qDebug() << "CEP: " << ui->lineEditCEP->text().remove("-");
  // qDebug() << "Cidade: " << ui->lineEditCidade->text();
  // qDebug() << "Comp: " << ui->lineEditComp->text();
  // qDebug() << "Descricao: " << ui->lineEditDescricao->text();
  // qDebug() << "Endereco: " << ui->lineEditEndereco->text();
  // qDebug() << "Nro: " << ui->lineEditNro->text();
  // qDebug() << "UF: " << ui->lineEditUF->text();
  // return true;
}
