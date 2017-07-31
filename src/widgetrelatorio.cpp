#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>

#include "porcentagemdelegate.h"
#include "reaisdelegate.h"
#include "ui_widgetrelatorio.h"
#include "usersession.h"
#include "widgetrelatorio.h"
#include <xlsxdocument.h>

WidgetRelatorio::WidgetRelatorio(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetRelatorio) { ui->setupUi(this); }

WidgetRelatorio::~WidgetRelatorio() { delete ui; }

void WidgetRelatorio::setFilterTotaisVendedor() {
  QString filter;

  if (UserSession::tipoUsuario() == "VENDEDOR" or UserSession::tipoUsuario() == "VENDEDOR ESPECIAL") {
    filter = "Mês = '" + ui->dateEditMes->date().toString("yyyy-MM") + "' AND idUsuario = " + QString::number(UserSession::idUsuario()) + " ORDER BY Loja, Vendedor";
  } else if (UserSession::tipoUsuario() == "GERENTE LOJA") {
    filter = "Mês = '" + ui->dateEditMes->date().toString("yyyy-MM") + "' AND Loja = '" + UserSession::fromLoja("descricao") + "' ORDER BY Loja, Vendedor";
  } else {
    filter = "Mês = '" + ui->dateEditMes->date().toString("yyyy-MM") + "' ORDER BY Loja, Vendedor";
  }

  modelTotalVendedor.setFilter(filter);

  if (not modelTotalVendedor.select()) {
    emit errorSignal("Erro lendo tabela relatorio_vendedor: " + modelTotalVendedor.lastError().text());
  }
}

void WidgetRelatorio::setFilterTotaisLoja() {
  QString filter;

  if (UserSession::tipoUsuario() == "GERENTE LOJA") {
    filter = "Mês = '" + ui->dateEditMes->date().toString("yyyy-MM") + "' AND Loja = '" + UserSession::fromLoja("descricao") + "' ORDER BY Loja";
  } else {
    filter = "Mês = '" + ui->dateEditMes->date().toString("yyyy-MM") + "' ORDER BY Loja";
  }

  modelTotalLoja.setFilter(filter);

  if (not modelTotalLoja.select()) {
    emit errorSignal("Erro lendo tabela relatorio_loja: " + modelTotalLoja.lastError().text());
  }
}

bool WidgetRelatorio::setupTables() {
  modelRelatorio.setTable("view_relatorio");
  modelRelatorio.setEditStrategy(QSqlTableModel::OnManualSubmit);

  setFilterRelatorio();

  if (not modelRelatorio.select()) {
    emit errorSignal("Erro lendo tabela relatorio: " + modelRelatorio.lastError().text());
    return false;
  }

  ui->tableRelatorio->setModel(&modelRelatorio);
  ui->tableRelatorio->setItemDelegateForColumn("Faturamento", new ReaisDelegate(this));
  ui->tableRelatorio->setItemDelegateForColumn("Valor Comissão", new ReaisDelegate(this));
  ui->tableRelatorio->setItemDelegateForColumn("% Comissão", new PorcentagemDelegate(this));
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
  ui->tableTotalVendedor->setItemDelegateForColumn("Faturamento", new ReaisDelegate(this));
  ui->tableTotalVendedor->setItemDelegateForColumn("Valor Comissão", new ReaisDelegate(this));
  ui->tableTotalVendedor->setItemDelegateForColumn("% Comissão", new PorcentagemDelegate(this));
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
  ui->tableTotalLoja->setItemDelegateForColumn("Faturamento", new ReaisDelegate(this));
  ui->tableTotalLoja->setItemDelegateForColumn("Valor Comissão", new ReaisDelegate(this));
  ui->tableTotalLoja->setItemDelegateForColumn("% Comissão", new PorcentagemDelegate(this));
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
  ui->doubleSpinBoxPorcentagemComissao->setValue(porcentagem / modelTotalLoja.rowCount());
}

