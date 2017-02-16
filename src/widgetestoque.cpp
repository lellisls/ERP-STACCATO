#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>

#include "doubledelegate.h"
#include "estoque.h"
#include "importarxml.h"
#include "ui_widgetestoque.h"
#include "widgetestoque.h"
#include "xml.h"

WidgetEstoque::WidgetEstoque(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetEstoque) {
  ui->setupUi(this);

  connect(ui->lineEditBusca, &QLineEdit::textChanged, this, &WidgetEstoque::montaFiltro);
  connect(ui->radioButtonIgual, &QRadioButton::toggled, this, &WidgetEstoque::montaFiltro);
  connect(ui->radioButtonMaior, &QRadioButton::toggled, this, &WidgetEstoque::montaFiltro);
  connect(ui->radioButtonEstoque, &QRadioButton::toggled, this, &WidgetEstoque::montaFiltro);
}

WidgetEstoque::~WidgetEstoque() { delete ui; }

void WidgetEstoque::setupTables() {
  model.setTable("view_estoque");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  model.setHeaderData("status", "Status");
  model.setHeaderData("idEstoque", "Estoque");
  model.setHeaderData("fornecedor", "Fornecedor");
  model.setHeaderData("descricao", "Produto");
  model.setHeaderData("restante", "Quant. Rest.");
  model.setHeaderData("restante est", "Quant. Dep.");
  model.setHeaderData("un", "Un.");
  model.setHeaderData("un2", "Un.2");
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
  Estoque *estoque = new Estoque(this);
  estoque->setAttribute(Qt::WA_DeleteOnClose);
  estoque->viewRegisterById(model.data(index.row(), "idEstoque").toString());
}

void WidgetEstoque::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetEstoque::montaFiltro() {
  ui->radioButtonEstoque->isChecked() ? ui->table->showColumn("restante est") : ui->table->hideColumn("restante est");

  const QString text = ui->lineEditBusca->text();

  const QString restante = ui->radioButtonEstoque->isChecked()
                               ? "`restante est` > 0"
                               : ui->radioButtonIgual->isChecked() ? "restante <= 0" : "restante > 0";

  model.setFilter("(idEstoque LIKE '%" + text + "%' OR descricao LIKE '%" + text + "%' OR codComercial LIKE '%" + text +
                  "%') AND " + restante);

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
}

// NOTE: gerenciar lugares de estoque (cadastro/permissoes)
// TODO: 3tem produto com unidade barra que na verdade significa ML
// TODO: 3ajustar estoques com quant negativa
// TODO: 3mudar status do pedido_fornecedor para terminar em 'estoque'
// TODO: 3recalcular caixas para pedido_fornecedor, estoque e estoque_consumo

// estoque 1545 ?
