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

WidgetOrcamento::WidgetOrcamento(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetOrcamento) { ui->setupUi(this); }

WidgetOrcamento::~WidgetOrcamento() { delete ui; }

void WidgetOrcamento::setPermissions() {
  if (UserSession::tipoUsuario() == "ADMINISTRADOR" or UserSession::tipoUsuario() == "DIRETOR") {
    QSqlQuery query("SELECT descricao, idLoja FROM loja WHERE desativado = FALSE");

    while (query.next()) ui->comboBoxLojas->addItem(query.value("descricao").toString(), query.value("idLoja"));

    ui->comboBoxLojas->setCurrentValue(UserSession::idLoja());

    ui->groupBoxMes->setChecked(true);
  }

  if (UserSession::tipoUsuario() == "GERENTE LOJA") {
    ui->groupBoxLojas->hide();

    QSqlQuery query("SELECT idUsuario, user FROM usuario WHERE desativado = FALSE AND idLoja = " + QString::number(UserSession::idLoja()));

    ui->comboBoxVendedores->addItem("");

    while (query.next()) ui->comboBoxVendedores->addItem(query.value("user").toString(), query.value("idUsuario"));
  }

  if (UserSession::tipoUsuario() == "VENDEDOR") {
    QSqlQuery query("SELECT descricao, idLoja FROM loja WHERE desativado = FALSE");

    while (query.next()) ui->comboBoxLojas->addItem(query.value("descricao").toString(), query.value("idLoja"));

    ui->radioButtonProprios->click();
    ui->checkBoxValido->setChecked(true);
    ui->checkBoxExpirado->setChecked(true);

    ui->groupBoxVendedores->hide();
  } else {
    ui->radioButtonTodos->click();
  }

  ui->dateEdit->setDate(QDate::currentDate());
}

void WidgetOrcamento::setupTables() {
  model.setTable("view_orcamento2"); // TODO: refactor other querys that use 'find last of'

  ui->table->setModel(new OrcamentoProxyModel(&model, this));
  ui->table->setItemDelegateForColumn("Total", new ReaisDelegate(this));
  ui->table->hideColumn("idLoja");
  ui->table->hideColumn("idUsuario");

  if (UserSession::tipoUsuario() != "VENDEDOR ESPECIAL") ui->table->hideColumn("Indicou");
}

void WidgetOrcamento::setupConnections() {
  connect(ui->checkBoxCancelado, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->checkBoxExpirado, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->checkBoxFechado, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->checkBoxPerdido, &QCheckBox::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->checkBoxReplicado, &QCheckBox::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->checkBoxValido, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->comboBoxLojas, &QComboBox::currentTextChanged, this, &WidgetOrcamento::montaFiltro);
  connect(ui->comboBoxVendedores, &QComboBox::currentTextChanged, this, &WidgetOrcamento::montaFiltro);
  connect(ui->dateEdit, &QDateEdit::dateChanged, this, &WidgetOrcamento::montaFiltro);
  connect(ui->groupBoxMes, &QGroupBox::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->lineEditBusca, &QLineEdit::textChanged, this, &WidgetOrcamento::montaFiltro);
  connect(ui->radioButtonProprios, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->radioButtonTodos, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
}

bool WidgetOrcamento::updateTables() {
  if (model.tableName().isEmpty()) {
    setPermissions();
    setupTables();
    montaFiltro();
    setupConnections();
  }

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela orçamento: " + model.lastError().text());
    return false;
  }

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetOrcamento::on_table_activated(const QModelIndex &index) {
  auto *orcamento = new Orcamento(this);
  orcamento->setAttribute(Qt::WA_DeleteOnClose);
  orcamento->viewRegisterById(model.data(index.row(), "Código"));
  orcamento->show();
}

void WidgetOrcamento::on_pushButtonCriarOrc_clicked() {
  auto *orcamento = new Orcamento(this);
  orcamento->setAttribute(Qt::WA_DeleteOnClose);
  orcamento->show();
}

void WidgetOrcamento::montaFiltro() {
  QString filtroLoja = ui->comboBoxLojas->currentText().isEmpty() ? "(Código LIKE '%" + UserSession::fromLoja("sigla") + "%')" : "idLoja = " + ui->comboBoxLojas->getCurrentValue().toString();

  const QString filtroRadio = ui->radioButtonTodos->isChecked() ? "" : " AND Vendedor = '" + UserSession::nome() + "'";

  QString filtroCheck;

  for (auto const &child : ui->groupBoxStatus->findChildren<QCheckBox *>()) {
    if (child->isChecked()) {
      filtroCheck += filtroCheck.isEmpty() ? "status = '" + child->text().toUpper() + "'" : " OR status = '" + child->text().toUpper() + "'";
    }
  }

  filtroCheck = filtroCheck.isEmpty() ? "" : " AND (" + filtroCheck + ")";

  const QString filtroData = ui->groupBoxMes->isChecked() ? " AND DATE_FORMAT(Data, '%Y-%m') = '" + ui->dateEdit->date().toString("yyyy-MM") + "'" : "";

  const QString filtroVendedor = ui->comboBoxVendedores->currentText().isEmpty() ? "" : " AND idUsuario = " + ui->comboBoxVendedores->getCurrentValue().toString();

  const QString textoBusca = ui->lineEditBusca->text();

  const QString filtroBusca =
      textoBusca.isEmpty() ? ""
                           : " AND (Código LIKE '%" + textoBusca + "%' OR Vendedor LIKE '%" + textoBusca + "%' OR Cliente LIKE '%" + textoBusca + "%' OR Profissional LIKE '%" + textoBusca + "%')";

  model.setFilter(filtroLoja + filtroData + filtroVendedor + filtroRadio + filtroCheck + filtroBusca);

  ui->table->resizeColumnsToContents();
}

void WidgetOrcamento::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetOrcamento::on_pushButtonFollowup_clicked() {
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhuma linha selecionada!");
    return;
  }

  FollowUp *followup = new FollowUp(model.data(list.first().row(), "Código").toString(), FollowUp::Orcamento, this);
  followup->setAttribute(Qt::WA_DeleteOnClose);
  followup->show();
}

void WidgetOrcamento::on_groupBoxStatus_toggled(const bool enabled) {
  for (auto const &child : ui->groupBoxStatus->findChildren<QCheckBox *>()) {
    child->setEnabled(true);
    child->setChecked(enabled);
  }
}

void WidgetOrcamento::on_comboBoxLojas_currentIndexChanged(const int) {
  ui->comboBoxVendedores->clear();

  QSqlQuery query2("SELECT idUsuario, user FROM usuario WHERE desativado = FALSE AND tipo = 'VENDEDOR'" +
                   (ui->comboBoxLojas->currentText().isEmpty() ? "" : " AND idLoja = " + ui->comboBoxLojas->getCurrentValue().toString()));

  ui->comboBoxVendedores->addItem("");

  while (query2.next()) ui->comboBoxVendedores->addItem(query2.value("user").toString(), query2.value("idUsuario"));
}

// TODO: 1por padrao nao ativar filtro mes quando for vendedor (acho que já foi feito)
// TODO: alterar followup para guardar apenas os 12 primeiros caracteres (remover -RevXXX)
