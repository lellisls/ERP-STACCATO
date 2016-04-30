#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

#include "followup.h"
#include "orcamento.h"
#include "orcamentoproxymodel.h"
#include "reaisdelegate.h"
#include "ui_widgetorcamento.h"
#include "usersession.h"
#include "widgetorcamento.h"

WidgetOrcamento::WidgetOrcamento(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetOrcamento) {
  ui->setupUi(this);

  connect(ui->lineEditBusca, &QLineEdit::textChanged, this, &WidgetOrcamento::montaFiltro);
  connect(ui->checkBoxCancelado, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->checkBoxExpirado, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->checkBoxFechado, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->radioButtonProprios, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->radioButtonTodos, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->checkBoxValido, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->checkBoxReplicado, &QCheckBox::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->checkBoxPerdido, &QCheckBox::toggled, this, &WidgetOrcamento::montaFiltro);

  if (UserSession::tipoUsuario() == "VENDEDOR") {
    ui->radioButtonProprios->click();
    ui->checkBoxValido->setChecked(true);
    ui->checkBoxExpirado->setChecked(true);
  } else {
    ui->radioButtonTodos->click();
  }
}

WidgetOrcamento::~WidgetOrcamento() { delete ui; }

void WidgetOrcamento::setupTables() {
  ReaisDelegate *reaisDelegate = new ReaisDelegate(this);

  model.setTable("view_orcamento");

  ui->table->setModel(new OrcamentoProxyModel(&model, this));
  ui->table->setItemDelegateForColumn("Total", reaisDelegate);
}

bool WidgetOrcamento::updateTables(QString &error) {
  if (model.tableName().isEmpty()) {
    setupTables();
    montaFiltro();
  }

  if (not model.select()) {
    error = "Erro lendo tabela orçamento: " + model.lastError().text();
    return false;
  }

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetOrcamento::on_table_activated(const QModelIndex &index) {
  Orcamento *orcamento = new Orcamento(this);
  orcamento->viewRegisterById(model.data(index.row(), "Código"));
  orcamento->show();
}

void WidgetOrcamento::on_pushButtonCriarOrc_clicked() {
  Orcamento *orcamento = new Orcamento(this);
  orcamento->show();
}

void WidgetOrcamento::montaFiltro() {
  const QString filtroLoja = "(Código LIKE '%" + UserSession::fromLoja("sigla") + "%')";

  const QString filtroRadio = ui->radioButtonTodos->isChecked() ? "" : " AND Vendedor = '" + UserSession::nome() + "'";

  QString filtroCheck;

  for (auto const &child : ui->groupBoxStatus->findChildren<QCheckBox *>()) {
    if (child->isChecked()) {
      filtroCheck += filtroCheck.isEmpty() ? "status = '" + child->text().toUpper() + "'"
                                           : " OR status = '" + child->text().toUpper() + "'";
    }
  }

  filtroCheck = filtroCheck.isEmpty() ? "" : " AND (" + filtroCheck + ")";

  const QString textoBusca = ui->lineEditBusca->text();

  const QString filtroBusca = textoBusca.isEmpty() ? "" : " AND ((Código LIKE '%" + textoBusca +
                                                              "%') OR (Vendedor LIKE '%" + textoBusca +
                                                              "%') OR (Cliente LIKE '%" + textoBusca + "%'))";

  model.setFilter(filtroLoja + filtroRadio + filtroCheck + filtroBusca);

  ui->table->resizeColumnsToContents();
}

void WidgetOrcamento::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetOrcamento::on_pushButtonFollowup_clicked() {
  auto list = ui->table->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhuma linha selecionada!");
    return;
  }

  FollowUp *followup = new FollowUp(model.data(list.first().row(), "Código").toString(), this);
  followup->show();
}

void WidgetOrcamento::on_groupBoxStatus_toggled(const bool &enabled) {
  for (auto const &child : ui->groupBoxStatus->findChildren<QCheckBox *>()) {
    child->setEnabled(true);
    child->setChecked(enabled);
  }
}
