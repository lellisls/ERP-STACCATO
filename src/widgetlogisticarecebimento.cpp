#include <QDate>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "checkboxdelegate.h"
#include "comboboxdelegate.h"
#include "inputdialog.h"
#include "ui_widgetlogisticarecebimento.h"
#include "widgetlogisticarecebimento.h"

WidgetLogisticaRecebimento::WidgetLogisticaRecebimento(QWidget *parent)
  : QWidget(parent), ui(new Ui::WidgetLogisticaRecebimento) {
  ui->setupUi(this);

  setupTables();
}

WidgetLogisticaRecebimento::~WidgetLogisticaRecebimento() { delete ui; }

QString WidgetLogisticaRecebimento::updateTables() {
  if (not modelReceb.select()) {
    return "Erro lendo tabela pedido_fornecedor_has_produto: " + modelReceb.lastError().text();
  }

  for (int i = 0; i < modelReceb.rowCount(); ++i) {
    ui->tableRecebimento->openPersistentEditor(modelReceb.index(i, modelReceb.fieldIndex("selecionado")));
  }

  ui->tableRecebimento->resizeColumnsToContents();

  return QString();
}

void WidgetLogisticaRecebimento::TableFornLogistica_activated(const QString &fornecedor) {
  modelReceb.setFilter("fornecedor = '" + fornecedor + "' AND status = 'EM RECEBIMENTO'");

  if (not modelReceb.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + modelReceb.lastError().text());
    return;
  }
}

void WidgetLogisticaRecebimento::setupTables() {
  modelReceb.setTable("pedido_fornecedor_has_produto");
  modelReceb.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelReceb.setHeaderData("selecionado", "");
  modelReceb.setHeaderData("fornecedor", "Fornecedor");
  modelReceb.setHeaderData("descricao", "Descrição");
  modelReceb.setHeaderData("colecao", "Coleção");
  modelReceb.setHeaderData("quant", "Quant.");
  modelReceb.setHeaderData("un", "Un.");
  modelReceb.setHeaderData("preco", "Preço");
  modelReceb.setHeaderData("formComercial", "Form. Com.");
  modelReceb.setHeaderData("codComercial", "Cód. Com.");
  modelReceb.setHeaderData("codBarras", "Cód. Bar.");
  modelReceb.setHeaderData("idCompra", "Compra");
  modelReceb.setHeaderData("dataRealColeta", "Data Coleta");
  modelReceb.setHeaderData("dataPrevReceb", "Prev. Receb.");
  modelReceb.setHeaderData("status", "Status");

  modelReceb.setFilter("status = 'EM RECEBIMENTO'");

  ui->tableRecebimento->setModel(&modelReceb);
  ui->tableRecebimento->setItemDelegateForColumn("status", new ComboBoxDelegate(this));
  ui->tableRecebimento->setItemDelegateForColumn("selecionado", new CheckBoxDelegate(this));
  ui->tableRecebimento->hideColumn("idPedido");
  ui->tableRecebimento->hideColumn("idLoja");
  ui->tableRecebimento->hideColumn("item");
  ui->tableRecebimento->hideColumn("idProduto");
  ui->tableRecebimento->hideColumn("prcUnitario");
  ui->tableRecebimento->hideColumn("parcial");
  ui->tableRecebimento->hideColumn("desconto");
  ui->tableRecebimento->hideColumn("parcialDesc");
  ui->tableRecebimento->hideColumn("descGlobal");
  ui->tableRecebimento->hideColumn("dataPrevCompra");
  ui->tableRecebimento->hideColumn("dataRealCompra");
  ui->tableRecebimento->hideColumn("dataPrevConf");
  ui->tableRecebimento->hideColumn("dataRealConf");
  ui->tableRecebimento->hideColumn("dataPrevFat");
  ui->tableRecebimento->hideColumn("dataRealFat");
  ui->tableRecebimento->hideColumn("dataPrevEnt");
  ui->tableRecebimento->hideColumn("dataRealEnt");
  ui->tableRecebimento->hideColumn("dataPrevColeta");
  ui->tableRecebimento->hideColumn("dataRealReceb");
  ui->tableRecebimento->hideColumn("quantUpd");
}

void WidgetLogisticaRecebimento::on_pushButtonMarcarRecebido_clicked() {
  QList<int> lista;

  for (const auto index : modelReceb.match(modelReceb.index(0, modelReceb.fieldIndex("selecionado")), Qt::DisplayRole,
                                           true, -1, Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap))) {
    lista.append(index.row());
  }

  if (lista.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  InputDialog *inputDlg = new InputDialog(InputDialog::Recebimento, this);

  if (inputDlg->exec() != InputDialog::Accepted) return;

  const QDate dataReceb = inputDlg->getDate();

  for (const auto row : lista) {
    modelReceb.setData(row, "selecionado", false);

    if (modelReceb.data(row, "status").toString() != "EM RECEBIMENTO") {
      modelReceb.select();
      QMessageBox::critical(this, "Erro!", "Produto não estava em recebimento!");
      return;
    }

    if (not modelReceb.setData(row, "status", "ESTOQUE")) {
      QMessageBox::critical(this, "Erro!", "Erro marcando status ESTOQUE: " + modelReceb.lastError().text());
      return;
    }

    // salvar status na venda
    QSqlQuery query;

    query.prepare(
          "UPDATE venda_has_produto SET dataRealReceb = :dataRealReceb, status = 'ESTOQUE' WHERE idCompra = :idCompra");
    query.bindValue(":dataRealReceb", dataReceb);
    query.bindValue(":idCompra", modelReceb.data(row, "idCompra"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status da venda: " + query.lastError().text());
      return;
    }

    if (not query.exec("CALL update_venda_status()")) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
      return;
    }
    //

    if (not modelReceb.setData(row, "dataRealReceb", dataReceb.toString("yyyy-MM-dd"))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando data de recebimento: " + modelReceb.lastError().text());
      return;
    }

    query.prepare("UPDATE estoque SET status = 'RECEBIDO' WHERE idCompra = :idCompra AND idVendaProduto = 0");
    query.bindValue(":idCompra", modelReceb.data(row, "idCompra"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status do estoque: " + query.lastError().text());
      return;
    }
  }

  if (not modelReceb.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados da tabela pedido_fornecedor_has_produto: " +
                          modelReceb.lastError().text());
    return;
  }

  updateTables();

  QMessageBox::information(this, "Aviso!", "Confirmado recebimento.");
}

void WidgetLogisticaRecebimento::on_checkBoxMarcarTodos_clicked(const bool &checked) {
  for (int row = 0; row < modelReceb.rowCount(); ++row) {
    modelReceb.setData(row, "selecionado", checked);
  }
}
