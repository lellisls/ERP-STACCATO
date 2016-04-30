#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

#include "apagaorcamento.h"
#include "ui_apagaorcamento.h"

ApagaOrcamento::ApagaOrcamento(QWidget *parent) : QDialog(parent), ui(new Ui::ApagaOrcamento) {
  ui->setupUi(this);

  setAttribute(Qt::WA_DeleteOnClose);

  model.setTable("orcamento");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela de orçamentos: " + model.lastError().text());
  }

  show();
}

ApagaOrcamento::~ApagaOrcamento() { delete ui; }

void ApagaOrcamento::on_pushButtonSalvar_clicked() {
  if (ui->plainTextEdit->toPlainText().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Deve preencher o motivo");
    return;
  }

  QString motivo;

  for (const auto &child : ui->groupBox->findChildren<QRadioButton *>()) {
    if (child->isChecked()) motivo = child->text();
  }

  model.setData(row, "status", "PERDIDO");
  model.setData(row, "motivoCancelamento", motivo);
  model.setData(row, "observacaoCancelamento", ui->plainTextEdit->toPlainText());

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro cancelando orçamento: " + model.lastError().text());
    return;
  }

  parentWidget()->close();
  close();
}

void ApagaOrcamento::on_pushButtonCancelar_clicked() { close(); }

void ApagaOrcamento::apagar(const int &index) { row = index; }
