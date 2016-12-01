#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

#include "checkboxdelegate.h"
#include "doubledelegate.h"
#include "financeiroproxymodel.h"
#include "inputdialog.h"
#include "ui_widgetlogisticaentrega.h"
#include "usersession.h"
#include "widgetlogisticaentrega.h"

WidgetLogisticaEntrega::WidgetLogisticaEntrega(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetLogisticaEntrega) {
  ui->setupUi(this);

  if (UserSession::tipoUsuario() == "VENDEDOR") {
    ui->tableVendas->hide();
    ui->labelEntregasCliente->hide();
  }

  ui->itemBoxVeiculo->setSearchDialog(SearchDialog::veiculo(this));
}

WidgetLogisticaEntrega::~WidgetLogisticaEntrega() { delete ui; }

void WidgetLogisticaEntrega::setupTables() {
  modelVendas.setTable("view_entrega_pendente");
  modelVendas.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelVendas.setHeaderData("idVenda", "Venda");
  modelVendas.setHeaderData("statusFinanceiro", "Financeiro");
  modelVendas.setHeaderData("prazoEntrega", "Prazo Limite");

  ui->tableVendas->setModel(new FinanceiroProxyModel(&modelVendas, this));
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
  modelProdutos.setHeaderData("dataPrevEnt", "Prev. Ent.");

  modelProdutos.setFilter("0");

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela produtos: " + modelProdutos.lastError().text());
  }

  ui->tableProdutos->setModel(&modelProdutos);
  ui->tableProdutos->hideColumn("idVendaProduto");
  ui->tableProdutos->hideColumn("idProduto");

  //

  modelTransp.setTable("veiculo_has_produto");
  modelTransp.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelTransp.setHeaderData("idVenda", "Venda");
  modelTransp.setHeaderData("status", "Status");
  modelTransp.setHeaderData("produto", "Produto");
  modelTransp.setHeaderData("caixas", "Cx.");
  modelTransp.setHeaderData("quant", "Quant.");
  modelTransp.setHeaderData("un", "Un.");
  modelTransp.setHeaderData("codComercial", "Cód. Com.");
  modelTransp.setHeaderData("fornecedor", "Fornecedor");
  modelTransp.setHeaderData("unCaixa", "Un./Cx.");
  modelTransp.setHeaderData("formComercial", "Form. Com.");

  modelTransp.setFilter("0");

  if (not modelTransp.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela transportadora: " + modelTransp.lastError().text());
    return;
  }

  ui->tableTransp->setModel(&modelTransp);
  ui->tableTransp->hideColumn("id");
  ui->tableTransp->hideColumn("idEstoque");
  ui->tableTransp->hideColumn("idEvento");
  ui->tableTransp->hideColumn("idVeiculo");
  ui->tableTransp->hideColumn("idCompra");
  ui->tableTransp->hideColumn("idVendaProduto");
  ui->tableTransp->hideColumn("idNfeSaida");
  ui->tableTransp->hideColumn("idLoja");
  ui->tableTransp->hideColumn("idProduto");
  ui->tableTransp->hideColumn("obs");
  ui->tableTransp->hideColumn("data");

  //

  modelTransp2.setTable("veiculo_has_produto");
  modelTransp2.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelTransp2.setHeaderData("idEstoque", "Estoque");
  modelTransp2.setHeaderData("idVenda", "Venda");
  modelTransp2.setHeaderData("data", "Agendado");
  modelTransp2.setHeaderData("status", "Status");
  modelTransp2.setHeaderData("produto", "Produto");
  modelTransp2.setHeaderData("caixas", "Cx.");
  modelTransp2.setHeaderData("quant", "Quant.");
  modelTransp2.setHeaderData("un", "Un.");
  modelTransp2.setHeaderData("codComercial", "Cód. Com.");
  modelTransp2.setHeaderData("fornecedor", "Fornecedor");
  modelTransp2.setHeaderData("unCaixa", "Un./Cx.");
  modelTransp2.setHeaderData("formComercial", "Form. Com.");

  modelTransp2.setFilter("0");

  if (not modelTransp2.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela transportadora: " + modelTransp2.lastError().text());
    return;
  }

  ui->tableTransp2->setModel(&modelTransp2);
  ui->tableTransp2->hideColumn("id");
  ui->tableTransp2->hideColumn("idEvento");
  ui->tableTransp2->hideColumn("idVeiculo");
  ui->tableTransp2->hideColumn("idCompra");
  ui->tableTransp2->hideColumn("idVendaProduto");
  ui->tableTransp2->hideColumn("idNfeSaida");
  ui->tableTransp2->hideColumn("idLoja");
  ui->tableTransp2->hideColumn("idProduto");
  ui->tableTransp2->hideColumn("obs");
}

