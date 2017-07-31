#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>

#include "doubledelegate.h"
#include "estoque.h"
#include "importarxml.h"
#include "ui_widgetestoque.h"
#include "widgetestoque.h"
#include "xlsxdocument.h"
#include "xml.h"

WidgetEstoque::WidgetEstoque(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetEstoque) {
  ui->setupUi(this);

  connect(ui->lineEditBusca, &QLineEdit::textChanged, this, &WidgetEstoque::montaFiltro);
  connect(ui->radioButtonIgual, &QRadioButton::toggled, this, &WidgetEstoque::montaFiltro);
  connect(ui->radioButtonMaior, &QRadioButton::toggled, this, &WidgetEstoque::montaFiltro);
  connect(ui->radioButtonEstoque, &QRadioButton::toggled, this, &WidgetEstoque::montaFiltro);

  ui->dateEditMes->setDate(QDate::currentDate());
}

WidgetEstoque::~WidgetEstoque() { delete ui; }

void WidgetEstoque::setupTables() {
  model.setTable("view_estoque");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  model.setHeaderData("cnpjDest", "CNPJ");
  model.setHeaderData("status", "Status");
  model.setHeaderData("idEstoque", "Estoque");
  model.setHeaderData("fornecedor", "Fornecedor");
  model.setHeaderData("descricao", "Produto");
  model.setHeaderData("restante", "Quant. Rest.");
  model.setHeaderData("restante est", "Quant. Dep.");
  model.setHeaderData("unEst", "Un. Est.");
  model.setHeaderData("unProd", "Un. Prod.");
  //  model.setHeaderData("unProd2", "Un. Prod. 2");
  model.setHeaderData("lote", "Lote");
  model.setHeaderData("local", "Local");
  model.setHeaderData("bloco", "Bloco");
  model.setHeaderData("codComercial", "Cód. Com.");
  model.setHeaderData("nfe", "NFe");
  model.setHeaderData("dataPrevColeta", "Prev. Coleta");
  model.setHeaderData("dataRealColeta", "Coleta");
  model.setHeaderData("dataPrevReceb", "Prev. Receb.");
  model.setHeaderData("dataRealReceb", "Receb.");
  model.setHeaderData("dataPrevEnt", "Prev. Ent.");
  model.setHeaderData("dataRealEnt", "Entrega");

  model.setFilter("restante > 0");

  ui->table->setModel(&model);
  ui->table->hideColumn("idCompra");
  ui->table->hideColumn("restante est");
  ui->table->setItemDelegate(new DoubleDelegate(this));
}

bool WidgetEstoque::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela estoque: " + model.lastError().text());
    return false;
  }

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetEstoque::on_table_activated(const QModelIndex &index) {
  auto *estoque = new Estoque(this);
  estoque->setAttribute(Qt::WA_DeleteOnClose);
  estoque->viewRegisterById(model.data(index.row(), "idEstoque").toString());
}

void WidgetEstoque::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetEstoque::montaFiltro() {
  ui->radioButtonEstoque->isChecked() ? ui->table->showColumn("restante est") : ui->table->hideColumn("restante est");

  const QString text = ui->lineEditBusca->text();

  const QString restante = ui->radioButtonEstoque->isChecked() ? "`restante est` > 0 AND status = 'ESTOQUE'" : ui->radioButtonIgual->isChecked() ? "restante <= 0" : "restante > 0";

  model.setFilter("(idEstoque LIKE '%" + text + "%' OR fornecedor LIKE '%" + text + "%' OR descricao LIKE '%" + text + "%' OR codComercial LIKE '%" + text + "%' OR cnpjDest LIKE '%" + text +
                  "%') AND " + restante);

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
}

void WidgetEstoque::on_pushButtonRelatorio_clicked() {
  // TODO: para filtrar corretamente pela data refazer a view para considerar o `restante est` na data desejada e nao na
  // atual
  // TODO: adicionar caixas

  // 1. codigo produto
  // 2. descricao
  // 3. ncm
  // 4. unidade
  // 5. quantidade
  // 6. valor
  // 7. aliquota icms (se tiver)

  SqlTableModel modelContabil;
  modelContabil.setTable("view_estoque_contabil");
  modelContabil.setEditStrategy(SqlTableModel::OnManualSubmit);

  modelContabil.setFilter("dataRealReceb < '" + ui->dateEditMes->date().toString("yyyy-MM") + "'");

  modelContabil.select();

  const QString dir = QFileDialog::getExistingDirectory(this, "Pasta para salvar relatório");

  if (dir.isEmpty()) return;

  const QString arquivoModelo = "relatorio.xlsx";

  QFile modelo(QDir::currentPath() + "/" + arquivoModelo);

  if (not modelo.exists()) {
    QMessageBox::critical(this, "Erro!", "Não encontrou o modelo do Excel!");
    return;
  }

  //  const QString fileName = dir + "/relatorio-" + ui->dateEditMes->date().toString("MM-yyyy") + ".xlsx";
  const QString fileName = dir + "/relatorio_contabil.xlsx";

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

  for (int col = 0; col < modelContabil.columnCount(); ++col, ++column) {
    xlsx.write(column + QString::number(1), modelContabil.headerData(col, Qt::Horizontal).toString());
  }

  column = 'A';

  for (int row = 0; row < modelContabil.rowCount(); ++row) {
    for (int col = 0; col < modelContabil.columnCount(); ++col, ++column) {
      xlsx.write(column + QString::number(row + 2), modelContabil.data(row, col));
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

// NOTE: gerenciar lugares de estoque (cadastro/permissoes)
// TODO: 3tem produto com unidade barra que na verdade significa ML

// TODO: colocar um filtro para mostrar os cancelados/quebrados?
// TODO: 2poder trocar bloco do estoque
// TODO: -1verificar se o custo do pedido_fornecedor bate com os valores do estoque/consumo
