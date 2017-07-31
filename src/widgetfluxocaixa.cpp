#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include "pagamentosdia.h"
#include "reaisdelegate.h"
#include "ui_widgetfluxocaixa.h"
#include "widgetfluxocaixa.h"

WidgetFluxoCaixa::WidgetFluxoCaixa(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetFluxoCaixa) { ui->setupUi(this); }

WidgetFluxoCaixa::~WidgetFluxoCaixa() { delete ui; }

bool WidgetFluxoCaixa::updateTables() {
  if (not ui->tableCaixa->model()) {
    ui->dateEdit->setDate(QDate::currentDate());

    connect(ui->groupBoxMes, &QGroupBox::toggled, this, &WidgetFluxoCaixa::montaFiltro);
    connect(ui->dateEdit, &QDateEdit::dateChanged, this, &WidgetFluxoCaixa::montaFiltro);
    connect(ui->itemBoxCaixa1, &ItemBox::textChanged, this, &WidgetFluxoCaixa::montaFiltro);
    connect(ui->itemBoxCaixa2, &ItemBox::textChanged, this, &WidgetFluxoCaixa::montaFiltro);

    ui->itemBoxCaixa1->setSearchDialog(SearchDialog::conta(this));
    ui->itemBoxCaixa2->setSearchDialog(SearchDialog::conta(this));

    // TODO: dont hardcode magic numbers
    ui->itemBoxCaixa1->setValue(3);
    ui->itemBoxCaixa2->setValue(8);

    ui->groupBoxCaixa1->setChecked(true);
    ui->groupBoxCaixa2->setChecked(true);

    isReady = true;
  }

  montaFiltro();

  return true;
}

void WidgetFluxoCaixa::montaFiltro() {
  if (not isReady) return;

  const QString filtroData = ui->groupBoxMes->isChecked() ? "`Data` IS NOT NULL AND DATE_FORMAT(`Data`, '%Y-%m') = '" + ui->dateEdit->date().toString("yyyy-MM") + "'" : "`Data` IS NOT NULL";

  const QString filtroConta = ui->groupBoxCaixa1->isChecked() and ui->itemBoxCaixa1->getValue().isValid() ? "idConta = " + ui->itemBoxCaixa1->getValue().toString() : "";

  if (filtroConta.isEmpty()) {
    modelCaixa.setQuery("SELECT * FROM (SELECT v.*, @running_total := @running_total + COALESCE(v.`R$`, 0) AS "
                        "Acumulado FROM view_fluxo_resumo v JOIN (SELECT @running_total := 0) r WHERE `Data` IS NOT "
                        "NULL ORDER BY Data, idConta) x WHERE " +
                        filtroData);
  } else {
    modelCaixa.setQuery("SELECT * FROM (SELECT v.*, @running_total := @running_total + COALESCE(v.`R$`, 0) AS Acumulado FROM "
                        "view_fluxo_resumo2 v JOIN (SELECT @running_total := 0) r WHERE " +
                        filtroConta + " AND `Data` IS NOT NULL ORDER BY Data) x WHERE " + filtroData);
  }

  ui->tableCaixa->setModel(&modelCaixa);
  ui->tableCaixa->setItemDelegateForColumn("SAIDA", new ReaisDelegate(this));
  ui->tableCaixa->setItemDelegateForColumn("ENTRADA", new ReaisDelegate(this));
  ui->tableCaixa->setItemDelegateForColumn("R$", new ReaisDelegate(this));
  ui->tableCaixa->setItemDelegateForColumn("Acumulado", new ReaisDelegate(this));

  ui->tableCaixa->showColumn("SAIDA");

  ui->tableCaixa->hideColumn("Conta");
  ui->tableCaixa->hideColumn("idConta");
  ui->tableCaixa->hideColumn("Data Pag");

  // calcular saldo

  QSqlQuery query;

  if (not query.exec(modelCaixa.query().executedQuery() + " ORDER BY DATA DESC LIMIT 1") or not query.first()) {
    emit errorSignal("Erro buscando saldo: " + query.lastError().text());
    return;
  }

  ui->doubleSpinBoxSaldo1->setValue(query.value("Acumulado").toDouble());

  // ----------------------------------------------------------------------------------------------------------

  const QString filtroConta2 = ui->groupBoxCaixa2->isChecked() and ui->itemBoxCaixa2->getValue().isValid() ? "idConta = " + ui->itemBoxCaixa2->getValue().toString() : "";

  if (filtroConta2.isEmpty()) {
    modelCaixa2.setQuery("SELECT * FROM (SELECT v.*, @running_total := @running_total + COALESCE(v.`R$`, 0) AS "
                         "Acumulado FROM view_fluxo_resumo v JOIN (SELECT @running_total := 0) r WHERE `Data` IS NOT "
                         "NULL ORDER BY Data, idConta) x WHERE " +
                         filtroData);
  } else {
    modelCaixa2.setQuery("SELECT * FROM (SELECT v.*, @running_total := @running_total + COALESCE(v.`R$`, 0) AS Acumulado FROM "
                         "view_fluxo_resumo2 v JOIN (SELECT @running_total := 0) r WHERE " +
                         filtroConta2 + " AND `Data` IS NOT NULL ORDER BY Data) x WHERE " + filtroData);
  }

  ui->tableCaixa2->setModel(&modelCaixa2);
  ui->tableCaixa2->setItemDelegateForColumn("SAIDA", new ReaisDelegate(this));
  ui->tableCaixa2->setItemDelegateForColumn("ENTRADA", new ReaisDelegate(this));
  ui->tableCaixa2->setItemDelegateForColumn("R$", new ReaisDelegate(this));
  ui->tableCaixa2->setItemDelegateForColumn("Acumulado", new ReaisDelegate(this));

  ui->tableCaixa2->showColumn("SAIDA");

  ui->tableCaixa2->hideColumn("Conta");
  ui->tableCaixa2->hideColumn("idConta");
  ui->tableCaixa2->hideColumn("Data Pag");

  // calcular saldo

  if (not query.exec(modelCaixa2.query().executedQuery() + " ORDER BY DATA DESC LIMIT 1") or not query.first()) {
    emit errorSignal("Erro buscando saldo: " + query.lastError().text());
    return;
  }

  ui->doubleSpinBoxSaldo2->setValue(query.value("Acumulado").toDouble());

  // ----------------------------------------------------------------------------------------------------------

  modelFuturo.setQuery(
      // TODO: fix empty first row
      "SELECT v.*, @running_total := @running_total + COALESCE(v.`R$`, 0) AS Acumulado FROM view_fluxo_resumo3 v "
      "JOIN (SELECT @running_total := 0) r");

  ui->tableFuturo->setModel(&modelFuturo);
  ui->tableFuturo->setItemDelegateForColumn("SAIDA", new ReaisDelegate(this));
  ui->tableFuturo->setItemDelegateForColumn("ENTRADA", new ReaisDelegate(this));
  ui->tableFuturo->setItemDelegateForColumn("R$", new ReaisDelegate(this));
  ui->tableFuturo->setItemDelegateForColumn("Acumulado", new ReaisDelegate(this));

  ui->tableFuturo->hideColumn("idConta");
}

