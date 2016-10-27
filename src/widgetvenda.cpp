#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

#include "followup.h"
#include "orcamentoproxymodel.h"
#include "reaisdelegate.h"
#include "ui_widgetvenda.h"
#include "usersession.h"
#include "venda.h"
#include "widgetvenda.h"

WidgetVenda::WidgetVenda(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetVenda) { ui->setupUi(this); }

WidgetVenda::~WidgetVenda() { delete ui; }

void WidgetVenda::setupTables() {
  ReaisDelegate *reaisDelegate = new ReaisDelegate(this);

  model.setTable("view_venda");

  ui->table->setModel(new OrcamentoProxyModel(&model, this));
  ui->table->hideColumn("statusEntrega");
  ui->table->hideColumn("idLoja");
  ui->table->hideColumn("idUsuario");
  ui->table->hideColumn("Data Próx Followup");
  ui->table->hideColumn("Observação");
  ui->table->setItemDelegateForColumn("Total R$", reaisDelegate);

  if (UserSession::tipoUsuario() != "VENDEDOR ESPECIAL") ui->table->hideColumn("Indicou");

  ui->pushButtonFollowup->hide();
}

void WidgetVenda::montaFiltro() {
  QString sigla = UserSession::fromLoja("sigla");

  if (ui->groupBoxLojas->isVisible() and not ui->comboBoxLojas->currentText().isEmpty()) {
    QSqlQuery query;
    query.prepare("SELECT sigla FROM loja WHERE descricao = :descricao");
    query.bindValue(":descricao", ui->comboBoxLojas->currentText());

    if (not query.exec() or not query.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando sigla da loja: " + query.lastError().text());
      return;
    }

    sigla = query.value("sigla").toString();
  }

  const QString filtroLoja = "(Código LIKE '%" + sigla + "%')";

  const QString filtroRadio = ui->radioButtonTodos->isChecked() ? "" : " AND Vendedor = '" + UserSession::nome() + "'";

  QString filtroCheck;

  if (financeiro) {
    for (auto const &child : ui->groupBoxStatusFinanceiro->findChildren<QCheckBox *>()) {
      if (child->isChecked()) {
        filtroCheck += filtroCheck.isEmpty() ? "`Status Financeiro` = '" + child->text().toUpper() + "'"
                                             : " OR `Status Financeiro` = '" + child->text().toUpper() + "'";
      }
    }
  } else {
    for (auto const &child : ui->groupBoxStatus->findChildren<QCheckBox *>()) {
      if (child->isChecked()) {
        filtroCheck += filtroCheck.isEmpty() ? "status = '" + child->text().toUpper() + "'"
                                             : " OR status = '" + child->text().toUpper() + "'";
      }
    }
  }

  filtroCheck = filtroCheck.isEmpty() ? "" : " AND (" + filtroCheck + ")";

  const QString filtroData =
      ui->groupBoxMes->isChecked()
          ? " AND DATE_FORMAT(Data, '%Y-%m') = '" + ui->dateEdit->date().toString("yyyy-MM") + "'"
          : "";

  const QString filtroVendedor = ui->comboBoxVendedores->currentText().isEmpty()
                                     ? ""
                                     : " AND idUsuario = " + ui->comboBoxVendedores->getCurrentValue().toString();

  const QString textoBusca = ui->lineEditBusca->text();

  const QString filtroBusca = textoBusca.isEmpty() ? ""
                                                   : " AND ((Código LIKE '%" + textoBusca + "%') OR (Vendedor LIKE '%" +
                                                         textoBusca + "%') OR (Cliente LIKE '%" + textoBusca + "%'))";

  model.setFilter(filtroLoja + filtroData + filtroVendedor + filtroRadio + filtroCheck + filtroBusca);

  ui->table->resizeColumnsToContents();
}

void WidgetVenda::on_groupBoxStatus_toggled(const bool &enabled) {
  for (auto const &child : ui->groupBoxStatus->findChildren<QCheckBox *>()) {
    child->setEnabled(true);
    child->setChecked(enabled);
  }
}

