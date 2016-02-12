#include <QDate>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>

#include "checkboxdelegate.h"
#include "inputdialog.h"
#include "sendmail.h"
#include "ui_widgetcompragerar.h"
#include "usersession.h"
#include "widgetcompragerar.h"
#include "xlsxdocument.h"

WidgetCompraGerar::WidgetCompraGerar(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompraGerar) {
  ui->setupUi(this);

  connect(ui->tableProdutos->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this,
          &WidgetCompraGerar::fixPersistente);
}

WidgetCompraGerar::~WidgetCompraGerar() { delete ui; }

void WidgetCompraGerar::setupTables() {
  modelForn.setTable("view_fornecedor_compra");

  modelForn.setHeaderData("fornecedor", "Fornecedor");
  modelForn.setHeaderData("COUNT(fornecedor)", "Itens");

  ui->tableForn->setModel(&modelForn);

  modelProdutos.setTable("pedido_fornecedor_has_produto");
  modelProdutos.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelProdutos.setHeaderData("selecionado", "");
  modelProdutos.setHeaderData("fornecedor", "Fornecedor");
  modelProdutos.setHeaderData("descricao", "Descrição");
  modelProdutos.setHeaderData("colecao", "Coleção");
  modelProdutos.setHeaderData("caixas", "Caixas");
  modelProdutos.setHeaderData("quant", "Quant.");
  modelProdutos.setHeaderData("un", "Un.");
  modelProdutos.setHeaderData("un2", "Un.2");
  modelProdutos.setHeaderData("preco", "Preço");
  modelProdutos.setHeaderData("kgcx", "Kg./Cx.");
  modelProdutos.setHeaderData("formComercial", "Form. Com.");
  modelProdutos.setHeaderData("codComercial", "Cód. Com.");
  modelProdutos.setHeaderData("codBarras", "Cód. Bar.");
  modelProdutos.setHeaderData("dataPrevCompra", "Prev. Compra");
  modelProdutos.setHeaderData("dataCompra", "Data Compra");
  modelProdutos.setHeaderData("status", "Status");

  modelProdutos.setFilter("status = 'PENDENTE'");

  ui->tableProdutos->setModel(&modelProdutos);
  ui->tableProdutos->setItemDelegateForColumn("selecionado", new CheckBoxDelegate(this));
  ui->tableProdutos->hideColumn("idCompra");
  ui->tableProdutos->hideColumn("idNfe");
  ui->tableProdutos->hideColumn("idEstoque");
  ui->tableProdutos->hideColumn("quantUpd");
  ui->tableProdutos->hideColumn("idPedido");
  ui->tableProdutos->hideColumn("idLoja");
  ui->tableProdutos->hideColumn("item");
  ui->tableProdutos->hideColumn("idProduto");
  ui->tableProdutos->hideColumn("prcUnitario");
  ui->tableProdutos->hideColumn("parcial");
  ui->tableProdutos->hideColumn("desconto");
  ui->tableProdutos->hideColumn("parcialDesc");
  ui->tableProdutos->hideColumn("descGlobal");
  ui->tableProdutos->hideColumn("dataRealCompra");
  ui->tableProdutos->hideColumn("dataPrevConf");
  ui->tableProdutos->hideColumn("dataRealConf");
  ui->tableProdutos->hideColumn("dataPrevFat");
  ui->tableProdutos->hideColumn("dataRealFat");
  ui->tableProdutos->hideColumn("dataPrevColeta");
  ui->tableProdutos->hideColumn("dataRealColeta");
  ui->tableProdutos->hideColumn("dataPrevEnt");
  ui->tableProdutos->hideColumn("dataRealEnt");
  ui->tableProdutos->hideColumn("dataPrevReceb");
  ui->tableProdutos->hideColumn("dataRealReceb");
}

QString WidgetCompraGerar::updateTables() {
  if (modelForn.tableName().isEmpty()) setupTables();

  auto selection = ui->tableForn->selectionModel()->selectedRows();

  auto index = selection.size() > 0 ? selection.first() : QModelIndex();

  if (not modelForn.select()) return "Erro lendo tabela fornecedores: " + modelForn.lastError().text();

  ui->tableForn->resizeColumnsToContents();

  modelProdutos.setFilter("0");

  if (not modelProdutos.select()) {
    return "Erro lendo tabela pedido_fornecedor_has_produto: " + modelProdutos.lastError().text();
  }

  if (selection.size() > 0) {
    on_tableForn_activated(index);
    ui->tableForn->selectRow(index.row());
  }

  return QString();
}

