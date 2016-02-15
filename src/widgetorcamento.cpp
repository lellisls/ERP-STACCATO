#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

#include "doubledelegate.h"
#include "orcamento.h"
#include "orcamentoproxymodel.h"
#include "ui_widgetorcamento.h"
#include "usersession.h"
#include "widgetorcamento.h"

WidgetOrcamento::WidgetOrcamento(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetOrcamento) {
  ui->setupUi(this);

  connect(ui->lineEditBusca, &QLineEdit::textChanged, this, &WidgetOrcamento::montaFiltro);
  connect(ui->radioButtonCancelado, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->radioButtonExpirado, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->radioButtonFechado, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->radioButtonProprios, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->radioButtonTodos, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->radioButtonValido, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);

  if (UserSession::tipoUsuario() == "VENDEDOR") {
    ui->radioButtonProprios->click();
    ui->radioButtonValido->setChecked(true);
  }

  ui->radioButtonTodos->click();
}

WidgetOrcamento::~WidgetOrcamento() { delete ui; }

void WidgetOrcamento::setupTables() {
  DoubleDelegate *doubledelegate = new DoubleDelegate(this);

  model.setTable("view_orcamento");

  ui->table->setModel(new OrcamentoProxyModel(&model, this));
  ui->table->setItemDelegate(doubledelegate);
  ui->table->sortByColumn("Código");
}

QString WidgetOrcamento::updateTables() {
  if (model.tableName().isEmpty()) {
    setupTables();
    montaFiltro();
  }

  if (not model.select()) {
    return "Erro lendo tabela orçamento: " + model.lastError().text();
  }

  ui->table->resizeColumnsToContents();

  return QString();
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

  QString filtroRadio;

  if (ui->radioButtonTodos->isChecked()) filtroRadio = " AND status != 'CANCELADO' AND status != 'FECHADO'";
  if (ui->radioButtonProprios->isChecked()) filtroRadio = " AND Vendedor = '" + UserSession::nome() + "'";
  if (ui->radioButtonValido->isChecked()) filtroRadio = " AND status = 'ATIVO'";
  if (ui->radioButtonExpirado->isChecked()) filtroRadio = " AND status = 'EXPIRADO'";
  if (ui->radioButtonCancelado->isChecked()) filtroRadio = " AND status = 'CANCELADO'";
  if (ui->radioButtonFechado->isChecked()) filtroRadio = " AND status = 'FECHADO'";

  const QString textoBusca = ui->lineEditBusca->text();

  const QString filtroBusca = textoBusca.isEmpty() ? "" : " AND ((Código LIKE '%" + textoBusca +
                                                     "%') OR (Vendedor LIKE '%" + textoBusca +
                                                     "%') OR (Cliente LIKE '%" + textoBusca + "%'))";

  model.setFilter(filtroLoja + filtroRadio + filtroBusca);

  ui->table->resizeColumnsToContents();
}

// TODO: transformar os filtros em checkbox
// TODO: colocar tela de total orcamento (copiar do total venda mes)
// TODO: colocar autoredimensionar no scroll do table