void WidgetLogisticaEntrega::calcularPeso() {
  double peso = 0;

  const auto list = ui->tableProdutos->selectionModel()->selectedRows();

  for (auto const &item : list) {
    QSqlQuery query;
    query.prepare("SELECT kgcx FROM produto WHERE idProduto = :idProduto");
    query.bindValue(":idProduto", modelProdutos.data(item.row(), "idProduto"));

    if (not query.exec() or not query.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando peso do produto: " + query.lastError().text());
      return;
    }

    const double kg = query.value("kgcx").toDouble();
    const double caixas = modelProdutos.data(item.row(), "caixas").toDouble();
    peso += kg * caixas;
  }

  ui->doubleSpinBoxPeso->setValue(peso);
}

bool WidgetLogisticaEntrega::updateTables() {
  if (modelVendas.tableName().isEmpty()) {
    setupTables();

    connect(ui->tableProdutos->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &WidgetLogisticaEntrega::calcularPeso);

    connect(ui->radioButtonEntregaLimpar, &QRadioButton::clicked, this, &WidgetLogisticaEntrega::montaFiltro);
    connect(ui->radioButtonParcialEstoque, &QRadioButton::clicked, this, &WidgetLogisticaEntrega::montaFiltro);
    connect(ui->radioButtonSemEstoque, &QRadioButton::clicked, this, &WidgetLogisticaEntrega::montaFiltro);
    connect(ui->radioButtonTotalEstoque, &QRadioButton::clicked, this, &WidgetLogisticaEntrega::montaFiltro);
    connect(ui->lineEditBusca, &QLineEdit::textChanged, this, &WidgetLogisticaEntrega::montaFiltro);
  }

  if (not modelVendas.select()) {
    emit errorSignal("Erro lendo tabela vendas: " + modelVendas.lastError().text());
    return false;
  }

  if (not modelVendas.select()) {
    emit errorSignal("Erro lendo tabela venda: " + modelVendas.lastError().text());
    return false;
  }

  ui->tableVendas->sortByColumn("prazoEntrega");

  ui->tableVendas->resizeColumnsToContents();

  if (not modelProdutos.select()) {
    emit errorSignal("Erro lendo tabela produtos: " + modelProdutos.lastError().text());
    return false;
  }

  for (int row = 0; row < modelProdutos.rowCount(); ++row) ui->tableProdutos->openPersistentEditor(row, "selecionado");

  ui->tableProdutos->resizeColumnsToContents();

  if (not modelTransp2.select()) {
    emit errorSignal("Erro lendo tabela veiculo: " + modelTransp2.lastError().text());
    return false;
  }

  ui->tableTransp2->resizeColumnsToContents();

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
  if (ui->radioButtonParcialEstoque->isChecked()) filtroCheck = "Estoque > 0 AND (Entregue > 0 OR Outros > 0)";
  if (ui->radioButtonSemEstoque->isChecked()) filtroCheck = "Estoque = 0";

  QString textoBusca = ui->lineEditBusca->text();

  QString filtroBusca = textoBusca.isEmpty()
                            ? ""
                            : "idVenda LIKE '%" + textoBusca + "%' OR Bairro LIKE '%" + textoBusca +
                                  "%' OR Logradouro LIKE '%" + textoBusca + "%' OR Cidade LIKE '%" + textoBusca + "%'";

  if (not filtroCheck.isEmpty() and not filtroBusca.isEmpty()) filtroBusca.prepend(" AND ");

  modelVendas.setFilter(filtroCheck + filtroBusca);

  if (not modelVendas.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + modelVendas.lastError().text());
    return;
  }

  ui->tableVendas->sortByColumn("prazoEntrega");

  ui->tableVendas->resizeColumnsToContents();
}

void WidgetLogisticaEntrega::on_tableProdutos_entered(const QModelIndex &) {
  ui->tableProdutos->resizeColumnsToContents();
}

void WidgetLogisticaEntrega::on_pushButtonAgendarCarga_clicked() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not processRows()) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  updateTables();
  QMessageBox::information(this, "Aviso!", "Confirmado agendamento!");
}

bool WidgetLogisticaEntrega::processRows() {
  if (modelTransp.rowCount() == 0) {
    QMessageBox::critical(this, "Erro!", "Carga vazia!");
    return false;
  }

  InputDialog *inputDlg = new InputDialog(InputDialog::AgendarEntrega, this);

  if (inputDlg->exec() != InputDialog::Accepted) return false;

  const QDateTime dataPrevEnt = inputDlg->getNextDate();

  QSqlQuery query;

  query.exec("SELECT COALESCE(MAX(idEvento), 0) + 1 FROM veiculo_has_produto");
  query.first();

  const int idEvento = query.value(0).toInt();

  for (int row = 0; row < modelTransp.rowCount(); ++row) {
    if (not modelTransp.setData(row, "data", dataPrevEnt)) return false;
    if (not modelTransp.setData(row, "idEvento", idEvento)) return false;

    const int idVendaProduto = modelTransp.data(row, "idVendaProduto").toInt();

    query.prepare("SELECT idVenda, codComercial FROM venda_has_produto WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":idVendaProduto", idVendaProduto);

    if (not query.exec() or not query.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando dados do produto: " + query.lastError().text());
      return false;
    }

    const QString idVenda = query.value("idVenda").toString();
    const QString codComercial = query.value("codComercial").toString();

    // TODO: use idVendaProduto instead of idVenda/codComercial (that may select more than 1 row)
    query.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'ENTREGA AGEND.', dataPrevEnt = :dataPrevEnt "
                  "WHERE idVenda = :idVenda AND codComercial = :codComercial");
    query.bindValue(":dataPrevEnt", dataPrevEnt);
    query.bindValue(":idVenda", idVenda);
    query.bindValue(":codComercial", codComercial);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status da compra: " + query.lastError().text());
      return false;
    }

    query.prepare("UPDATE venda_has_produto SET status = 'ENTREGA AGEND.', dataPrevEnt = :dataPrevEnt WHERE "
                  "idVendaProduto = :idVendaProduto");
    query.bindValue(":dataPrevEnt", dataPrevEnt);
    query.bindValue(":idVendaProduto", idVendaProduto);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando produtos venda: " + query.lastError().text());
      return false;
    }
  }

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    return false;
  }

  if (not modelTransp.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando carga veiculo: " + modelTransp.lastError().text());
    return false;
  }

  return true;
}

