#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

#include "checkboxdelegate.h"
#include "doubledelegate.h"
#include "entregascliente.h"
#include "inputdialog.h"
#include "orcamentoproxymodel.h"
#include "ui_widgetlogisticaentrega.h"
#include "usersession.h"
#include "widgetlogisticaentrega.h"

WidgetLogisticaEntrega::WidgetLogisticaEntrega(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetLogisticaEntrega) {
  ui->setupUi(this);

  if (UserSession::tipoUsuario() == "VENDEDOR") {
    ui->tableVendas->hide();
    ui->labelEntregasCliente->hide();
  }

  ui->frameCaminhao->hide();
}

WidgetLogisticaEntrega::~WidgetLogisticaEntrega() { delete ui; }

void WidgetLogisticaEntrega::setupTables() {
  modelVendas.setTable("view_entrega_base");
  modelVendas.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelVendas.setHeaderData("idVenda", "Venda");

  ui->tableVendas->setModel(new OrcamentoProxyModel(&modelVendas, this));
  ui->tableVendas->setItemDelegate(new DoubleDelegate(this));

  if (UserSession::tipoUsuario() != "VENDEDOR ESPECIAL") ui->tableVendas->hideColumn("Indicou");

  // TODO: dont allow sorting (break checkboxDelegate) or redo delegates

  modelProdutos.setTable("venda_has_produto");
  modelProdutos.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelProdutos.setHeaderData("selecionado", "");
  modelProdutos.setHeaderData("status", "Status");
  modelProdutos.setHeaderData("fornecedor", "Fornecedor");
  modelProdutos.setHeaderData("idVenda", "Venda");
  modelProdutos.setHeaderData("produto", "Produto");
  modelProdutos.setHeaderData("caixas", "Caixas");
  modelProdutos.setHeaderData("quant", "Quant.");
  modelProdutos.setHeaderData("un", "Un.");
  modelProdutos.setHeaderData("unCaixa", "Un./Cx.");
  modelProdutos.setHeaderData("codComercial", "Cód. Com.");
  modelProdutos.setHeaderData("formComercial", "Form. Com.");
  modelProdutos.setHeaderData("dataPrevEnt", "Prev. Ent.");

  ui->tableProdutos->setModel(&modelProdutos);
  ui->tableProdutos->hideColumn("selecionado");
  ui->tableProdutos->hideColumn("idVendaProduto");
  ui->tableProdutos->hideColumn("idNFeSaida");
  ui->tableProdutos->hideColumn("idLoja");
  ui->tableProdutos->hideColumn("idProduto");
  ui->tableProdutos->hideColumn("obs");
  ui->tableProdutos->hideColumn("prcUnitario");
  ui->tableProdutos->hideColumn("descUnitario");
  ui->tableProdutos->hideColumn("parcial");
  ui->tableProdutos->hideColumn("desconto");
  ui->tableProdutos->hideColumn("parcialDesc");
  ui->tableProdutos->hideColumn("descGlobal");
  ui->tableProdutos->hideColumn("total");
  ui->tableProdutos->hideColumn("idCompra");
  ui->tableProdutos->hideColumn("estoque_promocao");
  ui->tableProdutos->hideColumn("dataPrevCompra");
  ui->tableProdutos->hideColumn("dataRealCompra");
  ui->tableProdutos->hideColumn("dataPrevConf");
  ui->tableProdutos->hideColumn("dataRealConf");
  ui->tableProdutos->hideColumn("dataPrevFat");
  ui->tableProdutos->hideColumn("dataRealFat");
  ui->tableProdutos->hideColumn("dataPrevColeta");
  ui->tableProdutos->hideColumn("dataRealColeta");
  ui->tableProdutos->hideColumn("dataPrevReceb");
  ui->tableProdutos->hideColumn("dataRealReceb");
  ui->tableProdutos->hideColumn("dataRealEnt");
  //  ui->tableProdutos->setItemDelegateForColumn("selecionado", new CheckBoxDelegate(this));
}

