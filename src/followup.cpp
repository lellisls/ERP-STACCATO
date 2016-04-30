#include <QMessageBox>
#include <QSqlError>

#include "followup.h"
#include "followupproxy.h"
#include "ui_followup.h"
#include "usersession.h"

FollowUp::FollowUp(QString idOrcamento, QWidget *parent)
    : QDialog(parent), ui(new Ui::FollowUp), idOrcamento(idOrcamento) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables();

  ui->dateFollowup->setDateTime(QDateTime::currentDateTime());
  ui->dateProxFollowup->setDateTime(QDateTime::currentDateTime().addDays(1));
}

FollowUp::~FollowUp() { delete ui; }

void FollowUp::on_pushButtonCancelar_clicked() { close(); }

void FollowUp::on_pushButtonSalvar_clicked() {
  if (not verifyFields()) return;

  row = model.rowCount();
  model.insertRow(row);
  // saving procedures
  if (not savingProcedures()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro salvando dados na tabela orcamento_has_followup: " + model.lastError().text());
    return;
  }

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando tabela orcamento_has_followup: " + model.lastError().text());
    return;
  }

  QMessageBox::information(this, "Aviso!", "Followup salvo com sucesso!");
  close();
}

bool FollowUp::verifyFields() {
  if (not ui->radioButtonQuente->isChecked() and not ui->radioButtonMorno->isChecked() and
      not ui->radioButtonFrio->isChecked()) {
    QMessageBox::critical(this, "Erro!", "Deve selecionar uma temperatura!");
    return false;
  }

  if (ui->plainTextEdit->toPlainText().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Deve escrever uma observação!");
    return false;
  }

  return true;
}

void FollowUp::setupTables() {
  model.setTable("orcamento_has_followup");
  model.setEditStrategy(SqlTableModel::OnManualSubmit);

  model.setHeaderData("idOrcamento", "Orçamento");
  model.setHeaderData("observacao", "Observação");
  model.setHeaderData("dataFollowup", "Data");
  model.setHeaderData("dataProxFollowup", "Próx. Data");

  model.setFilter("idOrcamento LIKE '" + idOrcamento.left(12) + "%'");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela orcamento_has_followup: " + model.lastError().text());
    return;
  }

  ui->table->setModel(new FollowUpProxy(&model, this));
  ui->table->hideColumn("idFollowup");
  ui->table->hideColumn("idLoja");
  ui->table->hideColumn("idUsuario");
  ui->table->hideColumn("semaforo");
  ui->table->hideColumn("created");
  ui->table->hideColumn("lastUpdated");

  ui->table->resizeColumnsToContents();
}

bool FollowUp::savingProcedures() {
  if (not model.setData(row, "idOrcamento", idOrcamento)) return false;
  if (not model.setData(row, "idLoja", UserSession::idLoja())) return false;
  if (not model.setData(row, "idUsuario", UserSession::idUsuario())) return false;
  int semaforo = ui->radioButtonQuente->isChecked() ? 1 : ui->radioButtonMorno->isChecked()
                                                              ? 2
                                                              : ui->radioButtonFrio->isChecked() ? 3 : 0;
  if (not model.setData(row, "semaforo", semaforo)) return false;
  if (not model.setData(row, "observacao", ui->plainTextEdit->toPlainText())) return false;
  if (not model.setData(row, "dataFollowup", ui->dateFollowup->dateTime())) return false;
  if (not model.setData(row, "dataProxFollowup", ui->dateProxFollowup->dateTime())) return false;

  return true;
}

// TODO: ao incrementar data de followup subir data proximo junto
