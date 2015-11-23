#include "widgetlogistica.h"
#include "ui_widgetlogistica.h"
#include "entregascliente.h"
#include "checkboxdelegate.h"
#include "comboboxdelegate.h"
#include "doubledelegate.h"
#include "usersession.h"
#include "inputdialog.h"

#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

WidgetLogistica::WidgetLogistica(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetLogistica) {
  ui->setupUi(this);

  setupTables();

  ui->splitter_6->setStretchFactor(0, 0);
  ui->splitter_6->setStretchFactor(1, 1);

  if (UserSession::getTipoUsuario() == "VENDEDOR") {
    ui->tableEntregasCliente->hide();
    ui->labelEntregasCliente->hide();
  }
}

WidgetLogistica::~WidgetLogistica() { delete ui; }

void WidgetLogistica::setupTables() {
  DoubleDelegate *doubledelegate = new DoubleDelegate(this);

  // Fornecedores Logística --------------------------------------------------------------------------------------------
  modelPedForn2 = new SqlTableModel(this);
  modelPedForn2->setTable("view_fornecedor_logistica");

  modelPedForn2->setHeaderData("fornecedor", "Fornecedor");
  modelPedForn2->setHeaderData("COUNT(fornecedor)", "Itens");

  ui->tableFornLogistica->setModel(modelPedForn2);

  // Coletas -----------------------------------------------------------------------------------------------------------
  modelColeta = new SqlTableModel(this);
  modelColeta->setTable("pedido_fornecedor_has_produto");
  modelColeta->setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelColeta->setHeaderData("selecionado", "");
  modelColeta->setHeaderData("fornecedor", "Fornecedor");
  modelColeta->setHeaderData("descricao", "Descrição");
  modelColeta->setHeaderData("colecao", "Coleção");
  modelColeta->setHeaderData("quant", "Quant.");
  modelColeta->setHeaderData("un", "Un.");
  modelColeta->setHeaderData("preco", "Preço");
  modelColeta->setHeaderData("formComercial", "Form. Com.");
  modelColeta->setHeaderData("codComercial", "Cód. Com.");
  modelColeta->setHeaderData("codBarras", "Cód. Bar.");
  modelColeta->setHeaderData("idCompra", "Compra");
  modelColeta->setHeaderData("dataRealFat", "Data Fat.");
  modelColeta->setHeaderData("dataPrevColeta", "Prev. Coleta");
  modelColeta->setHeaderData("status", "Status");

  modelColeta->setFilter("status = 'EM COLETA'");

  ui->tableColeta->setModel(modelColeta);
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

  // Recebimentos fornecedor -------------------------------------------------------------------------------------------
  modelReceb = new SqlTableModel(this);
  modelReceb->setTable("pedido_fornecedor_has_produto");
  modelReceb->setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelReceb->setHeaderData("selecionado", "");
  modelReceb->setHeaderData("fornecedor", "Fornecedor");
  modelReceb->setHeaderData("descricao", "Descrição");
  modelReceb->setHeaderData("colecao", "Coleção");
  modelReceb->setHeaderData("quant", "Quant.");
  modelReceb->setHeaderData("un", "Un.");
  modelReceb->setHeaderData("preco", "Preço");
  modelReceb->setHeaderData("formComercial", "Form. Com.");
  modelReceb->setHeaderData("codComercial", "Cód. Com.");
  modelReceb->setHeaderData("codBarras", "Cód. Bar.");
  modelReceb->setHeaderData("idCompra", "Compra");
  modelReceb->setHeaderData("dataRealColeta", "Data Coleta");
  modelReceb->setHeaderData("dataPrevReceb", "Prev. Receb.");
  modelReceb->setHeaderData("status", "Status");

  modelReceb->setFilter("status = 'EM RECEBIMENTO'");

  ui->tableRecebimento->setModel(modelReceb);
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

  // Entregas cliente --------------------------------------------------------------------------------------------------
  modelEntregasCliente = new SqlTableModel(this);
  modelEntregasCliente->setTable("view_venda");

  ui->tableEntregasCliente->setModel(modelEntregasCliente);
  ui->tableEntregasCliente->setItemDelegate(doubledelegate);
}

