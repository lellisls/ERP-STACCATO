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

  ui->radioButtonOrcLimpar->click();

  connect(ui->radioButtonOrcCancelado, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->radioButtonOrcExpirado, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->radioButtonOrcLimpar, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->radioButtonOrcProprios, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->radioButtonOrcValido, &QAbstractButton::toggled, this, &WidgetOrcamento::montaFiltro);
  connect(ui->lineEditBuscaOrcamentos, &QLineEdit::textChanged, this, &WidgetOrcamento::montaFiltro);

  if (UserSession::getTipoUsuario() == "VENDEDOR") {
    ui->radioButtonOrcProprios->click();
    ui->radioButtonOrcValido->setChecked(true);
  }

  montaFiltro();
}

WidgetOrcamento::~WidgetOrcamento() { delete ui; }

void WidgetOrcamento::setupTables() {
  DoubleDelegate *doubledelegate = new DoubleDelegate(this);

  modelOrcamento.setTable("view_orcamento");

  ui->tableOrcamentos->setModel(new OrcamentoProxyModel(&modelOrcamento, "Dias restantes", this));
  ui->tableOrcamentos->setItemDelegate(doubledelegate);
  ui->tableOrcamentos->sortByColumn("Código");
}

QString WidgetOrcamento::updateTables() {
  if (not modelOrcamento.select()) {
    return "Erro lendo tabela orçamento: " + modelOrcamento.lastError().text();
  }

  ui->tableOrcamentos->resizeColumnsToContents();

  return QString();
}

void WidgetOrcamento::on_tableOrcamentos_activated(const QModelIndex &index) {
  Orcamento *orcamento = new Orcamento(this);
  orcamento->viewRegisterById(modelOrcamento.data(index.row(), "Código"));
  orcamento->show();
}

void WidgetOrcamento::on_pushButtonCriarOrc_clicked() {
  Orcamento *orcamento = new Orcamento(this);
  orcamento->show();
}

void WidgetOrcamento::montaFiltro() {
  QString textoBusca = ui->lineEditBuscaOrcamentos->text();

  QString filtroBusca = "(Código LIKE '%" + UserSession::getFromLoja("sigla") + "%')" +
                        (textoBusca.isEmpty() ? "" : " AND ((Código LIKE '%" + textoBusca + "%') OR (Vendedor LIKE '%" +
                                                textoBusca + "%') OR (Cliente LIKE '%" + textoBusca + "%'))");

  QString f2;

  f2 += ui->radioButtonOrcLimpar->isChecked() ? "status != 'CANCELADO'" : "";
  f2 += ui->radioButtonOrcProprios->isChecked()
        ? "Vendedor = '" + UserSession::getNome() + "' AND status != 'CANCELADO'"
        : "";
  f2 += ui->radioButtonOrcValido->isChecked() ? "`Dias restantes` > 0 AND status != 'CANCELADO'" : "";
  f2 += ui->radioButtonOrcExpirado->isChecked() ? "`Dias restantes` <= 0 AND status != 'CANCELADO'" : "";
  f2 += ui->radioButtonOrcCancelado->isChecked() ? "status = 'CANCELADO'" : "";

  modelOrcamento.setFilter(textoBusca.isEmpty() ? f2 : filtroBusca + " AND " + f2);

  ui->tableOrcamentos->resizeColumnsToContents();
}
