#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

#include "doubledelegate.h"
#include "reaisdelegate.h"
#include "ui_widgetfluxocaixa.h"
#include "widgetfluxocaixa.h"

WidgetFluxoCaixa::WidgetFluxoCaixa(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetFluxoCaixa) {
  ui->setupUi(this);
}

WidgetFluxoCaixa::~WidgetFluxoCaixa() { delete ui; }

void WidgetFluxoCaixa::setupTables() {
  modelCaixa.setTable("view_fluxo_caixa");
  ui->tableCaixa->setModel(&modelCaixa);
  ui->tableCaixa->setItemDelegateForColumn("R$", new ReaisDelegate(this));

  ui->tableCaixa->hideColumn("Código");
  ui->tableCaixa->hideColumn("contaDestino");
  ui->tableCaixa->hideColumn("idConta");
  ui->tableCaixa->hideColumn("Tipo");

  modelResumo.setTable("view_fluxo_resumo");
  ui->tableResumo->setModel(&modelResumo);
  ui->tableResumo->setItemDelegateForColumn("SAIDA", new ReaisDelegate(this));
  ui->tableResumo->setItemDelegateForColumn("ENTRADA", new ReaisDelegate(this));
  ui->tableResumo->setItemDelegateForColumn("R$", new ReaisDelegate(this));

  ui->tableResumo->hideColumn("Conta");
  ui->tableResumo->hideColumn("idConta");
}

bool WidgetFluxoCaixa::updateTables() {
  if (modelCaixa.tableName().isEmpty()) {
    setupTables();

    ui->dateEdit->setDate(QDate::currentDate());
    montaFiltro();

    connect(ui->groupBoxMes, &QGroupBox::toggled, this, &WidgetFluxoCaixa::montaFiltro);
    connect(ui->dateEdit, &QDateEdit::dateChanged, this, &WidgetFluxoCaixa::montaFiltro);
    connect(ui->itemBox1, &ItemBox::textChanged, this, &WidgetFluxoCaixa::montaFiltro);
    connect(ui->itemBox2, &ItemBox::textChanged, this, &WidgetFluxoCaixa::montaFiltro);
    connect(ui->groupBoxContas1, &QGroupBox::toggled, this, &WidgetFluxoCaixa::montaFiltro);
    connect(ui->groupBoxContas2, &QGroupBox::toggled, this, &WidgetFluxoCaixa::montaFiltro);

    ui->itemBox1->setSearchDialog(SearchDialog::conta(this));
    ui->itemBox2->setSearchDialog(SearchDialog::conta(this));
  }

  return montaFiltro();
}

bool WidgetFluxoCaixa::montaFiltro() {
  const QString filtroData =
      ui->groupBoxMes->isChecked()
          ? "DATE_FORMAT(`Data Pag`, '%Y-%m') = '" + ui->dateEdit->date().toString("yyyy-MM") + "'"
          : "";

  const QString conta =
      ui->groupBoxContas1->isChecked() and ui->itemBox1->getValue().isValid()
          ? QString(filtroData.isEmpty() ? "" : " AND ") + "idConta = " + ui->itemBox1->getValue().toString()
          : "";

  modelCaixa.setFilter(filtroData + conta);

  if (not modelCaixa.select()) {
    emit errorSignal("Erro lendo tabela Caixa: " + modelCaixa.lastError().text());
    return false;
  }

  const QString conta2 =
      ui->groupBoxContas2->isChecked() and ui->itemBox2->getValue().isValid()
          ? QString(filtroData.isEmpty() ? "" : " AND ") + "idConta = " + ui->itemBox2->getValue().toString()
          : "";

  modelResumo.setFilter(filtroData + conta2);

  if (not modelResumo.select()) {
    emit errorSignal("Erro lendo tabela Resumo: " + modelResumo.lastError().text());
    return false;
  }

  return true;
}

void WidgetFluxoCaixa::on_tableCaixa_entered(const QModelIndex &) { ui->tableCaixa->resizeColumnsToContents(); }

void WidgetFluxoCaixa::on_tableResumo_entered(const QModelIndex &) { ui->tableResumo->resizeColumnsToContents(); }

// TODO: 1nao mostrar pendentes
// TODO: 1mostrar apenas entrada/saida/saldo
// TODO: 1terceira tabela futuro
