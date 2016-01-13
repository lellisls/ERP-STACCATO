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
  if (not model.select()) {
    return "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text();
  }

  for (int row = 0; row < model.rowCount(); ++row) {
    ui->table->openPersistentEditor(model.index(row, model.fieldIndex("selecionado")));
  }

  ui->table->resizeColumnsToContents();

  return QString();
}

void WidgetLogisticaRecebimento::TableFornLogistica_activated(const QString &fornecedor) {
  model.setFilter("fornecedor = '" + fornecedor + "' AND status = 'EM RECEBIMENTO'");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return;
  }
}

void WidgetLogisticaRecebimento::setupTables() {
  model.setTable("pedido_fornecedor_has_produto");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  model.setHeaderData("selecionado", "");
  model.setHeaderData("fornecedor", "Fornecedor");
  model.setHeaderData("descricao", "Descrição");
  model.setHeaderData("colecao", "Coleção");
  model.setHeaderData("quant", "Quant.");
  model.setHeaderData("un", "Un.");
  model.setHeaderData("preco", "Preço");
  model.setHeaderData("formComercial", "Form. Com.");
  model.setHeaderData("codComercial", "Cód. Com.");
  model.setHeaderData("codBarras", "Cód. Bar.");
  model.setHeaderData("idCompra", "Compra");
  model.setHeaderData("dataRealColeta", "Data Coleta");
  model.setHeaderData("dataPrevReceb", "Prev. Receb.");
  model.setHeaderData("status", "Status");

  model.setFilter("status = 'EM RECEBIMENTO'");

  ui->table->setModel(&model);
  ui->table->setItemDelegateForColumn("status", new ComboBoxDelegate(this));
  ui->table->setItemDelegateForColumn("selecionado", new CheckBoxDelegate(this));
  ui->table->hideColumn("idPedido");
  ui->table->hideColumn("idLoja");
  ui->table->hideColumn("item");
  ui->table->hideColumn("idProduto");
  ui->table->hideColumn("prcUnitario");
  ui->table->hideColumn("parcial");
  ui->table->hideColumn("desconto");
  ui->table->hideColumn("parcialDesc");
  ui->table->hideColumn("descGlobal");
  ui->table->hideColumn("dataPrevCompra");
  ui->table->hideColumn("dataRealCompra");
  ui->table->hideColumn("dataPrevConf");
  ui->table->hideColumn("dataRealConf");
  ui->table->hideColumn("dataPrevFat");
  ui->table->hideColumn("dataRealFat");
  ui->table->hideColumn("dataPrevEnt");
  ui->table->hideColumn("dataRealEnt");
  ui->table->hideColumn("dataPrevColeta");
  ui->table->hideColumn("dataRealReceb");
  ui->table->hideColumn("quantUpd");
}

void WidgetLogisticaRecebimento::on_pushButtonMarcarRecebido_clicked() {
  QList<int> lista;

  for (const auto index : model.match(model.index(0, model.fieldIndex("selecionado")), Qt::DisplayRole, true, -1,
                                      Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap))) {
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
    model.setData(row, "selecionado", false);

    if (model.data(row, "status").toString() != "EM RECEBIMENTO") {
      model.select();
      QMessageBox::critical(this, "Erro!", "Produto não estava em recebimento!");
      return;
    }

    if (not model.setData(row, "status", "ESTOQUE")) {
      QMessageBox::critical(this, "Erro!", "Erro marcando status ESTOQUE: " + model.lastError().text());
      return;
    }

    // salvar status na venda
    QSqlQuery query;

    query.prepare(
          "UPDATE venda_has_produto SET dataRealReceb = :dataRealReceb, status = 'ESTOQUE' WHERE idCompra = :idCompra");
    query.bindValue(":dataRealReceb", dataReceb);
    query.bindValue(":idCompra", model.data(row, "idCompra"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status da venda: " + query.lastError().text());
      return;
    }

    if (not query.exec("CALL update_venda_status()")) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
      return;
    }
    //

    if (not model.setData(row, "dataRealReceb", dataReceb.toString("yyyy-MM-dd"))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando data de recebimento: " + model.lastError().text());
      return;
    }

    query.prepare("UPDATE estoque SET status = 'RECEBIDO' WHERE idCompra = :idCompra AND idVendaProduto = 0");
    query.bindValue(":idCompra", model.data(row, "idCompra"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status do estoque: " + query.lastError().text());
      return;
    }
  }

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro salvando dados da tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return;
  }

  updateTables();

  QMessageBox::information(this, "Aviso!", "Confirmado recebimento.");
}

void WidgetLogisticaRecebimento::on_checkBoxMarcarTodos_clicked(const bool &checked) {
  for (int row = 0; row < model.rowCount(); ++row) {
    model.setData(row, "selecionado", checked);
  }
}
