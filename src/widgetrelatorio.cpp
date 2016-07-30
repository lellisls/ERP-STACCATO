#include <QMessageBox>
#include <QSqlError>

#include "porcentagemdelegate.h"
#include "reaisdelegate.h"
#include "ui_widgetrelatorio.h"
#include "usersession.h"
#include "widgetrelatorio.h"

WidgetRelatorio::WidgetRelatorio(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetRelatorio) { ui->setupUi(this); }

WidgetRelatorio::~WidgetRelatorio() { delete ui; }

void WidgetRelatorio::setFilterTotaisVendedor() {
  if (UserSession::tipoUsuario() == "VENDEDOR" or UserSession::tipoUsuario() == "VENDEDOR ESPECIAL") {
    modelTotalVendedor.setFilter("Mês = '" + ui->dateEditMes->date().toString("yyyy-MM") + "' AND idUsuario = " +
                                 QString::number(UserSession::idUsuario()) + " ORDER BY Loja, Vendedor");
  } else if (UserSession::tipoUsuario() == "GERENTE LOJA") {
    modelTotalVendedor.setFilter("Mês = '" + ui->dateEditMes->date().toString("yyyy-MM") + "' AND Loja = '" +
                                 UserSession::fromLoja("descricao") + "' ORDER BY Loja, Vendedor");
  } else {
    modelTotalVendedor.setFilter("Mês = '" + ui->dateEditMes->date().toString("yyyy-MM") + "' ORDER BY Loja, Vendedor");
  }
}

void WidgetRelatorio::setFilterTotaisLoja() {
  if (UserSession::tipoUsuario() == "GERENTE LOJA") {
    modelTotalLoja.setFilter("Mês = '" + ui->dateEditMes->date().toString("yyyy-MM") + "' AND Loja = '" +
                             UserSession::fromLoja("descricao") + "' ORDER BY Loja");
  } else {
    modelTotalLoja.setFilter("Mês = '" + ui->dateEditMes->date().toString("yyyy-MM") + "' ORDER BY Loja");
  }
}

bool WidgetRelatorio::setupTables() {
  ReaisDelegate *reaisDelegate = new ReaisDelegate(this);
  PorcentagemDelegate *porcentagemDelegate = new PorcentagemDelegate(this);

  modelRelatorio.setTable("view_relatorio");
  modelRelatorio.setEditStrategy(QSqlTableModel::OnManualSubmit);

  setFilterRelatorio();

  if (not modelRelatorio.select()) {
    emit errorSignal("Erro lendo tabela relatorio: " + modelRelatorio.lastError().text());
    return false;
  }

  ui->tableRelatorio->setModel(&modelRelatorio);
  ui->tableRelatorio->setItemDelegateForColumn("Faturamento", reaisDelegate);
  ui->tableRelatorio->setItemDelegateForColumn("Valor Comissão", reaisDelegate);
  ui->tableRelatorio->setItemDelegateForColumn("% Comissão", porcentagemDelegate);
  ui->tableRelatorio->hideColumn("Mês");
  ui->tableRelatorio->hideColumn("idUsuario");
  ui->tableRelatorio->resizeColumnsToContents();

  //------------------------------------------------------------------------------

  modelTotalVendedor.setTable("view_relatorio_vendedor");
  modelTotalVendedor.setEditStrategy(QSqlTableModel::OnManualSubmit);

  setFilterTotaisVendedor();

  if (not modelTotalVendedor.select()) {
    emit errorSignal("Erro lendo view_relatorio_vendedor: " + modelTotalVendedor.lastError().text());
    return false;
  }

  ui->tableTotalVendedor->setModel(&modelTotalVendedor);
  ui->tableTotalVendedor->setItemDelegateForColumn("Faturamento", reaisDelegate);
  ui->tableTotalVendedor->setItemDelegateForColumn("Valor Comissão", reaisDelegate);
  ui->tableTotalVendedor->setItemDelegateForColumn("% Comissão", porcentagemDelegate);
  ui->tableTotalVendedor->hideColumn("idUsuario");
  ui->tableTotalVendedor->hideColumn("Mês");
  ui->tableTotalVendedor->resizeColumnsToContents();

  //--------------------------------------------------

  modelTotalLoja.setTable("view_relatorio_loja");
  modelTotalLoja.setEditStrategy(QSqlTableModel::OnManualSubmit);

  setFilterTotaisLoja();

  if (not modelTotalLoja.select()) {
    emit errorSignal("Erro lendo view_relatorio_vendedor: " + modelTotalLoja.lastError().text());
    return false;
  }

  ui->tableTotalLoja->setModel(&modelTotalLoja);
  ui->tableTotalLoja->setItemDelegateForColumn("Faturamento", reaisDelegate);
  ui->tableTotalLoja->setItemDelegateForColumn("Valor Comissão", reaisDelegate);
  ui->tableTotalLoja->setItemDelegateForColumn("% Comissão", porcentagemDelegate);
  ui->tableTotalLoja->hideColumn("Mês");
  ui->tableTotalLoja->resizeColumnsToContents();

  return true;
}

