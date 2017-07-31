#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

#include "doubledelegate.h"
#include "ui_widgetlogisticaentregues.h"
#include "usersession.h"
#include "vendaproxymodel.h"
#include "widgetlogisticaentregues.h"

WidgetLogisticaEntregues::WidgetLogisticaEntregues(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetLogisticaEntregues) { ui->setupUi(this); }

WidgetLogisticaEntregues::~WidgetLogisticaEntregues() { delete ui; }

bool WidgetLogisticaEntregues::updateTables() {
  if (modelVendas.tableName().isEmpty()) {
    setupTables();

    connect(ui->radioButtonEntregaLimpar, &QRadioButton::clicked, this, &WidgetLogisticaEntregues::montaFiltro);
    connect(ui->radioButtonParcialEntrega, &QRadioButton::clicked, this, &WidgetLogisticaEntregues::montaFiltro);
    connect(ui->radioButtonSemEntrega, &QRadioButton::clicked, this, &WidgetLogisticaEntregues::montaFiltro);
    connect(ui->radioButtonTotalEntrega, &QRadioButton::clicked, this, &WidgetLogisticaEntregues::montaFiltro);
    connect(ui->lineEditBusca, &QLineEdit::textChanged, this, &WidgetLogisticaEntregues::montaFiltro);
  }

  if (not modelVendas.select()) {
    emit errorSignal("Erro lendo tabela vendas: " + modelVendas.lastError().text());
    return false;
  }

  ui->tableVendas->sortByColumn("prazoEntrega");

  ui->tableVendas->resizeColumnsToContents();

  if (not modelProdutos.select()) {
    emit errorSignal("Erro lendo tabela produtos: " + modelProdutos.lastError().text());
    return false;
  }

  ui->tableProdutos->resizeColumnsToContents();

  return true;
}

void WidgetLogisticaEntregues::montaFiltro() {
  QString filtroCheck;

  if (ui->radioButtonEntregaLimpar->isChecked()) filtroCheck = "";
  if (ui->radioButtonTotalEntrega->isChecked()) filtroCheck = "Entregue > 0 AND Estoque = 0 AND Outros = 0";
  if (ui->radioButtonParcialEntrega->isChecked()) filtroCheck = "Entregue > 0 AND (Estoque > 0 OR Outros > 0)";
  if (ui->radioButtonSemEntrega->isChecked()) filtroCheck = "Entregue = 0";

  const QString textoBusca = ui->lineEditBusca->text();

  QString filtroBusca = textoBusca.isEmpty() ? "" : "idVenda LIKE '%" + textoBusca + "%'";

  if (not filtroCheck.isEmpty() and not filtroBusca.isEmpty()) filtroBusca.prepend(" AND ");

  modelVendas.setFilter(filtroCheck + filtroBusca);

  ui->tableVendas->resizeColumnsToContents();
}

void WidgetLogisticaEntregues::setupTables() {
  modelVendas.setTable("view_entrega");
  modelVendas.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelVendas.setHeaderData("idVenda", "Venda");
  modelVendas.setHeaderData("prazoEntrega", "Prazo Limite");

  ui->tableVendas->setModel(new VendaProxyModel(&modelVendas, this));
  ui->tableVendas->setItemDelegate(new DoubleDelegate(this));

  if (UserSession::tipoUsuario() != "VENDEDOR ESPECIAL") ui->tableVendas->hideColumn("Indicou");

  modelProdutos.setTable("view_entrega_produtos");
  modelProdutos.setEditStrategy(QSqlTableModel::OnManualSubmit);

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
  modelProdutos.setHeaderData("dataRealEnt", "Data Ent.");

  modelProdutos.setFilter("0");

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela produtos: " + modelProdutos.lastError().text());
  }

  ui->tableProdutos->setModel(&modelProdutos);
  ui->tableProdutos->hideColumn("idVendaProduto");
  ui->tableProdutos->hideColumn("idProduto");
  ui->tableProdutos->hideColumn("dataPrevEnt");
}

void WidgetLogisticaEntregues::on_tableVendas_clicked(const QModelIndex &index) {
  modelProdutos.setFilter("idVenda = '" + modelVendas.data(index.row(), "idVenda").toString() + "'");

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + modelProdutos.lastError().text());
  }

  for (int row = 0; row < modelProdutos.rowCount(); ++row) ui->tableProdutos->openPersistentEditor(row, "selecionado");

  ui->tableProdutos->resizeColumnsToContents();
}

void WidgetLogisticaEntregues::on_pushButtonCancelar_clicked() {
  const auto list = ui->tableProdutos->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhuma linha selecionada!");
    return;
  }

  QMessageBox msgBox(QMessageBox::Question, "Cancelar?", "Tem certeza que deseja cancelar?", QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Cancelar");
  msgBox.setButtonText(QMessageBox::No, "Voltar");

  if (msgBox.exec() == QMessageBox::No) return;

  // desfazer passos da confirmacao de entrega (volta para tela de confirmar entrega)

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not cancelar(list)) {
    QSqlQuery("ROLLBACK").exec();
    if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
    return;
  }

  QSqlQuery("COMMIT").exec();

  QSqlQuery query;

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    return;
  }

  updateTables();
  QMessageBox::information(this, "Aviso!", "Entrega cancelada! Produto voltou para 'EM ENTREGA'!");
}

bool WidgetLogisticaEntregues::cancelar(const QModelIndexList &list) {
  // TODO: testar essa funcao
  // TODO: dataPrevEnt nao foi limpa
  QSqlQuery query;

  for (const auto item : list) {
    // TODO: cancelar no lugar de voltar para em entrega?
    query.prepare("UPDATE veiculo_has_produto SET status = 'EM ENTREGA' WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":idVendaProduto", modelProdutos.data(item.row(), "idVendaProduto"));

    if (not query.exec()) {
      error = "Erro atualizando veiculo_produto: " + query.lastError().text();
      return false;
    }

    query.prepare("UPDATE venda_has_produto SET status = 'ESTOQUE', entregou = NULL, recebeu = NULL, "
                  "dataRealEnt = NULL WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":idVendaProduto", modelProdutos.data(item.row(), "idVendaProduto"));

    if (not query.exec()) {
      error = "Erro atualizando venda_produto: " + query.lastError().text();
      return false;
    }
  }

  return true;
}

// TODO: mostrar quem entregou/recebeu
