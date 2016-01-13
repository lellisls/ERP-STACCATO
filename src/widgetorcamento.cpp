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

  setupTables();

  ui->radioButtonTodos->click();

  connect(ui->radioButtonCancelado, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->radioButtonExpirado, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->radioButtonTodos, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->radioButtonProprios, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->radioButtonValido, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->lineEditBusca, &QLineEdit::textChanged, this, &WidgetOrcamento::montaFiltro);

  if (UserSession::tipoUsuario() == "VENDEDOR") {
    ui->radioButtonProprios->click();
    ui->radioButtonValido->setChecked(true);
  }

  montaFiltro();
}

WidgetOrcamento::~WidgetOrcamento() { delete ui; }

void WidgetOrcamento::setupTables() {
  DoubleDelegate *doubledelegate = new DoubleDelegate(this);

  model.setTable("view_orcamento");

  ui->table->setModel(new OrcamentoProxyModel(&model, "Dias restantes", this));
  ui->table->setItemDelegate(doubledelegate);
  ui->table->sortByColumn("Código");
}

QString WidgetOrcamento::updateTables() {
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

  QString filtroRadio = " AND ";

  filtroRadio += ui->radioButtonTodos->isChecked() ? "status != 'CANCELADO'" : "";
  filtroRadio += ui->radioButtonProprios->isChecked()
                 ? "Vendedor = '" + UserSession::nome() + "' AND status != 'CANCELADO'"
                 : "";
  filtroRadio += ui->radioButtonValido->isChecked() ? "`Dias restantes` > 0 AND status != 'CANCELADO'" : "";
  filtroRadio += ui->radioButtonExpirado->isChecked() ? "`Dias restantes` <= 0 AND status != 'CANCELADO'" : "";
  filtroRadio += ui->radioButtonCancelado->isChecked() ? "status = 'CANCELADO'" : "";

  const QString textoBusca = ui->lineEditBusca->text();

  const QString filtroBusca = textoBusca.isEmpty() ? "" : " AND ((Código LIKE '%" + textoBusca +
                                                     "%') OR (Vendedor LIKE '%" + textoBusca +
                                                     "%') OR (Cliente LIKE '%" + textoBusca + "%'))";

  model.setFilter(filtroLoja + filtroRadio + filtroBusca);

  ui->table->resizeColumnsToContents();
}
