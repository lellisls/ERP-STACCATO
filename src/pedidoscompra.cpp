#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "pedidoscompra.h"
#include "ui_pedidoscompra.h"
#include "cadastrarnfe.h"
#include "comboboxdelegate.h"

PedidosCompra::PedidosCompra(QWidget *parent) : QDialog(parent), ui(new Ui::PedidosCompra) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);
  setupTables();

  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

  ui->groupBox->hide();
  ui->dateTimeEdit->hide();
  ui->checkBox->hide();
  ui->pushButtonNFe->hide();

  show();
  ui->tablePedidos->resizeColumnsToContents();
}

PedidosCompra::~PedidosCompra() { delete ui; }

void PedidosCompra::viewPedido(const QString idPedido) {
  this->idPedido = idPedido;

  QModelIndex index = modelItemPedidos.match(modelItemPedidos.index(0, modelItemPedidos.fieldIndex("idPedido")),
                                             Qt::DisplayRole, idPedido).first();
  QString idProduto =
      modelItemPedidos.data(modelItemPedidos.index(index.row(), modelItemPedidos.fieldIndex("idProduto"))).toString();

  modelItemPedidos.setFilter("idProduto = '" + idProduto + "'");

  if (not modelItemPedidos.select()) {
    qDebug() << "erro modelItemPedidos: " << modelItemPedidos.lastError();
    return;
  }

  for (int row = 0, rowCount = modelItemPedidos.rowCount(); row < rowCount; ++row) {
    ui->tablePedidos->openPersistentEditor(modelItemPedidos.index(row, modelItemPedidos.fieldIndex("quant")));
  }
}

