#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

#include "widgetendereco.h"
#include "ui_widgetendereco.h"

WidgetEndereco::WidgetEndereco(QWidget *parent)
  : QWidget(parent), ui(new Ui::WidgetEndereco), id(-1), used(false) {
  ui->setupUi(this);
  ui->lineEditCEP->setInputMask("99999-999;_");
  ui->lineEditUF->setInputMask(">AA;_");
}

WidgetEndereco::~WidgetEndereco() {
  delete ui;
}

bool WidgetEndereco::verifyField(QLineEdit *lineEdit, bool silent) {
  if (lineEdit->text().isEmpty()) {
    if (!silent) {
      lineEdit->setFocus();
      QMessageBox::warning(this, "Atenção!", "Você não preencheu um campo obrigatório!", QMessageBox::Ok,
                           QMessageBox::NoButton);
    }
    return false;
  }
  return true;
}

bool WidgetEndereco::verifyFields(bool silent) {
  if (!isEnabled()) {
    return true;
  }
  if (!ui->lineEditCEP->isValid()) {
    if (!silent) {
      ui->lineEditCEP->setFocus();
      QMessageBox::warning(this, "Atenção!", "CEP inválido!", QMessageBox::Ok, QMessageBox::NoButton);
    }
    return false;
  }
  foreach (QLineEdit *line, this->findChildren<QLineEdit *>()) {
    if (line->styleSheet() == requiredStyle()) {
      if (!verifyField(line)) {
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
  used = false;
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
  if (!isEnabled())
    return true;
  if (!verifyFields()) {
    return false;
  }
  QString str;
  if (id == -1) {
    str = "INSERT INTO Endereco (descricao, CEP, logradouro, numero, complemento, bairro, cidade, uf)"
          "VALUES ( ?, ?, ?, ?, ?, ?, ?, ?);";
  } else {
    str = "UPDATE Endereco SET descricao=?, CEP = ?, logradouro = ?, numero = ?, complemento = ?, bairro = "
          "?, cidade = ?, uf = ? WHERE idEndereco = '" +
          QString::number(id) + "';";
  }
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
  QString str = "DELETE from Endereco WHERE idEndereco = '" + QString::number(id) + "';";
  QSqlQuery qry(str);
  qry.exec();
}

bool WidgetEndereco::viewCadastro(int id) {
  setEnabled(true);
  clearFields();
  this->id = id;
  QString str = "SELECT * FROM Endereco WHERE idEndereco = '" + QString::number(id) + "'";
  QSqlQuery qry(str);
  if (!qry.exec() || !qry.next()) {
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
  used = qry.value("used").toBool();
  return true;
}

int WidgetEndereco::getId() const {
  return id;
}

void WidgetEndereco::setId(int value) {
  viewCadastro(value);
}

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

bool WidgetEndereco::inRange(QString cep, int st, int end) {
  QStringList valueList = cep.split(QChar('-'));
  if (valueList.size() != 2)
    return false;
  int vl = valueList.at(0).toInt();
  return (vl >= st && vl <= end);
}

QString WidgetEndereco::buscaUF(QString cep) {
  if (inRange(cep, 01000, 19999))
    return "sp";
  if (inRange(cep, 69900, 69999))
    return "ac";
  if (inRange(cep, 57000, 57999))
    return "al";
  if (inRange(cep, 69000, 69299))
    return "am";
  if (inRange(cep, 69400, 69899))
    return "am";
  if (inRange(cep, 68900, 68999))
    return "ap";
  if (inRange(cep, 40000, 48999))
    return "ba";
  if (inRange(cep, 60000, 63999))
    return "ce";
  if (inRange(cep, 70000, 72799))
    return "df";
  if (inRange(cep, 73000, 73699))
    return "df";
  if (inRange(cep, 29000, 29999))
    return "es";
  if (inRange(cep, 72800, 72999))
    return "go";
  if (inRange(cep, 73700, 76799))
    return "go";
  if (inRange(cep, 65000, 65999))
    return "ma";
  if (inRange(cep, 30000, 39999))
    return "mg";
  if (inRange(cep, 79000, 79999))
    return "ms";
  if (inRange(cep, 78000, 78899))
    return "mt";
  if (inRange(cep, 66000, 68899))
    return "pa";
  if (inRange(cep, 58000, 58999))
    return "pb";
  if (inRange(cep, 50000, 56999))
    return "pe";
  if (inRange(cep, 64000, 64999))
    return "pi";
  if (inRange(cep, 80000, 87999))
    return "pr";
  if (inRange(cep, 20000, 28999))
    return "rj";
  if (inRange(cep, 59000, 59999))
    return "rn";
  if (inRange(cep, 76800, 76999))
    return "ro";
  if (inRange(cep, 69300, 69399))
    return "rr";
  if (inRange(cep, 90000, 99999))
    return "rs";
  if (inRange(cep, 88000, 89999))
    return "sc";
  if (inRange(cep, 49000, 49999))
    return "se";
  if (inRange(cep, 77000, 77999))
    return "to";
  return QString();
}

QString WidgetEndereco::requiredStyle() {
  return (QString("background-color: rgb(255, 255, 127);"));
}

void WidgetEndereco::on_lineEditCEP_textEdited(const QString &cep) {
  if (!ui->lineEditCEP->isValid()) {
    return;
  }
  QString UF = buscaUF(cep);
  if (UF.isEmpty())
    return;
  ui->lineEditUF->setText(UF.toUpper());
  QSqlQuery query;
  if (!query.exec("SELECT * FROM cep." + UF + " WHERE cep = '" + cep + "' LIMIT 1")) {
    qDebug() << "Erro buscando pelo cep: " << query.lastError();
    qDebug() << "Query: " << query.lastQuery();
  }
  if (query.size() != 0 and query.next()) {
    ui->lineEditCidade->setText(query.value("cidade").toString());
    QString endereco = query.value("tp_logradouro").toString() + " " + query.value("logradouro").toString();
    ui->lineEditEndereco->setText(endereco);
    ui->lineEditBairro->setText(query.value("bairro").toString());
  } else {
    QMessageBox::warning(this, "Aviso!", "CEP não encontrado!", QMessageBox::Ok);
  }
}

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