void WidgetFluxoCaixa::on_tableCaixa_entered(const QModelIndex &) { ui->tableCaixa->resizeColumnsToContents(); }

void WidgetFluxoCaixa::on_tableCaixa2_entered(const QModelIndex &) { ui->tableCaixa2->resizeColumnsToContents(); }

void WidgetFluxoCaixa::on_tableCaixa2_activated(const QModelIndex &index) {
  const QDate date = modelCaixa2.data(modelCaixa2.index(index.row(), modelCaixa2.record().indexOf("Data"))).toDate();
  const QString idConta = modelCaixa2.data(modelCaixa2.index(index.row(), modelCaixa2.record().indexOf("idConta"))).toString();

  auto *dia = new PagamentosDia(this);
  dia->setFilter(date, idConta);
  dia->show();
}

void WidgetFluxoCaixa::on_tableCaixa_activated(const QModelIndex &index) {
  const QDate date = modelCaixa.data(modelCaixa.index(index.row(), modelCaixa.record().indexOf("Data"))).toDate();
  const QString idConta = modelCaixa.data(modelCaixa.index(index.row(), modelCaixa.record().indexOf("idConta"))).toString();

  auto *dia = new PagamentosDia(this);
  dia->setFilter(date, idConta);
  dia->show();
}

void WidgetFluxoCaixa::on_groupBoxCaixa1_toggled(const bool checked) {
  if (not checked) {
    ui->itemBoxCaixa1->clear();
    return;
  }

  montaFiltro();
}

void WidgetFluxoCaixa::on_groupBoxCaixa2_toggled(const bool checked) {
  if (not checked) {
    ui->itemBoxCaixa2->clear();
    return;
  }

  montaFiltro();
}

// TODO: nao agrupar contas no view_fluxo_resumo (apenas quando filtrado)
// TODO: fazer delegate para reduzir tamanho da fonte
