#include <QSqlError>
#include <QSqlQuery>

#include "produtospendentes.h"
#include "ui_widgetcomprapendentes.h"
#include "widgetcomprapendentes.h"

WidgetCompraPendentes::WidgetCompraPendentes(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompraPendentes) {
  ui->setupUi(this);

  setupTables();

  connect(ui->checkBoxFiltroPendentes, &QAbstractButton::toggled, this, &WidgetCompraPendentes::montaFiltro);
  connect(ui->checkBoxFiltroIniciados, &QAbstractButton::toggled, this, &WidgetCompraPendentes::montaFiltro);
  connect(ui->checkBoxFiltroCompra, &QAbstractButton::toggled, this, &WidgetCompraPendentes::montaFiltro);
  connect(ui->checkBoxFiltroFaturamento, &QAbstractButton::toggled, this, &WidgetCompraPendentes::montaFiltro);
  connect(ui->checkBoxFiltroColeta, &QAbstractButton::toggled, this, &WidgetCompraPendentes::montaFiltro);
  connect(ui->checkBoxFiltroRecebimento, &QAbstractButton::toggled, this, &WidgetCompraPendentes::montaFiltro);
  connect(ui->checkBoxFiltroEstoque, &QAbstractButton::toggled, this, &WidgetCompraPendentes::montaFiltro);
  connect(ui->lineEditBusca, &QLineEdit::textChanged, this, &WidgetCompraPendentes::montaFiltro);

  ui->checkBoxFiltroPendentes->setChecked(true);

  QSqlQuery("set lc_time_names = 'pt_BR'").exec();
}

WidgetCompraPendentes::~WidgetCompraPendentes() { delete ui; }

QString WidgetCompraPendentes::updateTables() {
  if (not modelProdPend.select()) return "Erro lendo tabela produtos pendentes: " + modelProdPend.lastError().text();

  ui->tableProdutosPend->resizeColumnsToContents();

  return QString();
}

void WidgetCompraPendentes::setupTables() {
  modelProdPend.setTable("view_produtos_pendentes");

  modelProdPend.setHeaderData("Form", "Form.");
  modelProdPend.setHeaderData("Quant", "Quant.");
  modelProdPend.setHeaderData("Un", "Un.");
  modelProdPend.setHeaderData("Cód Com", "Cód. Com.");

  ui->tableProdutosPend->setModel(&modelProdPend);
}

void WidgetCompraPendentes::on_tableProdutosPend_activated(const QModelIndex &index) {
  ProdutosPendentes *produtos = new ProdutosPendentes(this);

  const QString codComercial = modelProdPend.data(index.row(), "Cód Com").toString();
  const QString status = modelProdPend.data(index.row(), "Status").toString();

  produtos->viewProduto(codComercial, status);
}

void WidgetCompraPendentes::on_groupBoxStatus_toggled(const bool &enabled) {
  for (auto const &child : ui->groupBoxStatus->findChildren<QCheckBox *>()) {
    child->setEnabled(true);
    child->setChecked(enabled);
  }
}

void WidgetCompraPendentes::montaFiltro() {
  QString filtroCheck;

  for (auto const &child : ui->groupBoxStatus->findChildren<QCheckBox *>()) {
    if (child->isChecked()) {
      filtroCheck += QString(filtroCheck.isEmpty() ? "" : " OR ") + "status = '" + child->text().toUpper() + "'";
    }
  }

  const QString textoBusca = ui->lineEditBusca->text();

  const QString filtroBusca = textoBusca.isEmpty() ? "" : " AND ((Fornecedor LIKE '%" + textoBusca +
                                                     "%') OR (Descrição LIKE '%" + textoBusca +
                                                     "%') OR (`Cód Com` LIKE '%" + textoBusca + "%'))";

  modelProdPend.setFilter(filtroCheck + filtroBusca + " AND status != 'CANCELADO'");

  ui->tableProdutosPend->resizeColumnsToContents();
}
