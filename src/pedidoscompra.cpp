#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "cadastrarnfe.h"
#include "comboboxdelegate.h"
#include "mainwindow.h"
#include "pedidoscompra.h"
#include "ui_pedidoscompra.h"

PedidosCompra::PedidosCompra(QWidget *parent) : QDialog(parent), ui(new Ui::PedidosCompra) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  modelItemPedidos.setTable("pedidofornecedor_has_produto");
  modelItemPedidos.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelItemPedidos.select()) {
    qDebug() << "Failed to populate pedidofornecedor_has_produto: " << modelItemPedidos.lastError();
    return;
  }

  modelPedidos.setTable("pedidofornecedor");
  modelPedidos.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelPedidos.select()) {
    qDebug() << "Failed to populate pedidofornecedor: " << modelPedidos.lastError();
    return;
  }

  ui->tablePedidos->setModel(&modelItemPedidos);
  ui->tablePedidos->setItemDelegateForColumn(modelItemPedidos.fieldIndex("status"),
                                             new ComboBoxDelegate(ui->tablePedidos));

  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

  ui->tablePedidos->hideColumn(modelItemPedidos.fieldIndex("idLoja"));
  ui->tablePedidos->hideColumn(modelItemPedidos.fieldIndex("item"));
  ui->tablePedidos->hideColumn(modelItemPedidos.fieldIndex("idProduto"));
  ui->tablePedidos->hideColumn(modelItemPedidos.fieldIndex("prcUnitario"));
  ui->tablePedidos->hideColumn(modelItemPedidos.fieldIndex("parcial"));
  ui->tablePedidos->hideColumn(modelItemPedidos.fieldIndex("desconto"));
  ui->tablePedidos->hideColumn(modelItemPedidos.fieldIndex("parcialDesc"));
  ui->tablePedidos->hideColumn(modelItemPedidos.fieldIndex("descGlobal"));

  modelItemPedidos.setHeaderData(modelItemPedidos.fieldIndex("idPedido"), Qt::Horizontal, "Pedido");
  modelItemPedidos.setHeaderData(modelItemPedidos.fieldIndex("fornecedor"), Qt::Horizontal, "Fornecedor");
  modelItemPedidos.setHeaderData(modelItemPedidos.fieldIndex("produto"), Qt::Horizontal, "Produto");
  modelItemPedidos.setHeaderData(modelItemPedidos.fieldIndex("obs"), Qt::Horizontal, "Obs.");
  modelItemPedidos.setHeaderData(modelItemPedidos.fieldIndex("caixas"), Qt::Horizontal, "Cx.");
  modelItemPedidos.setHeaderData(modelItemPedidos.fieldIndex("qte"), Qt::Horizontal, "Qte.");
  modelItemPedidos.setHeaderData(modelItemPedidos.fieldIndex("un"), Qt::Horizontal, "Un.");
  modelItemPedidos.setHeaderData(modelItemPedidos.fieldIndex("unCaixa"), Qt::Horizontal, "Un./Cx.");
  modelItemPedidos.setHeaderData(modelItemPedidos.fieldIndex("total"), Qt::Horizontal, "Total");
  modelItemPedidos.setHeaderData(modelItemPedidos.fieldIndex("status"), Qt::Horizontal, "Status");

  updateTables();
  show();
}

PedidosCompra::~PedidosCompra() { delete ui; }

void PedidosCompra::viewPedido(QString idPedido) {
  this->idPedido = idPedido;

  modelItemPedidos.setFilter("idPedido = '" + idPedido + "'");

  if (not modelItemPedidos.select()) {
    qDebug() << "erro modelItemPedidos: " << modelItemPedidos.lastError();
    return;
  }

  for (int row = 0; row < modelItemPedidos.rowCount(); ++row) {
    ui->tablePedidos->openPersistentEditor(modelItemPedidos.index(row, modelItemPedidos.fieldIndex("qte")));
  }
}

void PedidosCompra::on_pushButtonSalvar_clicked() {
  QSqlQuery query;
  query.prepare("INSERT INTO ContaAPagar(idVenda, dataEmissao) VALUES (:idVenda, :dataEmissao)");
  query.bindValue(":idVenda", idPedido);
  query.bindValue(":dataEmissao", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

  if (not query.exec()) {
    qDebug() << "Erro inserindo conta a pagar: " << query.lastError();
  }

  query.prepare("SELECT * FROM PedidoFornecedor_has_Produto WHERE idPedido = :idPedido");
  query.bindValue(":idPedido", idPedido);

  if (not query.exec()) {
    qDebug() << "Erro buscando produtos: " << query.lastError();
  }

  qDebug() << "size: " << query.size();

  for (int row = 0; row < query.size(); ++row) {
    QString combobox =
        modelItemPedidos.data(modelItemPedidos.index(row, modelItemPedidos.fieldIndex("status"))).toString();
    query.next();
    QString status = query.value("status").toString();

    if (status != combobox) {
      if (combobox == "Comprar") {
        qDebug() << "Comprar";
        QSqlQuery queryProduto;
        queryProduto.prepare("INSERT INTO ContaAPagar_has_Produto VALUES (:idVenda, :idProduto, :produto, "
                             ":prcUnitario, :qte, :un, :total)");
        queryProduto.bindValue(":idVenda", query.value("idPedido"));
        queryProduto.bindValue(":idProduto", query.value("idProduto"));
        queryProduto.bindValue(":produto", query.value("produto"));
        queryProduto.bindValue(":prcUnitario", query.value("prcUnitario"));
        queryProduto.bindValue(":qte", query.value("qte"));
        queryProduto.bindValue(":un", query.value("un"));
        queryProduto.bindValue(":total", query.value("total"));

        if (not queryProduto.exec()) {
          qDebug() << "Erro inserindo produtos na conta: " << queryProduto.lastError();
        }

        QSqlQuery queryEntrega;
        queryEntrega.prepare("INSERT INTO PedidoTransportadora (idTransportadora, idPedido, dataEmissao, "
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

        queryEntrega.prepare("INSERT INTO PedidoTransportadora (idTransportadora, idPedido, dataEmissao, "
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

void PedidosCompra::on_checkBox_toggled(bool checked) { Q_UNUSED(checked); }

void PedidosCompra::updateTables() {
  if (not modelItemPedidos.select()) {
    qDebug() << "erro modelItemPedidos: " << modelItemPedidos.lastError();
    return;
  }

  if (not modelPedidos.select()) {
    qDebug() << "erro modelPedidos: " << modelPedidos.lastError();
    return;
  }

  ui->tablePedidos->resizeColumnsToContents();
}

void PedidosCompra::on_radioButtonVenda_toggled(bool checked) {
  Q_UNUSED(checked);
  //  modelPedidos
}

void PedidosCompra::on_pushButtonNFe_clicked() {
  QModelIndexList list = ui->tablePedidos->selectionModel()->selectedRows();
  QList<int> rows;
  foreach (QModelIndex index, list) { rows.append(index.row()); }
  CadastrarNFe nfe(idPedido);
  nfe.prepararNFe(rows);
  qDebug() << rows;
}
