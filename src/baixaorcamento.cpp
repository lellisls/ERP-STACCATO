#include <QMessageBox>
#include <QSqlError>

#include "baixaorcamento.h"
#include "ui_baixaorcamento.h"

BaixaOrcamento::BaixaOrcamento(const QString &idOrcamento, QWidget *parent)
    : QDialog(parent), ui(new Ui::BaixaOrcamento) {
  ui->setupUi(this);

  model.setTable("orcamento");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);
  model.setFilter("idOrcamento = '" + idOrcamento + "'");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela orcamento: " + model.lastError().text());
  }
}

BaixaOrcamento::~BaixaOrcamento() { delete ui; }

void BaixaOrcamento::on_pushButtonCancelar_clicked() { close(); }

void BaixaOrcamento::on_pushButtonSalvar_clicked() {
  if (ui->plainTextEditObservacao->toPlainText().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Deve preencher a observação!");
    return;
  }

  QString motivo;

  for (auto const &child : ui->groupBox->findChildren<QRadioButton *>()) {
    if (child->isChecked()) motivo = child->text();
  }

  if (motivo.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Deve escolher um motivo!");
    return;
  }

  model.setData(0, "status", "PERDIDO");
  model.setData(0, "motivoCancelamento", motivo);
  model.setData(0, "observacaoCancelamento", ui->plainTextEditObservacao->toPlainText());

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro cancelando orçamento: " + model.lastError().text());
    return;
  }

  parentWidget()->close();
  close();
}