bool WidgetLogistica::updateTables() {
  if (not modelPedForn2->select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + modelPedForn2->lastError().text());
    return false;
  }

  ui->tableFornLogistica->resizeColumnsToContents();

  switch (ui->tabWidgetLogistica->currentIndex()) {
    case 0: // Coletas
      // TODO: set filter = 0? (for when no manufacturer is selected)
      if (not modelColeta->select()) {
        QMessageBox::critical(this, "Erro!",
                              "Erro lendo tabela pedido_fornecedor_has_produto: " + modelColeta->lastError().text());
        return false;
      }

      for (int i = 0; i < modelColeta->rowCount(); ++i) {
        ui->tableColeta->openPersistentEditor(modelColeta->index(i, modelColeta->fieldIndex("selecionado")));
      }

      ui->tableColeta->resizeColumnsToContents();
      break;

    case 1: // Recebimentos
      if (not modelReceb->select()) {
        QMessageBox::critical(this, "Erro!",
                              "Erro lendo tabela pedido_fornecedor_has_produto: " + modelReceb->lastError().text());
        return false;
      }

      for (int i = 0; i < modelReceb->rowCount(); ++i) {
        ui->tableRecebimento->openPersistentEditor(modelReceb->index(i, modelReceb->fieldIndex("selecionado")));
      }

      ui->tableRecebimento->resizeColumnsToContents();
      break;

    case 2: // Entregas
      if (not modelEntregasCliente->select()) {
        QMessageBox::critical(this, "Erro!", "Erro lendo tabela vendas: " + modelEntregasCliente->lastError().text());
        return false;
      }

      ui->tableEntregasCliente->resizeColumnsToContents();
      break;

    default:
      return true;
  }

  return true;
}

void WidgetLogistica::on_radioButtonEntregaLimpar_clicked() {
  modelEntregasCliente->setFilter("tipo = 'cliente'");
  ui->tableEntregasCliente->resizeColumnsToContents();
}

void WidgetLogistica::on_radioButtonEntregaEnviado_clicked() {
  modelEntregasCliente->setFilter("status = 'enviado' AND tipo = 'cliente'");
  ui->tableEntregasCliente->resizeColumnsToContents();
}

void WidgetLogistica::on_radioButtonEntregaPendente_clicked() {
  modelEntregasCliente->setFilter("status = 'pendente' AND tipo = 'cliente'");
  ui->tableEntregasCliente->resizeColumnsToContents();
}

void WidgetLogistica::on_lineEditBuscaEntregas_textChanged(const QString &text) {
  modelEntregasCliente->setFilter(text.isEmpty() ? "" : "(idPedido LIKE '%" + text + "%') OR (status LIKE '%" + text +
                                                   "%')");

  ui->tableEntregasCliente->resizeColumnsToContents();
}

void WidgetLogistica::on_tableEntregasCliente_activated(const QModelIndex &index) {
  EntregasCliente *entregas = new EntregasCliente(this);
  entregas->viewEntrega(modelEntregasCliente->data(index.row(), "Código").toString());
}