void WidgetVenda::setPermissions() {
  if (UserSession::tipoUsuario() == "ADMINISTRADOR" or UserSession::tipoUsuario() == "DIRETOR") {
    QSqlQuery query("SELECT descricao, idLoja FROM loja WHERE desativado = FALSE");

    while (query.next()) ui->comboBoxLojas->addItem(query.value("descricao").toString(), query.value("idLoja"));

    ui->comboBoxLojas->setCurrentValue(UserSession::idLoja());
  }

  if (UserSession::tipoUsuario() == "GERENTE LOJA") {
    ui->groupBoxLojas->hide();

    QSqlQuery query("SELECT idUsuario, user FROM usuario WHERE desativado = FALSE AND idLoja = " +
                    QString::number(UserSession::idLoja()));

    ui->comboBoxVendedores->addItem("");

    while (query.next()) ui->comboBoxVendedores->addItem(query.value("user").toString(), query.value("idUsuario"));
  }

  if (UserSession::tipoUsuario() == "VENDEDOR") {
    QSqlQuery query("SELECT descricao, idLoja FROM loja WHERE desativado = FALSE");

    while (query.next()) ui->comboBoxLojas->addItem(query.value("descricao").toString(), query.value("idLoja"));

    ui->radioButtonProprios->click();

    ui->groupBoxVendedores->hide();
  } else {
    ui->radioButtonTodos->click();
  }

  ui->dateEdit->setDate(QDate::currentDate());
}

void WidgetVenda::makeConnections() {
  connect(ui->checkBoxCancelado, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxDevolvido, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxEmColeta, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxEmCompra, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxEmFaturamento, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxEmRecebimento, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxEstoque, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxFinalizado, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxIniciado, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxPendente, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->comboBoxLojas, &ComboBox::currentTextChanged, this, &WidgetVenda::montaFiltro);
  connect(ui->comboBoxVendedores, &ComboBox::currentTextChanged, this, &WidgetVenda::montaFiltro);
  connect(ui->dateEdit, &QDateEdit::dateChanged, this, &WidgetVenda::montaFiltro);
  connect(ui->groupBoxMes, &QGroupBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->lineEditBusca, &QLineEdit::textChanged, this, &WidgetVenda::montaFiltro);
  connect(ui->radioButtonProprios, &QRadioButton::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->radioButtonTodos, &QRadioButton::toggled, this, &WidgetVenda::montaFiltro);
}

bool WidgetVenda::updateTables() {
  if (model.tableName().isEmpty()) {
    setPermissions();
    setupTables();
    montaFiltro();
    makeConnections();
  }

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela vendas: " + model.lastError().text());
    return false;
  }

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetVenda::on_table_activated(const QModelIndex &index) {
  Venda *vendas = new Venda(this);
  if (financeiro) vendas->setFinanceiro();
  vendas->viewRegisterById(model.data(index.row(), "Código"));
}

void WidgetVenda::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetVenda::on_radioButtonProprios_toggled(bool checked) {
  if (UserSession::tipoUsuario() == "VENDEDOR") checked ? ui->groupBoxLojas->show() : ui->groupBoxLojas->hide();
}

void WidgetVenda::on_comboBoxLojas_currentIndexChanged(int) {
  ui->comboBoxVendedores->clear();

  QSqlQuery query2("SELECT idUsuario, user FROM usuario WHERE desativado = FALSE AND tipo = 'VENDEDOR'" +
                   (ui->comboBoxLojas->currentText().isEmpty()
                        ? ""
                        : " AND idLoja = " + ui->comboBoxLojas->getCurrentValue().toString()));

  ui->comboBoxVendedores->addItem("");

  while (query2.next()) {
    ui->comboBoxVendedores->addItem(query2.value("user").toString(), query2.value("idUsuario"));
  }
}

void WidgetVenda::setFinanceiro() {
  ui->table->showColumn(model.fieldIndex("Status Financeiro"));
  ui->pushButtonFollowup->show();
  ui->table->showColumn(model.fieldIndex("Data Próx Followup"));
  ui->table->showColumn(model.fieldIndex("Observação"));
  ui->groupBoxStatus->hide();
  financeiro = true;

  connect(ui->checkBoxPendente2, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxLiberado, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxConferido, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
}

void WidgetVenda::on_pushButtonFollowup_clicked() {
  auto list = ui->table->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhuma linha selecionada!");
    return;
  }

  FollowUp *followup = new FollowUp(model.data(list.first().row(), "Código").toString(), FollowUp::Venda, this);
  followup->show();
}

void WidgetVenda::on_groupBoxStatusFinanceiro_toggled(const bool &enabled) {
  for (auto const &child : ui->groupBoxStatusFinanceiro->findChildren<QCheckBox *>()) {
    child->setEnabled(true);
    child->setChecked(enabled);
  }
}

// NOTE: verificar como lidar com brinde/reposicao
// TODO: arrumar update_venda_status para nao mudar status para devolvido em devolucoes parciais; nao alterar o status
// 'devolvido' de devolucoes
// TODO: ao abrir no financeiro setar o status atual em vez de mostrar 'PENDENTE'
// TODO: fazer followup pós-venda
