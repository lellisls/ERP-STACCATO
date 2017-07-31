#include "validadedialog.h"
#include "ui_validadedialog.h"

ValidadeDialog::ValidadeDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ValidadeDialog) {
  ui->setupUi(this);

  ui->dateEdit->setDate(QDate::currentDate());
}

ValidadeDialog::~ValidadeDialog() { delete ui; }

void ValidadeDialog::on_pushButtonSalvar_clicked() {
  QDialog::accept();
  close();
}

void ValidadeDialog::on_spinBox_valueChanged(const int dias) { ui->dateEdit->setDate(QDate::currentDate().addDays(dias)); }

void ValidadeDialog::on_dateEdit_dateChanged(const QDate &date) { ui->spinBox->setValue(QDate::currentDate().daysTo(date)); }

int ValidadeDialog::getValidade() { return ui->spinBox->value(); }
