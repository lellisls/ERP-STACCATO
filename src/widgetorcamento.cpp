#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

#include "widgetorcamento.h"
#include "ui_widgetorcamento.h"
#include "usersession.h"
#include "doubledelegate.h"
#include "orcamentoproxymodel.h"
#include "orcamento.h"

WidgetOrcamento::WidgetOrcamento(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetOrcamento) {
  ui->setupUi(this);

  setupTables();

  ui->radioButtonOrcLimpar->click();

  if (UserSession::getTipoUsuario() == "VENDEDOR") {
    ui->radioButtonOrcProprios->click();
    ui->radioButtonOrcValido->setChecked(true);
    on_radioButtonOrcValido_clicked();
  }
}

WidgetOrcamento::~WidgetOrcamento() { delete ui; }

void WidgetOrcamento::setupTables() {
  DoubleDelegate *doubledelegate = new DoubleDelegate(this);

  modelOrcamento = new SqlTableModel(this);
  modelOrcamento->setTable("view_orcamento");

  ui->tableOrcamentos->setModel(new OrcamentoProxyModel(modelOrcamento, "Dias restantes", this));
  ui->tableOrcamentos->setItemDelegate(doubledelegate);
  ui->tableOrcamentos->sortByColumn("Código");
}

bool WidgetOrcamento::updateTables() {
  if (not modelOrcamento->select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela orçamento: " + modelOrcamento->lastError().text());
    return false;
  }

  ui->tableOrcamentos->resizeColumnsToContents();

  return true;
}

void WidgetOrcamento::on_radioButtonOrcValido_clicked() {
  modelOrcamento->setFilter("(Código LIKE '%" + UserSession::getSiglaLoja() +
                            "%') AND `Dias restantes` > 0 AND status != 'CANCELADO'");
  ui->tableOrcamentos->resizeColumnsToContents();
}

void WidgetOrcamento::on_radioButtonOrcExpirado_clicked() {
  modelOrcamento->setFilter("(Código LIKE '%" + UserSession::getSiglaLoja() + "%') AND `Dias restantes` < 1");
  ui->tableOrcamentos->resizeColumnsToContents();
}

void WidgetOrcamento::on_radioButtonOrcLimpar_clicked() {
  modelOrcamento->setFilter("(Código LIKE '%" + UserSession::getSiglaLoja() + "%')");
  ui->tableOrcamentos->resizeColumnsToContents();
}

void WidgetOrcamento::on_radioButtonOrcProprios_clicked() {
  modelOrcamento->setFilter("(Código LIKE '%" + UserSession::getSiglaLoja() + "%') AND Vendedor = '" +
                            UserSession::getNome() + "'");
  ui->tableOrcamentos->resizeColumnsToContents();
}

void WidgetOrcamento::on_lineEditBuscaOrcamentos_textChanged(const QString &text) {
  modelOrcamento->setFilter("(Código LIKE '%" + UserSession::getSiglaLoja() + "%')" +
                            (text.isEmpty() ? "" : " AND ((Código LIKE '%" + text + "%') OR (Vendedor LIKE '%" + text +
                                              "%') OR (Cliente LIKE '%" + text + "%'))"));

  ui->tableOrcamentos->resizeColumnsToContents();
}

void WidgetOrcamento::on_tableOrcamentos_activated(const QModelIndex &index) {
  Orcamento *orcamento = new Orcamento(this);
  orcamento->viewRegisterById(modelOrcamento->data(index.row(), "Código"));
  orcamento->show();
}

void WidgetOrcamento::on_pushButtonCriarOrc_clicked() {
  Orcamento *orcamento = new Orcamento(this);
  orcamento->show();
}

// TODO: ao replicar marcar ids em ambos os orcamentos novos e antigos
// TODO: ao replicar marcar orcamento antigo como replicado e cancelar no bd
// TODO: fazer script SQL para expirar orcamentos vencidos
