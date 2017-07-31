#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "doubledelegate.h"
#include "inputdialog.h"
#include "noeditdelegate.h"
#include "reaisdelegate.h"
#include "singleeditdelegate.h"
#include "ui_inputdialog.h"
#include "usersession.h"

InputDialog::InputDialog(const Type &type, QWidget *parent) : QDialog(parent), type(type), ui(new Ui::InputDialog) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  ui->dateEditEvento->setDate(QDate::currentDate());
  ui->dateEditProximo->setDate(QDate::currentDate());

  if (type == Carrinho) {
    ui->labelEvento->hide();
    ui->dateEditEvento->hide();
    ui->labelObservacao->hide();
    ui->lineEditObservacao->hide();

    ui->labelProximoEvento->setText("Data prevista compra:");
  }

  if (type == Faturamento) {
    ui->labelEvento->hide();
    ui->dateEditEvento->hide();
    ui->labelObservacao->hide();
    ui->lineEditObservacao->hide();

    ui->labelProximoEvento->setText("Data prevista faturamento:");
  }

  if (type == AgendarColeta) {
    ui->labelEvento->hide();
    ui->dateEditEvento->hide();
    ui->labelProximoEvento->show();
    ui->dateEditProximo->show();
    ui->labelObservacao->hide();
    ui->lineEditObservacao->hide();

    ui->labelProximoEvento->setText("Data prevista coleta");
    ui->dateEditProximo->setDate(QDate::currentDate().addDays(8));
  }

  if (type == Coleta) {
    ui->labelObservacao->hide();
    ui->lineEditObservacao->hide();

    ui->labelEvento->setText("Data coleta:");
    ui->labelProximoEvento->setText("Data prevista recebimento:");
  }

  if (type == AgendarRecebimento) {
    ui->labelObservacao->hide();
    ui->lineEditObservacao->hide();
    ui->labelEvento->hide();
    ui->dateEditEvento->hide();
    ui->labelProximoEvento->show();
    ui->dateEditProximo->show();

    ui->labelProximoEvento->setText("Data prevista recebimento:");
  }

  if (type == AgendarEntrega) {
    ui->labelEvento->hide();
    ui->dateEditEvento->hide();
    ui->labelProximoEvento->show();
    ui->dateEditProximo->show();
    ui->labelObservacao->hide();
    ui->lineEditObservacao->hide();

    ui->labelProximoEvento->setText("Data prevista entrega:");
  }

  if (type == ReagendarPedido) {
    ui->labelEvento->hide();
    ui->dateEditEvento->hide();
    ui->labelProximoEvento->show();
    ui->dateEditProximo->show();
    ui->labelObservacao->show();
    ui->lineEditObservacao->show();

    ui->labelProximoEvento->setText("Data prevista:");
  }

  adjustSize();

  show();
}

InputDialog::~InputDialog() { delete ui; }

QDate InputDialog::getDate() const { return ui->dateEditEvento->date(); }

QDate InputDialog::getNextDate() const { return ui->dateEditProximo->date(); }

QString InputDialog::getObservacao() const { return ui->lineEditObservacao->text(); }

void InputDialog::on_dateEditEvento_dateChanged(const QDate &date) {
  if (ui->dateEditProximo->date() < date) ui->dateEditProximo->setDate(date);
}

void InputDialog::on_pushButtonSalvar_clicked() {
  if (type == ReagendarPedido) {
    if (ui->lineEditObservacao->text().isEmpty()) {
      QMessageBox::critical(this, "Erro!", "Observação não pode estar vazio!");
      return;
    }
  }

  QDialog::accept();
  close();
}

// TODO: colocar titulo, descricao para nao perder o fluxo
