#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include "anteciparrecebimento.h"
#include "checkboxdelegate.h"
#include "contas.h"
#include "doubledelegate.h"
#include "inserirlancamento.h"
#include "inserirtransferencia.h"
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
  model.setTable(tipo == Tipo::Receber ? "view_conta_receber" : "view_conta_pagar");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  //  model.sort(model.fieldIndex("dataEmissao"), Qt::DescendingOrder);

  // TODO: hide/remove column 'statusFinanceiro'

  model.setHeaderData("dataEmissao", "Data Emissão");
  model.setHeaderData("idVenda", "Código");
  if (tipo == Tipo::Pagar) model.setHeaderData("ordemCompra", "OC");
  model.setHeaderData("numeroNFe", "NFe");
  model.setHeaderData("contraParte", "ContraParte");
  model.setHeaderData("valor", "R$");
  model.setHeaderData("tipo", "Tipo");
  model.setHeaderData("parcela", "Parcela");
  model.setHeaderData("dataPagamento", "Data Pag.");
  model.setHeaderData("observacao", "Obs.");
  model.setHeaderData("status", "Status");
  model.setHeaderData("statusFinanceiro", "Status Financeiro");

  ui->table->setModel(&model);
  ui->table->hideColumn("representacao");
  ui->table->hideColumn("idPagamento");
  ui->table->hideColumn("idLoja");
  ui->table->setItemDelegate(new DoubleDelegate(this));
  //  ui->table->setItemDelegateForColumn("representacao", new CheckBoxDelegate(this, true));
  ui->table->setItemDelegateForColumn("valor", new ReaisDelegate(this, 2));

  //

  ui->tableVencidos->setModel(&modelVencidos);
  ui->tableVencidos->setItemDelegate(new ReaisDelegate(this));

  ui->tableVencer->setModel(&modelVencer);
  ui->tableVencer->setItemDelegate(new ReaisDelegate(this));
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

  modelVencidos.setQuery("SELECT v.*, @running_total := @running_total + v.Total AS Acumulado FROM " + QString(tipo == Tipo::Receber ? "view_a_receber_vencidos_base" : "view_a_pagar_vencidos_base") +
                         " v JOIN (SELECT @running_total := 0) r");

  ui->tableVencidos->resizeColumnsToContents();

  modelVencer.setQuery("SELECT v.*, @running_total := @running_total + v.Total AS Acumulado FROM " + QString(tipo == Tipo::Receber ? "view_a_receber_vencer_base" : "view_a_pagar_vencer_base") +
                       " v JOIN (SELECT @running_total := 0) r");

  ui->tableVencer->resizeColumnsToContents();

  return true;
}

void WidgetPagamento::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetPagamento::on_table_activated(const QModelIndex &index) {
  auto *contas = new Contas(tipo == Tipo::Receber ? Contas::Receber : Contas::Pagar, this);
  contas->setAttribute(Qt::WA_DeleteOnClose);
  QString idPagamento = model.data(index.row(), "idPagamento").toString();
  QString contraparte = model.data(index.row(), "Contraparte").toString();
  contas->viewConta(idPagamento, contraparte);
  // TODO: poder selecionar mais de um idPagamento (contraParte é estético)
  // ajustar para selecionar mais de uma linha e ajustar no filtro da Contas
}

void WidgetPagamento::montaFiltro() {
  QString status;

  for (auto const &child : ui->groupBoxFiltros->findChildren<QRadioButton *>()) {
    if (child->text() == "Todos") continue;

    if (child->isChecked() and child->text() == "Pendente/Conferido") {
      status = "(status = 'PENDENTE' OR status = 'CONFERIDO')";
      break;
    }

    if (child->isChecked()) status = child->text();
  }

  if (not status.isEmpty() and status != "(status = 'PENDENTE' OR status = 'CONFERIDO')") {
    status = "status = '" + status + "'";
  }

  const QString text = ui->lineEditBusca->text();

  const QString busca = QString(status.isEmpty() ? "" : " AND ") + "(" + QString(tipo == Tipo::Pagar ? "ordemCompra" : "idVenda") + " LIKE '%" + text + "%' OR contraparte LIKE '%" + text +
                        "%'" + /*OR contraParte IS NULL" +*/
                        QString(tipo == Tipo::Pagar ? " OR numeroNFe LIKE '%" + text + "%' OR idVenda LIKE '%" + text + "%'" : "") + ")";

  const QString valor = ui->doubleSpinBoxDe->value() != 0. or ui->doubleSpinBoxAte->value() != 0.
                            ? " AND valor BETWEEN " + QString::number(ui->doubleSpinBoxDe->value() - 1) + " AND " + QString::number(ui->doubleSpinBoxAte->value() + 1)
                            : "";

  const QString dataPag =
      ui->groupBoxData->isChecked() ? " AND dataPagamento BETWEEN '" + ui->dateEditDe->date().toString("yyyy-MM-dd") + "' AND '" + ui->dateEditAte->date().toString("yyyy-MM-dd") + "'" : "";

  const QString loja = ui->groupBoxLojas->isChecked() and not ui->itemBoxLojas->text().isEmpty() ? " AND idLoja = " + ui->itemBoxLojas->getValue().toString() : "";

  const QString representacao = tipo == Tipo::Receber ? " AND representacao = 0" : "";

  // TODO: replace 'dataPagamento' with 'dataRealizado'?
  const QString ordenacao = tipo == Tipo::Receber ? " ORDER BY `dataPagamento` , `idVenda` , `tipo` , `parcela` DESC" : " ORDER BY `dataPagamento` , `ordemCompra` , `tipo` , `parcela` DESC";

  model.setFilter(status + busca + valor + dataPag + loja + representacao + ordenacao);

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());

  ui->table->resizeColumnsToContents();
}

