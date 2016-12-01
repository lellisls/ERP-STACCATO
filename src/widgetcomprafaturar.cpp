#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "importarxml.h"
#include "inputdialogproduto.h"
#include "reaisdelegate.h"
#include "ui_widgetcomprafaturar.h"
#include "widgetcomprafaturar.h"

WidgetCompraFaturar::WidgetCompraFaturar(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompraFaturar) {
  ui->setupUi(this);
}

WidgetCompraFaturar::~WidgetCompraFaturar() { delete ui; }

void WidgetCompraFaturar::setupTables() {
  model.setTable("view_faturamento");

  model.setHeaderData("dataPrevFat", "Prev. Fat.");

  ui->table->setModel(&model);
  ui->table->setItemDelegateForColumn("Total", new ReaisDelegate(this));
  ui->table->hideColumn("idCompra");
  ui->table->hideColumn("representacao");
}

bool WidgetCompraFaturar::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela faturamento: " + model.lastError().text());
    return false;
  }

  ui->table->resizeColumnsToContents();

  return true;
}

bool WidgetCompraFaturar::faturarCompra() {
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Não selecionou nenhuma compra!");
    return false;
  }

  QSqlQuery query;
  query.prepare("SELECT representacao FROM fornecedor WHERE razaoSocial = :razaoSocial");
  query.bindValue(":razaoSocial", model.data(list.first().row(), "Fornecedor"));

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro verificando se fornecedor é representação: " + query.lastError().text());
    return false;
  }

  const bool representacao = query.value("representacao").toBool();

  QStringList idsCompra;

  for (auto const &item : list) idsCompra << model.data(item.row(), "idCompra").toString();

  InputDialogProduto *inputDlg = new InputDialogProduto(InputDialogProduto::Faturamento, this);
  if (not inputDlg->setFilter(idsCompra)) return false;
  if (inputDlg->exec() != InputDialogProduto::Accepted) return false;

  const QDateTime dataReal = inputDlg->getDate();

  if (representacao) {
    for (auto const &idCompra : idsCompra) {
      query.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'EM ENTREGA', dataRealFat = :dataRealFat WHERE "
                    "idCompra = :idCompra");
      query.bindValue(":dataRealFat", dataReal);
      query.bindValue(":idCompra", idCompra);

      if (not query.exec()) {
        QMessageBox::critical(this, "Erro!", "Erro atualizando status da compra: " + query.lastError().text());
        return false;
      }

      query.prepare("UPDATE venda_has_produto SET status = 'EM ENTREGA' WHERE idCompra = :idCompra");
      query.bindValue(":idCompra", idCompra);

      if (not query.exec()) {
        QMessageBox::critical(this, "Erro!",
                              "Erro atualizando status do produto da venda: " + query.lastError().text());
        return false;
      }
    }
  } else {
    ImportarXML *import = new ImportarXML(idsCompra, dataReal, this);
    import->showMaximized();

    if (import->exec() != QDialog::Accepted) return false;
  }

  // salvar status na venda
  for (auto const &idCompra : idsCompra) {
    // TODO: importar nota nao sai atualizando indiscriminadamente, apenas os que forem quantUpd = 1. Atualizar na venda
    // os pedidos que estiverem ok?
    // como pedido_fornecedor agora tem idVendaProduto, usar ele para determinar quais linhas atualizar na venda_produto
    query.prepare("UPDATE venda_has_produto SET dataRealFat = :dataRealFat WHERE idCompra = :idCompra");
    query.bindValue(":dataRealFat", dataReal);
    query.bindValue(":idCompra", idCompra);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando status da venda: " + query.lastError().text());
      return false;
    }
  }

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    return false;
  }

  return true;
}

void WidgetCompraFaturar::on_pushButtonMarcarFaturado_clicked() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not faturarCompra()) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  updateTables();
  QMessageBox::information(this, "Aviso!", "Confirmado faturamento.");
}

void WidgetCompraFaturar::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetCompraFaturar::on_checkBoxRepresentacao_toggled(bool checked) {
  model.setFilter("representacao = " + QString(checked ? "1" : "0"));

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
}

bool WidgetCompraFaturar::cancelar() {
  auto list = ui->table->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return false;
  }

  QMessageBox msgBox(QMessageBox::Question, "Cancelar?", "Tem certeza que deseja cancelar?",
                     QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Cancelar");
  msgBox.setButtonText(QMessageBox::No, "Voltar");

  if (msgBox.exec() == QMessageBox::No) return false;

  for (auto const &item : list) {
    QSqlQuery query;
    query.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'CANCELADO' WHERE ordemCompra = :ordemCompra");
    query.bindValue(":ordemCompra", model.data(item.row(), "OC"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando dados: " + query.lastError().text());
      return false;
    }

    query.prepare("SELECT idVendaProduto FROM pedido_fornecedor_has_produto WHERE ordemCompra = :ordemCompra");
    query.bindValue(":ordemCompra", model.data(item.row(), "OC"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando dados: " + query.lastError().text());
      return false;
    }

    while (query.next()) {
      QSqlQuery query2;
      query2.prepare("UPDATE venda_has_produto SET status = 'PENDENTE' WHERE idVendaProduto = :idVendaProduto AND "
                     "(status != 'DEVOLVIDO' OR status != 'CANCELADO')");
      query2.bindValue(":idVendaProduto", query.value("idVendaProduto"));

      if (not query2.exec()) {
        QMessageBox::critical(this, "Erro!", "Erro voltando status do produto: " + query2.lastError().text());
        return false;
      }
    }

    query.prepare("UPDATE conta_a_pagar_has_pagamento SET status = 'CANCELADO' WHERE idCompra = :idCompra");
    query.bindValue(":idCompra", model.data(item.row(), "idCompra"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando pagamentos: " + query.lastError().text());
      return false;
    }
  }

  return true;
}

void WidgetCompraFaturar::on_pushButtonCancelarCompra_clicked() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not cancelar()) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  updateTables();
  QMessageBox::information(this, "Aviso!", "Itens cancelados!");
}