bool WidgetLogisticaEntrega::adicionarProduto(const QModelIndexList &list) {
  for (auto const &item : list) {
    const int row = modelTransp.rowCount();
    modelTransp.insertRow(row);

    if (not modelTransp.setData(row, "fornecedor", modelProdutos.data(item.row(), "fornecedor"))) return false;
    if (not modelTransp.setData(row, "unCaixa", modelProdutos.data(item.row(), "unCaixa"))) return false;
    if (not modelTransp.setData(row, "formComercial", modelProdutos.data(item.row(), "formComercial"))) return false;
    if (not modelTransp.setData(row, "idVeiculo", ui->itemBoxVeiculo->value())) return false;
    if (not modelTransp.setData(row, "idVenda", modelProdutos.data(item.row(), "idVenda"))) return false;
    if (not modelTransp.setData(row, "idVendaProduto", modelProdutos.data(item.row(), "idVendaProduto"))) return false;
    if (not modelTransp.setData(row, "produto", modelProdutos.data(item.row(), "produto"))) return false;
    if (not modelTransp.setData(row, "codComercial", modelProdutos.data(item.row(), "codComercial"))) return false;
    if (not modelTransp.setData(row, "un", modelProdutos.data(item.row(), "un"))) return false;
    if (not modelTransp.setData(row, "caixas", modelProdutos.data(item.row(), "caixas"))) return false;
    if (not modelTransp.setData(row, "quant", modelProdutos.data(item.row(), "quant"))) return false;
    if (not modelTransp.setData(row, "status", "ENTREGA AGEND.")) return false;
  }

  ui->tableTransp->resizeColumnsToContents();

  return true;
}

void WidgetLogisticaEntrega::on_pushButtonAdicionarProduto_clicked() {
  if (ui->itemBoxVeiculo->value().isNull()) {
    QMessageBox::critical(this, "Erro!", "Deve escolher uma transportadora antes!");
    return;
  }

  const auto list = ui->tableProdutos->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  if (ui->doubleSpinBoxPeso->value() > ui->doubleSpinBoxCapacidade->value()) {
    QMessageBox::critical(this, "Erro!", "Peso maior que capacidade do veículo!");
    return;
  }

  for (auto const &item : list) {
    if (modelProdutos.data(item.row(), "status").toString() != "ESTOQUE") {
      QMessageBox::critical(this, "Erro!", "Produto não está em ESTOQUE!");
      return;
    }

    if (not modelProdutos.data(item.row(), "dataPrevEnt").isNull()) {
      QMessageBox::critical(this, "Erro!", "Produto já agendado!");
      return;
    }
  }

  if (not adicionarProduto(list)) modelTransp.select();
}

void WidgetLogisticaEntrega::on_pushButtonRemoverProduto_clicked() {
  const auto list = ui->tableTransp->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  for (auto const &item : list) modelTransp.removeRow(item.row());

  modelTransp.submitAll();
}

void WidgetLogisticaEntrega::on_itemBoxVeiculo_textChanged(const QString &) {
  QSqlQuery query;
  query.prepare("SELECT * FROM transportadora_has_veiculo WHERE idVeiculo = :idVeiculo");
  query.bindValue(":idVeiculo", ui->itemBoxVeiculo->value());

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados veiculo: " + query.lastError().text());
    return;
  }

  modelTransp2.setFilter("idVeiculo = " + ui->itemBoxVeiculo->value().toString() + " AND status != 'FINALIZADO'");

  if (not modelTransp2.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela veiculo: " + modelTransp2.lastError().text());
    return;
  }

  ui->tableTransp2->resizeColumnsToContents();

  ui->doubleSpinBoxCapacidade->setValue(query.value("capacidade").toDouble());
}

void WidgetLogisticaEntrega::on_tableTransp2_entered(const QModelIndex &) {
  ui->tableTransp2->resizeColumnsToContents();
}