void WidgetRelatorio::calcularTotalGeral() {
  double totalGeral = 0;
  double comissao = 0;
  double porcentagem = 0;

  for (int row = 0; row < modelTotalLoja.rowCount(); ++row) {
    totalGeral += modelTotalLoja.data(row, "Faturamento").toDouble();
    comissao += modelTotalLoja.data(row, "Valor Comissão").toDouble();
    porcentagem += modelTotalLoja.data(row, "% Comissão").toDouble();
  }

  ui->doubleSpinBoxGeral->setValue(totalGeral);
  ui->doubleSpinBoxValorComissao->setValue(comissao);
  ui->doubleSpinBoxPorcentagemComissao->setValue(porcentagem * 100 / modelTotalLoja.rowCount());
}

void WidgetRelatorio::setFilterRelatorio() {
  QDate date = ui->dateEditMes->date();

  if (UserSession::tipoUsuario() == "VENDEDOR" or UserSession::tipoUsuario() == "VENDEDOR ESPECIAL") {
    modelRelatorio.setFilter("Mês = '" + date.toString("yyyy-MM") + "' AND idUsuario = " +
                             QString::number(UserSession::idUsuario()) + " ORDER BY Loja, Vendedor, idVenda");
  } else if (UserSession::tipoUsuario() == "GERENTE LOJA") {
    modelRelatorio.setFilter("Mês = '" + date.toString("yyyy-MM") + "' AND Loja = '" +
                             UserSession::fromLoja("descricao") + "' ORDER BY Loja, Vendedor, idVenda");
  } else {
    modelRelatorio.setFilter("Mês = '" + date.toString("yyyy-MM") + "' ORDER BY Loja, Vendedor, idVenda");
  }
}

void WidgetRelatorio::on_dateEditMes_dateChanged(const QDate &) { updateTables(); }

void WidgetRelatorio::updateTables2() {
  setFilterRelatorio();
  setFilterTotaisVendedor();
  setFilterTotaisLoja();

  ui->tableRelatorio->resizeColumnsToContents();
  ui->tableTotalVendedor->resizeColumnsToContents();
  ui->tableTotalLoja->resizeColumnsToContents();

  calcularTotalGeral();

  QSqlQuery query;
  query.prepare("SET @mydate = :mydate");
  query.bindValue(":mydate", ui->dateEditMes->date().toString("yyyy-MM"));

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro setando mydate: " + query.lastError().text());
  }

  query.exec("SELECT @mydate");
  query.first();

  modelOrcamento.setTable("view_resumo_relatorio");
  modelOrcamento.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (UserSession::tipoUsuario() == "GERENTE LOJA") {
    modelOrcamento.setFilter("Loja = '" + UserSession::fromLoja("descricao") + "' ORDER BY Loja, Vendedor");
  }

  if (not modelOrcamento.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo view_resumo_relatorio: " + modelOrcamento.lastError().text());
  }

  ui->tableResumoOrcamento->setModel(&modelOrcamento);
  ui->tableResumoOrcamento->setItemDelegateForColumn("Validos Anteriores", new ReaisDelegate(this));
  ui->tableResumoOrcamento->setItemDelegateForColumn("Gerados Mes", new ReaisDelegate(this));
  ui->tableResumoOrcamento->setItemDelegateForColumn("Revalidados Mes", new ReaisDelegate(this));
  ui->tableResumoOrcamento->setItemDelegateForColumn("Fechados Mes", new ReaisDelegate(this));
  ui->tableResumoOrcamento->setItemDelegateForColumn("Perdidos Mes", new ReaisDelegate(this));
  ui->tableResumoOrcamento->setItemDelegateForColumn("Validos Mes", new ReaisDelegate(this));
  ui->tableResumoOrcamento->setItemDelegateForColumn("% Fechados / Gerados", new PorcentagemDelegate(this));
  ui->tableResumoOrcamento->setItemDelegateForColumn("% Fechados / Carteira", new PorcentagemDelegate(this));
  ui->tableResumoOrcamento->resizeColumnsToContents();
}

void WidgetRelatorio::on_tableRelatorio_entered(const QModelIndex &) { ui->tableRelatorio->resizeColumnsToContents(); }

bool WidgetRelatorio::updateTables() {
  if (modelRelatorio.tableName().isEmpty()) {
    if (UserSession::tipoUsuario() == "VENDEDOR" or UserSession::tipoUsuario() == "VENDEDOR ESPECIAL") {
      ui->labelTotalLoja->hide();
      ui->tableTotalLoja->hide();
      ui->labelGeral->hide();
      ui->doubleSpinBoxGeral->hide();
      ui->groupBoxResumoOrcamento->hide();
    }

    ui->dateEditMes->setDate(QDate::currentDate());

    if (not setupTables()) return false;

    calcularTotalGeral();
  }

  // TODO: refactor
  updateTables2();

  ui->tableRelatorio->resizeColumnsToContents();
  ui->tableTotalVendedor->resizeColumnsToContents();
  ui->tableTotalLoja->resizeColumnsToContents();

  return true;
}

void WidgetRelatorio::on_tableTotalLoja_entered(const QModelIndex &) { ui->tableTotalLoja->resizeColumnsToContents(); }

void WidgetRelatorio::on_tableTotalVendedor_entered(const QModelIndex &) {
  ui->tableTotalVendedor->resizeColumnsToContents();
}
