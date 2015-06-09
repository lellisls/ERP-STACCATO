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
  }

  modelPedidos.setTable("pedidofornecedor");
  modelPedidos.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelPedidos.select()) {
    qDebug() << "Failed to populate pedidofornecedor: " << modelPedidos.lastError();
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
  modelItemPedidos.select();

  for (int row = 0; row < modelItemPedidos.rowCount(); ++row) {
    ui->tablePedidos->openPersistentEditor(modelItemPedidos.index(row, modelItemPedidos.fieldIndex("qte")));
  }
}

void PedidosCompra::on_pushButtonSalvar_clicked() {
  QSqlQuery qryConta;
  qryConta.prepare("INSERT INTO contaapagar(idVenda, dataEmissao) VALUES (:idVenda, :dataEmissao)");
  qryConta.bindValue(":idVenda", idPedido);
  qryConta.bindValue(":dataEmissao", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

  if (not qryConta.exec()) {
    qDebug() << "Erro inserindo conta a pagar: " << qryConta.lastError();
  }

  QSqlQuery qry;
  qry.prepare("SELECT * FROM pedidofornecedor_has_produto WHERE idPedido = :idPedido");
  qry.bindValue(":idPedido", idPedido);

  if (not qry.exec()) {
    qDebug() << "Erro buscando produtos: " << qry.lastError();
  }

  qDebug() << "size: " << qry.size();

  for (int row = 0; row < qry.size(); ++row) {
    QString combobox =
        modelItemPedidos.data(modelItemPedidos.index(row, modelItemPedidos.fieldIndex("status"))).toString();
    qry.next();
    QString bd = qry.value("status").toString();

    if (bd != combobox) {
      if (combobox == "Comprar") {
        qDebug() << "Comprar";
        QSqlQuery qryProduto;
        qryProduto.prepare("INSERT INTO contaapagar_has_produto VALUES (:idVenda, :idProduto, :produto, "
                           ":prcUnitario, :qte, :un, :total)");
        qryProduto.bindValue(":idVenda", qry.value("idPedido"));
        qryProduto.bindValue(":idProduto", qry.value("idProduto"));
        qryProduto.bindValue(":produto", qry.value("produto"));
        qryProduto.bindValue(":prcUnitario", qry.value("prcUnitario"));
        qryProduto.bindValue(":qte", qry.value("qte"));
        qryProduto.bindValue(":un", qry.value("un"));
        qryProduto.bindValue(":total", qry.value("total"));

        if (not qryProduto.exec()) {
          qDebug() << "Erro inserindo produtos na conta: " << qryProduto.lastError();
        }

        QSqlQuery qryEntrega;
        qryEntrega.prepare("INSERT INTO pedidotransportadora (idTransportadora, idPedido, dataEmissao, "
                           "status, tipo) VALUES (:idTransportadora, :idPedido, :dataEmissao, :status, "
                           ":tipo)");
        qryEntrega.bindValue(":idTransportadora", 1);
        qryEntrega.bindValue(":idPedido", qry.value("idPedido"));
        qryEntrega.bindValue(":dataEmissao", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        qryEntrega.bindValue(":status", "PENDENTE");
        qryEntrega.bindValue(":tipo", "Fornecedor");

        if (not qryEntrega.exec()) {
          qDebug() << "Erro inserindo recebimento fornecedor: " << qryEntrega.lastError();
        }

        qryEntrega.prepare("INSERT INTO pedidotransportadora (idTransportadora, idPedido, dataEmissao, "
                           "status, tipo) VALUES (:idTransportadora, :idPedido, :dataEmissao, :status, "
                           ":tipo)");
        qryEntrega.bindValue(":idTransportadora", 1);
        qryEntrega.bindValue(":idPedido", qry.value("idPedido"));
        qryEntrega.bindValue(":dataEmissao", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        qryEntrega.bindValue(":status", "PENDENTE");
        qryEntrega.bindValue(":tipo", "Cliente");

        if (not qryEntrega.exec()) {
          qDebug() << "Erro inserindo entrega cliente: " << qryEntrega.lastError();
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

  if (MainWindow *window = qobject_cast<MainWindow *>(parentWidget())) {
    window->updateTables();
  }

  close();
}

void PedidosCompra::on_pushButtonCancelar_clicked() { close(); }

void PedidosCompra::on_checkBox_toggled(bool checked) { Q_UNUSED(checked); }

void PedidosCompra::updateTables() {
  modelItemPedidos.select();
  modelPedidos.select();

  ui->tablePedidos->resizeColumnsToContents();
}

void PedidosCompra::on_radioButtonVenda_toggled(bool checked) {
  Q_UNUSED(checked);
  //  modelPedidos
}

void PedidosCompra::on_pushButtonNFe_clicked() {
  QModelIndexList list = ui->tablePedidos->selectionModel()->selectedRows();
  QList<int> rows;
  foreach (QModelIndex idx, list) { rows.append(idx.row()); }
  CadastrarNFE nfe(idPedido);
  nfe.prepararNFe(rows);
  qDebug() << rows;
}
