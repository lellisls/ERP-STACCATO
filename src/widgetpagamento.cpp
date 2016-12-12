#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "adiantarrecebimento.h"
#include "checkboxdelegate.h"
#include "contas.h"
#include "doubledelegate.h"
#include "inserirlancamento.h"
#include "reaisdelegate.h"
#include "ui_widgetpagamento.h"
#include "widgetpagamento.h"

WidgetPagamento::WidgetPagamento(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetPagamento) {
  ui->setupUi(this);

  ui->radioButtonPendente->setChecked(true);
  ui->dateEditAte->setDate(QDate::currentDate());
  ui->dateEditDe->setDate(QDate::currentDate());

  ui->itemBoxLojas->setSearchDialog(SearchDialog::loja(this));
}

WidgetPagamento::~WidgetPagamento() { delete ui; }

void WidgetPagamento::setupTables() {
  model.setTable(tipo == Receber ? "view_conta_receber" : "view_conta_pagar");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  model.sort(model.fieldIndex("dataEmissao"), Qt::DescendingOrder);

  model.setHeaderData("dataEmissao", "Data Emissão");
  tipo == Receber ? model.setHeaderData("idVenda", "Código") : model.setHeaderData("ordemCompra", "OC");
  model.setHeaderData("contraParte", "ContraParte");
  model.setHeaderData("valor", "R$");
  model.setHeaderData("tipo", "Tipo");
  model.setHeaderData("parcela", "Parcela");
  model.setHeaderData("dataPagamento", "Data Pag.");
  model.setHeaderData("observacao", "Obs.");
  model.setHeaderData("status", "Status");
  model.setHeaderData("statusFinanceiro", "Status Financeiro");

  ui->table->setModel(&model);
  ui->table->hideColumn("idPagamento");
  ui->table->hideColumn("idLoja");
  ui->table->hideColumn("representacao");
  ui->table->setItemDelegate(new DoubleDelegate(this));
  ui->table->setItemDelegateForColumn("representacao", new CheckBoxDelegate(this, true));
  ui->table->setItemDelegateForColumn("valor", new ReaisDelegate(this, 2));
}

void WidgetPagamento::makeConnections() {
  connect(ui->lineEditBusca, &QLineEdit::textChanged, this, &WidgetPagamento::montaFiltro);
  connect(ui->radioButtonCancelado, &QRadioButton::toggled, this, &WidgetPagamento::montaFiltro);
  connect(ui->radioButtonPendente, &QRadioButton::toggled, this, &WidgetPagamento::montaFiltro);
  connect(ui->radioButtonRecebido, &QRadioButton::toggled, this, &WidgetPagamento::montaFiltro);
  connect(ui->radioButtonTodos, &QRadioButton::toggled, this, &WidgetPagamento::montaFiltro);
  connect(ui->dateEditAte, &QDateEdit::dateChanged, this, &WidgetPagamento::montaFiltro);
  connect(ui->doubleSpinBoxAte, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &WidgetPagamento::montaFiltro);
  connect(ui->itemBoxLojas, &ItemBox::textChanged, this, &WidgetPagamento::montaFiltro);
  connect(ui->groupBoxLojas, &QGroupBox::toggled, this, &WidgetPagamento::montaFiltro);
  connect(ui->groupBoxData, &QGroupBox::toggled, this, &WidgetPagamento::montaFiltro);
}

bool WidgetPagamento::updateTables() {
  if (model.tableName().isEmpty()) {
    setupTables();
    montaFiltro();
    makeConnections();
  }

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela conta_a_receber_has_pagamento: " + model.lastError().text());
    return false;
  }

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetPagamento::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetPagamento::on_table_activated(const QModelIndex &index) {
  Contas *contas = new Contas(tipo == Receber ? Contas::Receber : Contas::Pagar, this);
  QString idPagamento = model.data(index.row(), "idPagamento").toString();
  QString contraparte = model.data(index.row(), "Contraparte").toString();
  contas->viewConta(idPagamento, contraparte);
}

void WidgetPagamento::montaFiltro() {
  QString status;

  for (auto const &child : ui->groupBoxFiltros->findChildren<QRadioButton *>()) {
    if (child->text() == "Todos") continue;
    if (child->isChecked()) status = child->text();
  }

  if (not status.isEmpty()) status = "status = '" + status + "'";

  QString busca = ui->lineEditBusca->text();

  busca = "(" + QString(tipo == Pagar ? "ordemCompra" : "idVenda") + " LIKE '%" + busca + "%' OR contraparte LIKE '%" +
          busca + "%')";

  if (not status.isEmpty()) busca.prepend(" AND ");

  QString valor = ui->doubleSpinBoxDe->value() != 0. or ui->doubleSpinBoxAte->value() != 0.
                      ? "valor BETWEEN " + QString::number(ui->doubleSpinBoxDe->value() - 1) + " AND " +
                            QString::number(ui->doubleSpinBoxAte->value() + 1)
                      : "";

  valor = valor.isEmpty() ? "" : " AND " + valor;

  QString dataPag = ui->groupBoxData->isChecked()
                        ? "dataPagamento BETWEEN '" + ui->dateEditDe->date().toString("yyyy-MM-dd") + "' AND '" +
                              ui->dateEditAte->date().toString("yyyy-MM-dd") + "'"
                        : "";

  dataPag = dataPag.isEmpty() ? "" : " AND " + dataPag;

  QString loja;

  if (ui->groupBoxLojas->isChecked()) {
    loja = ui->itemBoxLojas->text().isEmpty() ? "" : ui->itemBoxLojas->value().toString();
    loja = loja.isEmpty() ? "" : " AND idLoja = " + loja;
  }

  QString representacao = tipo == Receber ? " AND representacao = 0" : "";

  model.setFilter(status + busca + valor + dataPag + loja + representacao);

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
  }

  ui->table->resizeColumnsToContents();
}

void WidgetPagamento::on_pushButtonInserirLancamento_clicked() {
  InserirLancamento *lancamento =
      new InserirLancamento(tipo == Receber ? InserirLancamento::Receber : InserirLancamento::Pagar, this);
  lancamento->show();
}

void WidgetPagamento::on_pushButtonAdiantarRecebimento_clicked() {
  AdiantarRecebimento *adiantar = new AdiantarRecebimento(this);
  adiantar->show();
}

void WidgetPagamento::on_doubleSpinBoxDe_valueChanged(const double value) { ui->doubleSpinBoxAte->setValue(value); }

void WidgetPagamento::on_dateEditDe_dateChanged(const QDate &date) { ui->dateEditAte->setDate(date); }

void WidgetPagamento::setTipo(const Tipo &value) {
  tipo = value;

  if (tipo == Pagar) {
    ui->pushButtonAdiantarRecebimento->hide();
    ui->radioButtonRecebido->hide();
  }

  if (tipo == Receber) ui->radioButtonPago->hide();
}

void WidgetPagamento::on_groupBoxData_toggled(const bool enabled) {
  for (auto const &child : ui->groupBoxData->findChildren<QDateEdit *>()) child->setEnabled(enabled);
}