bool WidgetLogisticaEntrega::updateTables() {
  if (modelVendas.tableName().isEmpty()) {
    setupTables();

    connect(ui->radioButtonEntregaLimpar, &QRadioButton::clicked, this, &WidgetLogisticaEntrega::montaFiltro);
    connect(ui->radioButtonParcialEstoque, &QRadioButton::clicked, this, &WidgetLogisticaEntrega::montaFiltro);
    connect(ui->radioButtonSemEstoque, &QRadioButton::clicked, this, &WidgetLogisticaEntrega::montaFiltro);
    connect(ui->radioButtonTotalEstoque, &QRadioButton::clicked, this, &WidgetLogisticaEntrega::montaFiltro);
    connect(ui->lineEditBusca, &QLineEdit::textChanged, this, &WidgetLogisticaEntrega::montaFiltro);
  }

  const QString filter = modelVendas.filter();

  if (not modelVendas.select()) {
    emit errorSignal("Erro lendo tabela vendas: " + modelVendas.lastError().text());
    return false;
  }

  modelVendas.setFilter(filter);

  ui->tableVendas->resizeColumnsToContents();

  for (int row = 0; row < modelProdutos.rowCount(); ++row) ui->tableProdutos->openPersistentEditor(row, "selecionado");

  const QString filter2 = modelProdutos.filter();

  if (not modelProdutos.select()) {
    emit errorSignal("Erro lendo tabela produtos: " + modelProdutos.lastError().text());
    return false;
  }

  modelProdutos.setFilter(filter2);

  ui->tableProdutos->resizeColumnsToContents();

  return true;
}

void WidgetLogisticaEntrega::on_tableVendas_entered(const QModelIndex &) { ui->tableVendas->resizeColumnsToContents(); }

void WidgetLogisticaEntrega::on_tableVendas_clicked(const QModelIndex &index) {
  modelProdutos.setFilter("idVenda = '" + modelVendas.data(index.row(), "idVenda").toString() + "'");

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + modelProdutos.lastError().text());
  }

  for (int row = 0; row < modelProdutos.rowCount(); ++row) ui->tableProdutos->openPersistentEditor(row, "selecionado");

  ui->tableProdutos->resizeColumnsToContents();
}

void WidgetLogisticaEntrega::montaFiltro() {
  QString filtroCheck;

  if (ui->radioButtonEntregaLimpar->isChecked()) filtroCheck = "";
  if (ui->radioButtonTotalEstoque->isChecked()) filtroCheck = "Estoque > 0 AND Entregue >= 0 AND Outros = 0";
  if (ui->radioButtonParcialEstoque->isChecked()) filtroCheck = "Estoque > 0 AND Entregue >= 0";
  if (ui->radioButtonSemEstoque->isChecked()) filtroCheck = "Estoque = 0";

  QString textoBusca = ui->lineEditBusca->text();

  QString filtroBusca = textoBusca.isEmpty() ? "" : "idVenda LIKE '%" + textoBusca + "%'";

  if (not filtroCheck.isEmpty() and not filtroBusca.isEmpty()) filtroBusca.prepend(" AND ");

  modelVendas.setFilter(filtroCheck + filtroBusca);

  ui->tableVendas->resizeColumnsToContents();
}

void WidgetLogisticaEntrega::on_tableProdutos_entered(const QModelIndex &) {
  ui->tableProdutos->resizeColumnsToContents();
}

void WidgetLogisticaEntrega::on_pushButtonAgendar_clicked() {
  // 1. Escolher produtos e montar carga
  // 2. Marcar data prevista
  // 3. Na tela seguinte é gerada a NFe

  // por hora apenas marcar data prevista

  auto list = ui->tableProdutos->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not processRows(list)) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  QMessageBox::information(this, "Aviso!", "Confirmado agendamento!");
  updateTables();
}

bool WidgetLogisticaEntrega::processRows(QModelIndexList list) {
  // TODO: avisar se status diferente de 'ESTOQUE'

  const QString filtro = modelProdutos.filter();

  InputDialog *inputDlg = new InputDialog(InputDialog::AgendarEntrega, this);

  if (inputDlg->exec() != InputDialog::Accepted) return false;

  const QDate dataPrevEnt = inputDlg->getNextDate();

  modelProdutos.setFilter(filtro);
  modelProdutos.select();

  QSqlQuery query;

  for (auto const &item : list) {
    query.prepare("UPDATE pedido_fornecedor_has_produto SET dataPrevEnt = :dataPrevEnt WHERE idVenda = :idVenda AND "
                  "codComercial = :codComercial");
    query.bindValue(":dataPrevEnt", dataPrevEnt);
    query.bindValue(":idVenda", modelProdutos.data(item.row(), "idVenda"));
    query.bindValue(":codComercial", modelProdutos.data(item.row(), "codComercial"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status da compra: " + query.lastError().text());
      return false;
    }

    // salvar status na venda
    query.prepare("UPDATE venda_has_produto SET dataPrevEnt = :dataPrevEnt WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":dataPrevEnt", dataPrevEnt);
    query.bindValue(":idVendaProduto", modelProdutos.data(item.row(), "idVendaProduto"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando produtos venda: " + query.lastError().text());
      return false;
    }
  }

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    return false;
  }

  return true;
}

// TODO: dataPrevEnt e dataRealEnt deve guardar hora tambem
