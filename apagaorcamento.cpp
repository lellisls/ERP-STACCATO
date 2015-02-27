#include <QSqlError>
#include <QMessageBox>
#include <QDateTime>

#include "apagaorcamento.h"
#include "ui_apagaorcamento.h"

ApagaOrcamento::ApagaOrcamento(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ApagaOrcamento)
{
  ui->setupUi(this);

  modelOrc.setTable("Orcamento");
  modelOrc.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelOrc.select();

  mapperOrc.setModel(&modelOrc);
  mapperOrc.addMapping(ui->lineEditMotivo, modelOrc.fieldIndex("motivoCancelamento"));

  show();
}

ApagaOrcamento::~ApagaOrcamento()
{
  delete ui;
}

void ApagaOrcamento::on_pushButtonSalvar_clicked()
{
  //submit model
  if(ui->lineEditMotivo->text().isEmpty()){
    QMessageBox::warning(this, "Aviso!", "Deve preencher o motivo");
    return;
  }

  modelOrc.setData(modelOrc.index(mapperOrc.currentIndex(), modelOrc.fieldIndex("status")), "Cancelado");
  modelOrc.setData(modelOrc.index(mapperOrc.currentIndex(), modelOrc.fieldIndex("motivoCancelamento")), ui->lineEditMotivo->text());
  if(!modelOrc.submitAll()){
    qDebug() << "Erro cancelando orçamento: " << modelOrc.lastError();
  }

  close();
}

void ApagaOrcamento::on_pushButtonCancelar_clicked()
{
  close();
}

void ApagaOrcamento::apagar(int index){
  mapperOrc.setCurrentIndex(index);
}
