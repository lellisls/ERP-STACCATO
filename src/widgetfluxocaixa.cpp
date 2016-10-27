#include <QSqlError>

#include "doubledelegate.h"
#include "reaisdelegate.h"
#include "ui_widgetfluxocaixa.h"
#include "widgetfluxocaixa.h"

WidgetFluxoCaixa::WidgetFluxoCaixa(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetFluxoCaixa) {
  ui->setupUi(this);

  setupTables();

  ui->dateEdit->setDate(QDate::currentDate());
  montaFiltro();

  connect(ui->groupBoxMes, &QGroupBox::toggled, this, &WidgetFluxoCaixa::montaFiltro);
  connect(ui->dateEdit, &QDateEdit::dateChanged, this, &WidgetFluxoCaixa::montaFiltro);
}

WidgetFluxoCaixa::~WidgetFluxoCaixa() { delete ui; }

void WidgetFluxoCaixa::setupTables() {
  model.setTable("view_fluxo_caixa");
  ui->tableCaixa->setModel(&model);
  ui->tableCaixa->setItemDelegateForColumn("R$", new ReaisDelegate(this));

  model2.setTable("view_fluxo_resumo");
  ui->tableResumo->setModel(&model2);
  ui->tableResumo->setItemDelegateForColumn("SAIDA", new ReaisDelegate(this));
  ui->tableResumo->setItemDelegateForColumn("ENTRADA", new ReaisDelegate(this));
  ui->tableResumo->setItemDelegateForColumn("R$", new ReaisDelegate(this));
}

bool WidgetFluxoCaixa::updateTables() {
  montaFiltro();

  return true;
}

void WidgetFluxoCaixa::montaFiltro() {
  const QString filtroData =
      ui->groupBoxMes->isChecked()
          ? "DATE_FORMAT(`Data Pag`, '%Y-%m') = '" + ui->dateEdit->date().toString("yyyy-MM") + "'"
          : "";

  model.setFilter(filtroData);

  if (not model.select()) emit errorSignal(model.lastError().text());

  model2.setFilter(filtroData);

  if (not model2.select()) emit errorSignal(model2.lastError().text());
}

void WidgetFluxoCaixa::on_tableCaixa_entered(const QModelIndex &) { ui->tableCaixa->resizeColumnsToContents(); }

void WidgetFluxoCaixa::on_tableResumo_entered(const QModelIndex &) { ui->tableResumo->resizeColumnsToContents(); }