void PedidosCompra::on_pushButtonSalvar_clicked() {
  QSqlQuery query;
  query.prepare("INSERT INTO conta_a_pagar(idVenda, dataEmissao) VALUES (:idVenda, :dataEmissao)");
  query.bindValue(":idVenda", idPedido);
  query.bindValue(":dataEmissao", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

  if (not query.exec()) {
    qDebug() << "Erro inserindo conta a pagar: " << query.lastError();
  }

  query.prepare("SELECT * FROM pedido_fornecedor_has_produto WHERE idPedido = :idPedido");
  query.bindValue(":idPedido", idPedido);

  if (not query.exec()) {
    qDebug() << "Erro buscando produtos: " << query.lastError();
  }

  qDebug() << "size: " << query.size();

  for (int row = 0, size = query.size(); row < size; ++row) {
    const QString combobox =
        modelItemPedidos.data(modelItemPedidos.index(row, modelItemPedidos.fieldIndex("status"))).toString();
    query.next();
    const QString status = query.value("status").toString();

    if (status != combobox) {
      if (combobox == "Comprar") {
        qDebug() << "Comprar";
        QSqlQuery queryProduto;
        queryProduto.prepare("INSERT INTO conta_a_pagar_has_produto VALUES (:idVenda, :idProduto, :produto, "
                             ":prcUnitario, :quant, :un, :total)");
        queryProduto.bindValue(":idVenda", query.value("idPedido"));
        queryProduto.bindValue(":idProduto", query.value("idProduto"));
        queryProduto.bindValue(":produto", query.value("produto"));
        queryProduto.bindValue(":prcUnitario", query.value("prcUnitario"));
        queryProduto.bindValue(":quant", query.value("quant"));
        queryProduto.bindValue(":un", query.value("un"));
        queryProduto.bindValue(":total", query.value("total"));

        if (not queryProduto.exec()) {
          qDebug() << "Erro inserindo produtos na conta: " << queryProduto.lastError();
        }

        QSqlQuery queryEntrega;
        queryEntrega.prepare("INSERT INTO pedido_transportadora (idTransportadora, idPedido, dataEmissao, "
                             "status, tipo) VALUES (:idTransportadora, :idPedido, :dataEmissao, :status, "
                             ":tipo)");
        queryEntrega.bindValue(":idTransportadora", 1);
        queryEntrega.bindValue(":idPedido", query.value("idPedido"));
        queryEntrega.bindValue(":dataEmissao", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        queryEntrega.bindValue(":status", "PENDENTE");
        queryEntrega.bindValue(":tipo", "Fornecedor");

        if (not queryEntrega.exec()) {
          qDebug() << "Erro inserindo recebimento fornecedor: " << queryEntrega.lastError();
        }

        queryEntrega.prepare("INSERT INTO pedido_transportadora (idTransportadora, idPedido, dataEmissao, "
                             "status, tipo) VALUES (:idTransportadora, :idPedido, :dataEmissao, :status, "
                             ":tipo)");
        queryEntrega.bindValue(":idTransportadora", 1);
        queryEntrega.bindValue(":idPedido", query.value("idPedido"));
        queryEntrega.bindValue(":dataEmissao", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        queryEntrega.bindValue(":status", "PENDENTE");
        queryEntrega.bindValue(":tipo", "Cliente");

        if (not queryEntrega.exec()) {
          qDebug() << "Erro inserindo entrega cliente: " << queryEntrega.lastError();
        }
      }

      if (combobox == "Pago") {
        qDebug() << "Pago";
        // do something
      }
    }
  }

  if (ui->checkBox->isChecked()) {
    qDebug() << "submit data to table";
    qDebug() << "generate NFe";
    qDebug() << "generate delivery for client";
  }

  if (not modelItemPedidos.submitAll()) {
    qDebug() << "modelItemPedidos falhou: " << modelItemPedidos.lastError();
  }

  close();
}

void PedidosCompra::on_pushButtonCancelar_clicked() { close(); }

void PedidosCompra::on_checkBox_toggled(const bool checked) { Q_UNUSED(checked); }

void PedidosCompra::on_radioButtonVenda_toggled(const bool checked) {
  Q_UNUSED(checked);
  //  modelPedidos
}

void PedidosCompra::setupTables() {
  modelItemPedidos.setTable("pedido_fornecedor_has_produto");
  modelItemPedidos.setEditStrategy(QSqlTableModel::OnManualSubmit);

  //  modelItemPedidos.setHeaderData(modelItemPedidos.fieldIndex("idPedido"), Qt::Horizontal, "Pedido");
  //  modelItemPedidos.setHeaderData(modelItemPedidos.fieldIndex("fornecedor"), Qt::Horizontal, "Fornecedor");
  //  modelItemPedidos.setHeaderData(modelItemPedidos.fieldIndex("produto"), Qt::Horizontal, "Produto");
  //  modelItemPedidos.setHeaderData(modelItemPedidos.fieldIndex("obs"), Qt::Horizontal, "Obs.");
  //  modelItemPedidos.setHeaderData(modelItemPedidos.fieldIndex("caixas"), Qt::Horizontal, "Cx.");
  //  modelItemPedidos.setHeaderData(modelItemPedidos.fieldIndex("quant"), Qt::Horizontal, "Quant.");
  //  modelItemPedidos.setHeaderData(modelItemPedidos.fieldIndex("un"), Qt::Horizontal, "Un.");
  //  modelItemPedidos.setHeaderData(modelItemPedidos.fieldIndex("unCaixa"), Qt::Horizontal, "Un./Cx.");
  //  modelItemPedidos.setHeaderData(modelItemPedidos.fieldIndex("total"), Qt::Horizontal, "Total");
  //  modelItemPedidos.setHeaderData(modelItemPedidos.fieldIndex("status"), Qt::Horizontal, "Status");

  if (not modelItemPedidos.select()) {
    qDebug() << "Failed to populate pedido_fornecedor_has_produto: " << modelItemPedidos.lastError();
    return;
  }

  ui->tablePedidos->setModel(&modelItemPedidos);
  ui->tablePedidos->setItemDelegateForColumn(modelItemPedidos.fieldIndex("status"), new ComboBoxDelegate(this));
  ui->tablePedidos->hideColumn(modelItemPedidos.fieldIndex("idLoja"));
  ui->tablePedidos->hideColumn(modelItemPedidos.fieldIndex("item"));
  ui->tablePedidos->hideColumn(modelItemPedidos.fieldIndex("idProduto"));
  ui->tablePedidos->hideColumn(modelItemPedidos.fieldIndex("prcUnitario"));
  ui->tablePedidos->hideColumn(modelItemPedidos.fieldIndex("parcial"));
  ui->tablePedidos->hideColumn(modelItemPedidos.fieldIndex("desconto"));
  ui->tablePedidos->hideColumn(modelItemPedidos.fieldIndex("parcialDesc"));
  ui->tablePedidos->hideColumn(modelItemPedidos.fieldIndex("descGlobal"));
  ui->tablePedidos->horizontalHeader()->setResizeContentsPrecision(0);
  ui->tablePedidos->verticalHeader()->setResizeContentsPrecision(0);
}

void PedidosCompra::on_pushButtonNFe_clicked() {
  CadastrarNFe *const cadNFe = new CadastrarNFe(idPedido, this);

  const QModelIndexList list = ui->tablePedidos->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::warning(this, "Aviso!", "Nenhum item selecionado!");
    return;
  }

  QList<int> lista;

  foreach (const QModelIndex index, list) { lista.append(index.row()); }
  qDebug() << "lista: " << lista;

  cadNFe->prepararNFe(lista);
  cadNFe->showMaximized();
}
