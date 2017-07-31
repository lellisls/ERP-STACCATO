#include <QMessageBox>
#include <QSqlError>

#include "followup.h"
#include "followupproxymodel.h"
#include "ui_followup.h"
#include "usersession.h"

FollowUp::FollowUp(const QString &id, const Tipo tipo, QWidget *parent) : QDialog(parent), id(id), tipo(tipo), ui(new Ui::FollowUp) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables();

  setWindowTitle((tipo == Orcamento ? "Orçamento: " : "Pedido: ") + id);

  ui->dateFollowup->setDateTime(QDateTime::currentDateTime());
  ui->dateProxFollowup->setDateTime(QDateTime::currentDateTime().addDays(1));

  if (tipo == Venda) ui->frameOrcamento->hide();
}

FollowUp::~FollowUp() { delete ui; }

void FollowUp::on_pushButtonCancelar_clicked() { close(); }

void FollowUp::on_pushButtonSalvar_clicked() {
  if (not verifyFields()) return;

  QSqlQuery query;
  if (tipo == Orcamento) {
    query.prepare("INSERT INTO orcamento_has_followup (idOrcamento, idLoja, idUsuario, semaforo, observacao, "
                  "dataFollowup, dataProxFollowup) VALUES (:idOrcamento, :idLoja, :idUsuario, :semaforo, :observacao, "
                  ":dataFollowup, :dataProxFollowup)");
    query.bindValue(":idOrcamento", id);
    query.bindValue(":idLoja", UserSession::idLoja());
    query.bindValue(":idUsuario", UserSession::idUsuario());
    query.bindValue(":semaforo", ui->radioButtonQuente->isChecked() ? 1 : ui->radioButtonMorno->isChecked() ? 2 : ui->radioButtonFrio->isChecked() ? 3 : 0);
    query.bindValue(":observacao", ui->plainTextEdit->toPlainText());
    query.bindValue(":dataFollowup", ui->dateFollowup->dateTime());
    query.bindValue(":dataProxFollowup", ui->dateProxFollowup->dateTime());
  }

  if (tipo == Venda) {
    query.prepare("INSERT INTO venda_has_followup (idVenda, idLoja, idUsuario, observacao, dataFollowup) VALUES "
                  "(:idVenda, :idLoja, :idUsuario, :observacao, :dataFollowup)");
    query.bindValue(":idVenda", id);
    query.bindValue(":idLoja", UserSession::idLoja());
    query.bindValue(":idUsuario", UserSession::idUsuario());
    query.bindValue(":observacao", ui->plainTextEdit->toPlainText());
    query.bindValue(":dataFollowup", ui->dateFollowup->dateTime());
  }

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando followup: " + query.lastError().text());
    return;
  }

  QMessageBox::information(this, "Aviso!", "Followup salvo com sucesso!");
  close();
}

bool FollowUp::verifyFields() {
  if (tipo == Orcamento and not ui->radioButtonQuente->isChecked() and not ui->radioButtonMorno->isChecked() and not ui->radioButtonFrio->isChecked()) {
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
  model.setTable("view_followup_" + QString(tipo == Orcamento ? "orcamento" : "venda"));
  model.setEditStrategy(SqlTableModel::OnManualSubmit);

  model.setHeaderData("idOrcamento", "Orçamento");
  model.setHeaderData("idVenda", "Venda");
  model.setHeaderData("nome", "Usuário");
  model.setHeaderData("observacao", "Observação");
  model.setHeaderData("dataFollowup", "Data");
  model.setHeaderData("dataProxFollowup", "Próx. Data");

  model.setFilter(tipo == Orcamento ? "idOrcamento LIKE '" + id.left(12) + "%'" : "idVenda LIKE '" + id.left(11) + "%'");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela followup: " + model.lastError().text());
    return;
  }

  //  if (tipo == Orcamento) {
  //    ui->table->setModel(new FollowUpProxyModel(&model, this));
  //  } else {
  //    ui->table->setModel(new Esto);
  //  }

  ui->table->setModel(new FollowUpProxyModel(&model, this));
  ui->table->hideColumn("semaforo");

  ui->table->resizeColumnsToContents();
}

void FollowUp::on_dateFollowup_dateChanged(const QDate &date) {
  if (ui->dateProxFollowup->date() < date) ui->dateProxFollowup->setDate(date);
}