void WidgetCompraGerar::on_pushButtonGerarCompra_clicked() {
  // NOTE: refactor this function into smaller functions

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  QList<int> lista;
  QStringList ids;

  for (const auto index :
       modelProdutos.match(modelProdutos.index(0, modelProdutos.fieldIndex("selecionado")), Qt::DisplayRole, true, -1,
                           Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap))) {
    lista.append(index.row());
    ids.append(modelProdutos.data(index.row(), "idPedido").toString());
  }

  if (lista.size() == 0) {
    QMessageBox::critical(this, "Aviso!", "Nenhum item selecionado!");
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QString filtro = modelProdutos.filter();

  InputDialog *inputDlg = new InputDialog(InputDialog::GerarCompra, this);
  inputDlg->setFilter(ids);

  if (inputDlg->exec() != InputDialog::Accepted) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  const QDate dataCompra = inputDlg->getDate();
  const QDate dataPrevista = inputDlg->getNextDate();

  modelProdutos.setFilter(filtro);
  modelProdutos.select();

  QStringList produtos;

  for (const auto row : lista) {
    QString produto = modelProdutos.data(row, "descricao").toString() + ", Quant: " +
                      modelProdutos.data(row, "quant").toString() + ", R$ " +
                      modelProdutos.data(row, "preco").toString().replace(".", ",");
    produtos.append(produto);
  }

  //------------------------------

  QMessageBox msgBox(QMessageBox::Question, "Enviar E-mail?", "Deseja enviar e-mail?",
                     QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Enviar");
  msgBox.setButtonText(QMessageBox::No, "Pular");

  if (msgBox.exec() == QMessageBox::Yes) {
    SendMail *mail = new SendMail(this);

    if (mail->exec() != SendMail::Accepted) {
      QSqlQuery("ROLLBACK").exec();
      return;
    }
  }

  modelProdutos.setFilter(filtro);
  modelProdutos.select();

  QString id;

  for (const auto row : lista) {
    if (not modelProdutos.setData(row, "status", "EM COMPRA")) {
      QMessageBox::critical(this, "Erro!", "Erro marcando status EM COMPRA: " + modelProdutos.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }

    QSqlQuery queryId;

    if (not queryId.exec("SELECT idCompra FROM pedido_fornecedor_has_produto ORDER BY idCompra DESC")) {
      QMessageBox::critical(this, "Erro!", "Erro buscando idCompra: " + queryId.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }

    id = queryId.first() ? QString::number(queryId.value("idCompra").toInt() + 1) : "1";

    if (not modelProdutos.setData(row, "idCompra", id)) {
      QMessageBox::critical(this, "Erro!", "Erro guardando idCompra: " + modelProdutos.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }

    // salvar status na venda
    QSqlQuery query;
    query.prepare("UPDATE venda_has_produto SET idCompra = :idCompra, dataRealCompra = :dataRealCompra, dataPrevConf = "
                  ":dataPrevConf, status = 'EM COMPRA' WHERE idProduto = :idProduto");
    query.bindValue(":idCompra", id);
    query.bindValue(":dataRealCompra", dataCompra);
    query.bindValue(":dataPrevConf", dataPrevista);
    query.bindValue(":idProduto", modelProdutos.data(row, "idProduto"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status da venda: " + query.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }

    if (not query.exec("CALL update_venda_status()")) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }
    //

    if (not modelProdutos.setData(row, "dataRealCompra", dataCompra)) {
      QMessageBox::critical(this, "Erro!", "Erro guardando data da compra: " + modelProdutos.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }

    if (not modelProdutos.setData(row, "dataPrevConf", dataPrevista)) {
      QMessageBox::critical(this, "Erro!", "Erro guardando data prevista: " + modelProdutos.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }
  }

  if (not modelProdutos.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados da tabela pedido_fornecedor_has_produto: " +
                          modelProdutos.lastError().text());
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  updateTables();
}

void WidgetCompraGerar::on_checkBoxMarcarTodos_clicked(const bool &checked) {
  for (int row = 0, rowCount = modelProdutos.rowCount(); row < rowCount; ++row) {
    modelProdutos.setData(row, "selecionado", checked);
  }
}

void WidgetCompraGerar::on_tableForn_activated(const QModelIndex &index) {
  const QString fornecedor = modelForn.data(index.row(), "fornecedor").toString();

  modelProdutos.setFilter("fornecedor = '" + fornecedor + "' AND status = 'PENDENTE'");

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + modelProdutos.lastError().text());
    return;
  }

  fixPersistente();

  ui->tableProdutos->resizeColumnsToContents();
}

void WidgetCompraGerar::fixPersistente() {
  for (int row = 0, rowCount = ui->tableProdutos->model()->rowCount(); row < rowCount; ++row) {
    ui->tableProdutos->openPersistentEditor(row, "selecionado");
  }

  ui->checkBoxMarcarTodos->setChecked(false);
}
