#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "checkboxdelegate.h"
#include "comboboxdelegate.h"
#include "inputdialog.h"
#include "ui_widgetlogisticacoleta.h"
#include "widgetlogisticacoleta.h"

WidgetLogisticaColeta::WidgetLogisticaColeta(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetLogisticaColeta) {
  ui->setupUi(this);

  setupTables();
}

WidgetLogisticaColeta::~WidgetLogisticaColeta() { delete ui; }

QString WidgetLogisticaColeta::updateTables() {
  modelColeta.setFilter("0");

  if (not modelColeta.select()) {
    return "Erro lendo tabela pedido_fornecedor_has_produto: " + modelColeta.lastError().text();
  }

  for (int i = 0; i < modelColeta.rowCount(); ++i) {
    ui->tableColeta->openPersistentEditor(modelColeta.index(i, modelColeta.fieldIndex("selecionado")));
  }

  ui->tableColeta->resizeColumnsToContents();

  return QString();
}

void WidgetLogisticaColeta::TableFornLogistica_activated(const QString &fornecedor) {
  modelColeta.setFilter("fornecedor = '" + fornecedor + "' AND status = 'EM COLETA'");

  if (not modelColeta.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + modelColeta.lastError().text());
    return;
  }

  for (int row = 0; row < modelColeta.rowCount(); ++row) {
    ui->tableColeta->openPersistentEditor(modelColeta.index(row, modelColeta.fieldIndex("selecionado")));
  }

  ui->tableColeta->resizeColumnsToContents();
}

void WidgetLogisticaColeta::setupTables() {
  modelColeta.setTable("pedido_fornecedor_has_produto");
  modelColeta.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelColeta.setHeaderData("selecionado", "");
  modelColeta.setHeaderData("fornecedor", "Fornecedor");
  modelColeta.setHeaderData("descricao", "Descrição");
  modelColeta.setHeaderData("colecao", "Coleção");
  modelColeta.setHeaderData("quant", "Quant.");
  modelColeta.setHeaderData("un", "Un.");
  modelColeta.setHeaderData("preco", "Preço");
  modelColeta.setHeaderData("formComercial", "Form. Com.");
  modelColeta.setHeaderData("codComercial", "Cód. Com.");
  modelColeta.setHeaderData("codBarras", "Cód. Bar.");
  modelColeta.setHeaderData("idCompra", "Compra");
  modelColeta.setHeaderData("dataRealFat", "Data Fat.");
  modelColeta.setHeaderData("dataPrevColeta", "Prev. Coleta");
  modelColeta.setHeaderData("status", "Status");

  modelColeta.setFilter("status = 'EM COLETA'");

  ui->tableColeta->setModel(&modelColeta);
  ui->tableColeta->setItemDelegateForColumn("status", new ComboBoxDelegate(this));
  ui->tableColeta->setItemDelegateForColumn("selecionado", new CheckBoxDelegate(this));
  ui->tableColeta->hideColumn("idPedido");
  ui->tableColeta->hideColumn("idLoja");
  ui->tableColeta->hideColumn("item");
  ui->tableColeta->hideColumn("idProduto");
  ui->tableColeta->hideColumn("prcUnitario");
  ui->tableColeta->hideColumn("parcial");
  ui->tableColeta->hideColumn("desconto");
  ui->tableColeta->hideColumn("parcialDesc");
  ui->tableColeta->hideColumn("descGlobal");
  ui->tableColeta->hideColumn("dataPrevCompra");
  ui->tableColeta->hideColumn("dataRealCompra");
  ui->tableColeta->hideColumn("dataPrevConf");
  ui->tableColeta->hideColumn("dataRealConf");
  ui->tableColeta->hideColumn("dataPrevFat");
  ui->tableColeta->hideColumn("dataRealColeta");
  ui->tableColeta->hideColumn("dataPrevEnt");
  ui->tableColeta->hideColumn("dataRealEnt");
  ui->tableColeta->hideColumn("dataPrevReceb");
  ui->tableColeta->hideColumn("dataRealReceb");
  ui->tableColeta->hideColumn("quantUpd");
}

void WidgetLogisticaColeta::on_pushButtonMarcarColetado_clicked() {
  QList<int> lista;

  for (const auto index :
       modelColeta.match(modelColeta.index(0, modelColeta.fieldIndex("selecionado")), Qt::DisplayRole, true, -1,
                         Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap))) {
    lista.append(index.row());
  }

  if (lista.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  QString filtro = modelColeta.filter();

  InputDialog *inputDlg = new InputDialog(InputDialog::Coleta, this);

  if (inputDlg->exec() != InputDialog::Accepted) return;

  QDate dataColeta = inputDlg->getDate();
  QDate dataPrevista = inputDlg->getNextDate();

  modelColeta.setFilter(filtro);
  modelColeta.select();

  for (const auto row : lista) {
    if (modelColeta.data(row, "status").toString() != "EM COLETA") {
      modelColeta.select();
      QMessageBox::critical(this, "Erro!", "Produto não estava em coleta!");
      return;
    }

    if (not modelColeta.setData(row, "status", "EM RECEBIMENTO")) {
      QMessageBox::critical(this, "Erro!", "Erro marcando status EM RECEBIMENTO: " + modelColeta.lastError().text());
      return;
    }

    // salvar status na venda
    QSqlQuery query;

    query.prepare("UPDATE venda_has_produto SET dataRealColeta = :dataRealColeta, dataPrevReceb = :dataPrevReceb, "
                  "status = 'EM RECEBIMENTO' WHERE idCompra = :idCompra");
    query.bindValue(":dataRealColeta", dataColeta);
    query.bindValue(":dataPrevReceb", dataPrevista);
    query.bindValue(":idCompra", modelColeta.data(row, "idCompra"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status da venda: " + query.lastError().text());
      return;
    }

    if (not query.exec("CALL update_venda_status()")) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
      return;
    }
    //

    if (not modelColeta.setData(row, "dataRealColeta", dataColeta.toString("yyyy-MM-dd"))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando data da coleta: " + modelColeta.lastError().text());
      return;
    }

    if (not modelColeta.setData(row, "dataPrevReceb", dataPrevista.toString("yyyy-MM-dd"))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando data prevista: " + modelColeta.lastError().text());
      return;
    }
  }

  if (not modelColeta.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados da tabela pedido_fornecedor_has_produto: " +
                          modelColeta.lastError().text());
    return;
  }

  updateTables();

  QMessageBox::information(this, "Aviso!", "Confirmado coleta.");
}

void WidgetLogisticaColeta::on_checkBoxMarcarTodos_clicked(const bool &checked) {
  for (int row = 0; row < modelColeta.rowCount(); ++row) {
    modelColeta.setData(row, "selecionado", checked);
  }
}