void WidgetPagamento::on_pushButtonInserirLancamento_clicked() {
  auto *lancamento = new InserirLancamento(tipo == Tipo::Receber ? InserirLancamento::Receber : InserirLancamento::Pagar, this);
  lancamento->setAttribute(Qt::WA_DeleteOnClose);
  lancamento->show();
}

void WidgetPagamento::on_pushButtonAdiantarRecebimento_clicked() {
  //  const auto list = ui->table->selectionModel()->selectedRows();

  //  if (list.isEmpty()) {
  //    QMessageBox::critical(this, "Erro!", "Nenhuma linha selecionada!");
  //    return;
  //  }

  auto *adiantar = new AnteciparRecebimento(this);
  adiantar->setAttribute(Qt::WA_DeleteOnClose);
  //  adiantar->filtrar(model.data(list.first().row(), "idVenda").toString(),
  //                    model.data(list.first().row(), "tipo").toString());
  adiantar->show();
}

void WidgetPagamento::on_doubleSpinBoxDe_valueChanged(const double value) { ui->doubleSpinBoxAte->setValue(value); }

void WidgetPagamento::on_dateEditDe_dateChanged(const QDate &date) { ui->dateEditAte->setDate(date); }

void WidgetPagamento::setTipo(const Tipo &value) {
  tipo = value;

  if (tipo == Tipo::Pagar) {
    ui->pushButtonAdiantarRecebimento->hide();
    ui->radioButtonRecebido->hide();
  }

  if (tipo == Tipo::Receber) {
    ui->radioButtonPago->hide();
  }
}

void WidgetPagamento::on_groupBoxData_toggled(const bool enabled) {
  for (auto const &child : ui->groupBoxData->findChildren<QDateEdit *>()) child->setEnabled(enabled);
}

void WidgetPagamento::on_tableVencidos_entered(const QModelIndex &) { ui->tableVencidos->resizeColumnsToContents(); }

void WidgetPagamento::on_tableVencer_entered(const QModelIndex &) { ui->tableVencer->resizeColumnsToContents(); }

void WidgetPagamento::on_tableVencidos_doubleClicked(const QModelIndex &index) {
  ui->dateEditDe->setDate(modelVencidos.record(index.row()).value("Data Pagamento").toDate());
  ui->dateEditAte->setDate(modelVencidos.record(index.row()).value("Data Pagamento").toDate());

  ui->groupBoxData->setChecked(true);

  ui->tableVencer->clearSelection();
}

void WidgetPagamento::on_tableVencer_doubleClicked(const QModelIndex &index) {
  ui->dateEditDe->setDate(modelVencer.record(index.row()).value("Data Pagamento").toDate());
  ui->dateEditAte->setDate(modelVencer.record(index.row()).value("Data Pagamento").toDate());

  ui->groupBoxData->setChecked(true);

  ui->tableVencidos->clearSelection();
}

void WidgetPagamento::on_pushButtonInserirTransferencia_clicked() {
  auto *transferencia = new InserirTransferencia(this);
  transferencia->setAttribute(Qt::WA_DeleteOnClose);
  transferencia->show();
}

// TODO: 1as tabelas de contas tem duas colunas status redundantes

void WidgetPagamento::on_pushButtonExcluirLancamento_clicked() {
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhuma linha selecionada!");
    return;
  }

  QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Tem certeza que deseja excluir?", QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Excluir");
  msgBox.setButtonText(QMessageBox::No, "Voltar");

  if (msgBox.exec() == QMessageBox::Yes) {
    QSqlQuery query;
    query.prepare("UPDATE " + QString(tipo == Tipo::Pagar ? "conta_a_pagar_has_pagamento" : "conta_a_receber_has_pagamento") + " SET desativado = TRUE WHERE idPagamento = :idPagamento");
    query.bindValue(":idPagamento", model.data(list.first().row(), "idPagamento"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro excluindo lançamento: " + query.lastError().text());
      return;
    }

    if (not model.select()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando tabela: " + model.lastError().text());
      return;
    }

    QMessageBox::information(this, "Aviso!", "Lançamento excluído com sucesso!");
  }
}
