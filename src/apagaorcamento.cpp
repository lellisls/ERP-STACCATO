#include <QMessageBox>
#include <QDebug>
#include <QSqlError>

#include "apagaorcamento.h"
#include "ui_apagaorcamento.h"

ApagaOrcamento::ApagaOrcamento(QWidget *parent) : QDialog(parent), ui(new Ui::ApagaOrcamento) {
  ui->setupUi(this);

  setAttribute(Qt::WA_DeleteOnClose);

  modelOrc.setTable("orcamento");
  modelOrc.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelOrc.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela de orçamentos: " + modelOrc.lastError().text());
  }

  show();
}

ApagaOrcamento::~ApagaOrcamento() { delete ui; }

void ApagaOrcamento::on_pushButtonSalvar_clicked() {
  if (ui->lineEditMotivo->text().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Deve preencher o motivo");
    return;
  }

  modelOrc.setData(row, "status", "CANCELADO");
  modelOrc.setData(row, "motivoCancelamento", ui->lineEditMotivo->text());

  if (not modelOrc.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro cancelando orçamento: " + modelOrc.lastError().text());
    return;
  }

  parentWidget()->close();
}

void ApagaOrcamento::on_pushButtonCancelar_clicked() { close(); }

void ApagaOrcamento::apagar(const int &index) { row = index; }

// TODO: colocar opcoes pré-definidas para motivos de cancelamento
