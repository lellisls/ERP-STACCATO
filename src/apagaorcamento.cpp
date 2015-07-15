#include <QMessageBox>
#include <QDebug>
#include <QSqlError>

#include "apagaorcamento.h"
#include "ui_apagaorcamento.h"

ApagaOrcamento::ApagaOrcamento(QWidget *parent) : QDialog(parent), ui(new Ui::ApagaOrcamento) {
  ui->setupUi(this);

  modelOrc.setTable("Orcamento");
  modelOrc.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelOrc.select()) {
    qDebug() << "erro modelOrc: " << modelOrc.lastError();
    return;
  }

  mapperOrc.setModel(&modelOrc);
  mapperOrc.addMapping(ui->lineEditMotivo, modelOrc.fieldIndex("motivoCancelamento"));

  show();
}

ApagaOrcamento::~ApagaOrcamento() { delete ui; }

void ApagaOrcamento::on_pushButtonSalvar_clicked() {
  if (ui->lineEditMotivo->text().isEmpty()) {
    QMessageBox::warning(this, "Aviso!", "Deve preencher o motivo");
    return;
  }

  modelOrc.setData(modelOrc.index(mapperOrc.currentIndex(), modelOrc.fieldIndex("status")), "CANCELADO");
  modelOrc.setData(modelOrc.index(mapperOrc.currentIndex(), modelOrc.fieldIndex("motivoCancelamento")),
                   ui->lineEditMotivo->text());

  if (not modelOrc.submitAll()) {
    qDebug() << "Erro cancelando orçamento: " << modelOrc.lastError();
  }

  parentWidget()->close();
  close();
}

void ApagaOrcamento::on_pushButtonCancelar_clicked() { close(); }

void ApagaOrcamento::apagar(const int index) { mapperOrc.setCurrentIndex(index); }