void WidgetRelatorio::setFilterRelatorio() {
  const QString date = ui->dateEditMes->date().toString("yyyy-MM");
  QString filter;

  if (UserSession::tipoUsuario() == "VENDEDOR" or UserSession::tipoUsuario() == "VENDEDOR ESPECIAL") {
    filter = "Mês = '" + date + "' AND idUsuario = " + QString::number(UserSession::idUsuario()) + " ORDER BY Loja, Vendedor, idVenda";
  } else if (UserSession::tipoUsuario() == "GERENTE LOJA") {
    filter = "Mês = '" + date + "' AND Loja = '" + UserSession::fromLoja("descricao") + "' ORDER BY Loja, Vendedor, idVenda";
  } else {
    filter = "Mês = '" + date + "' ORDER BY Loja, Vendedor, idVenda";
  }

  modelRelatorio.setFilter(filter);

  if (not modelRelatorio.select()) {
    emit errorSignal("Erro lendo tabela relatorio: " + modelRelatorio.lastError().text());
  }
}

void WidgetRelatorio::on_dateEditMes_dateChanged(const QDate &) { updateTables(); }

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
    emit errorSignal("Erro setando mydate: " + query.lastError().text());
    return false;
  }

  query.exec("SELECT @mydate");
  query.first();

  modelOrcamento.setTable("view_resumo_relatorio");
  modelOrcamento.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (UserSession::tipoUsuario() == "GERENTE LOJA") {
    modelOrcamento.setFilter("Loja = '" + UserSession::fromLoja("descricao") + "' ORDER BY Loja, Vendedor");
  }

  if (not modelOrcamento.select()) {
    emit errorSignal("Erro lendo view_resumo_relatorio: " + modelOrcamento.lastError().text());
    return false;
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

  ui->tableRelatorio->resizeColumnsToContents();
  ui->tableTotalVendedor->resizeColumnsToContents();
  ui->tableTotalLoja->resizeColumnsToContents();

  return true;
}

void WidgetRelatorio::on_tableTotalLoja_entered(const QModelIndex &) { ui->tableTotalLoja->resizeColumnsToContents(); }

void WidgetRelatorio::on_tableTotalVendedor_entered(const QModelIndex &) { ui->tableTotalVendedor->resizeColumnsToContents(); }

void WidgetRelatorio::on_pushButtonExcel_clicked() {
  const QString dir = QFileDialog::getExistingDirectory(this, "Pasta para salvar relatório");

  if (dir.isEmpty()) return;

  const QString arquivoModelo = "relatorio.xlsx";

  QFile modelo(QDir::currentPath() + "/" + arquivoModelo);

  if (not modelo.exists()) {
    QMessageBox::critical(this, "Erro!", "Não encontrou o modelo do Excel!");
    return;
  }

  const QString fileName = dir + "/relatorio-" + ui->dateEditMes->date().toString("MM-yyyy") + ".xlsx";

  QFile file(fileName);

  if (not file.open(QFile::WriteOnly)) {
    QMessageBox::critical(this, "Erro!", "Não foi possível abrir o arquivo para escrita: " + fileName);
    QMessageBox::critical(this, "Erro!", "Erro: " + file.errorString());
    return;
  }

  file.close();

  QXlsx::Document xlsx(arquivoModelo);

  xlsx.selectSheet("Sheet1");

  char column = 'A';

  for (int col = 0; col < modelRelatorio.columnCount(); ++col, ++column) {
    xlsx.write(column + QString::number(1), modelRelatorio.headerData(col, Qt::Horizontal).toString());
  }

  column = 'A';

  for (int row = 0; row < modelRelatorio.rowCount(); ++row) {
    for (int col = 0; col < modelRelatorio.columnCount(); ++col, ++column) {
      xlsx.write(column + QString::number(row + 2), modelRelatorio.data(row, col));
    }

    column = 'A';
  }

  xlsx.selectSheet("Sheet2");

  for (int col = 0; col < modelTotalVendedor.columnCount(); ++col, ++column) {
    xlsx.write(column + QString::number(1), modelTotalVendedor.headerData(col, Qt::Horizontal).toString());
  }

  column = 'A';

  for (int row = 0; row < modelTotalVendedor.rowCount(); ++row) {
    for (int col = 0; col < modelTotalVendedor.columnCount(); ++col, ++column) {
      xlsx.write(column + QString::number(row + 2), modelTotalVendedor.data(row, col));
    }

    column = 'A';
  }

  if (not xlsx.saveAs(fileName)) {
    QMessageBox::critical(this, "Erro!", "Ocorreu algum erro ao salvar o arquivo.");
    return;
  }

  QMessageBox::information(this, "Ok!", "Arquivo salvo como " + fileName);
  QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}