void WidgetLogistica::on_pushButtonMarcarColetado_clicked() {
  QList<int> lista;

  for (int i = 0; i < modelColeta->rowCount(); ++i) {
    qDebug() << "selecionado: " << modelColeta->data(i, "selecionado");
  }

  // TODO: see why the hell this wont work with true while others tables do
  for (const auto index :
       modelColeta->match(modelColeta->index(0, modelColeta->fieldIndex("selecionado")), Qt::DisplayRole, 1, -1,
                          Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap))) {
    lista.append(index.row());
  }

  if (lista.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  InputDialog *inputDlg = new InputDialog(InputDialog::Coleta, this);
  QDate dataColeta, dataPrevista;

  if (inputDlg->exec() == InputDialog::Accepted) {
    dataColeta = inputDlg->getDate();
    dataPrevista = inputDlg->getNextDate();
  } else {
    return;
  }

  for (const auto row : lista) {
    modelColeta->setData(row, "selecionado", false);

    if (modelColeta->data(row, "status").toString() != "EM COLETA") {
      modelColeta->select();
      QMessageBox::warning(this, "Aviso!", "Produto não estava em coleta!");
      return;
    }

    if (not modelColeta->setData(row, "status", "EM RECEBIMENTO")) {
      QMessageBox::critical(this, "Erro!", "Erro marcando status EM RECEBIMENTO: " + modelColeta->lastError().text());
      return;
    }

    // salvar status na venda
    QSqlQuery query;

    query.prepare("UPDATE venda_has_produto SET dataRealColeta = :dataRealColeta, dataPrevReceb = :dataPrevReceb, "
                  "status = 'EM RECEBIMENTO' WHERE idCompra = :idCompra");
    query.bindValue(":dataRealColeta", dataColeta);
    query.bindValue(":dataPrevReceb", dataPrevista);
    query.bindValue(":idCompra", modelColeta->data(row, "idCompra"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status da venda: " + query.lastError().text());
      return;
    }
    //

    if (not modelColeta->setData(row, "dataRealColeta", dataColeta.toString("yyyy-MM-dd"))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando data da coleta: " + modelColeta->lastError().text());
      return;
    }

    if (not modelColeta->setData(row, "dataPrevReceb", dataPrevista.toString("yyyy-MM-dd"))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando data prevista: " + modelColeta->lastError().text());
      return;
    }
  }

  if (not modelColeta->submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados da tabela pedido_fornecedor_has_produto: " +
                          modelColeta->lastError().text());
    return;
  }

  updateTables();

  QMessageBox::information(this, "Aviso!", "Confirmado coleta.");
}

void WidgetLogistica::on_pushButtonMarcarRecebido_clicked() {
  QList<int> lista;

  for (const auto index :
       modelReceb->match(modelReceb->index(0, modelReceb->fieldIndex("selecionado")), Qt::DisplayRole, true, -1,
                         Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap))) {
    lista.append(index.row());
  }

  if (lista.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  InputDialog *inputDlg = new InputDialog(InputDialog::Recebimento, this);

  if (inputDlg->exec() != InputDialog::Accepted) {
    return;
  }

  QDate dataReceb = inputDlg->getDate();

  for (const auto row : lista) {
    modelReceb->setData(row, "selecionado", false);

    if (modelReceb->data(row, "status").toString() != "EM RECEBIMENTO") {
      modelReceb->select();
      QMessageBox::critical(this, "Erro!", "Produto não estava em recebimento!");
      return;
    }

    if (not modelReceb->setData(row, "status", "FINALIZADO")) {
      QMessageBox::critical(this, "Erro!", "Erro marcando status ESTOQUE: " + modelReceb->lastError().text());
      return;
    }

    // salvar status na venda
    QSqlQuery query;

    query.prepare(
          "UPDATE venda_has_produto SET dataRealReceb = :dataRealReceb, status = 'ESTOQUE' WHERE idCompra = :idCompra");
    query.bindValue(":dataRealReceb", dataReceb);
    query.bindValue(":idCompra", modelReceb->data(row, "idCompra"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status da venda: " + query.lastError().text());
      return;
    }
    //

    if (not modelReceb->setData(row, "dataRealReceb", dataReceb.toString("yyyy-MM-dd"))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando data de recebimento: " + modelReceb->lastError().text());
      return;
    }
  }

  // TODO: marcar flag no estoque de produto disponivel

  if (not modelReceb->submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados da tabela pedido_fornecedor_has_produto: " +
                          modelReceb->lastError().text());
    return;
  }

  updateTables();

  QMessageBox::information(this, "Aviso!", "Confirmado recebimento.");
}

void WidgetLogistica::on_tableFornLogistica_activated(const QModelIndex &index) {
  QString fornecedor = modelPedForn2->data(index.row(), "fornecedor").toString();

  modelColeta->setFilter("fornecedor = '" + fornecedor + "' AND status = 'EM COLETA'");

  if (not modelColeta->select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + modelColeta->lastError().text());
    return;
  }

  modelReceb->setFilter("fornecedor = '" + fornecedor + "' AND status = 'EM RECEBIMENTO'");

  if (not modelReceb->select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + modelReceb->lastError().text());
    return;
  }

  updateTables();
}

void WidgetLogistica::on_tabWidgetLogistica_currentChanged(const int &) { updateTables(); }

// TODO: a tabela de fornecedores deve mostrar apenas os pedidos que estejam coleta/recebimento/entrega
