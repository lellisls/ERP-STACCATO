#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "checkboxdelegate.h"
#include "comboboxdelegate.h"
#include "contas.h"
#include "doubledelegate.h"
#include "itemboxdelegate.h"
#include "lineeditdelegate.h"
#include "ui_contas.h"

Contas::Contas(const Tipo tipo, QWidget *parent) : QDialog(parent), tipo(tipo), ui(new Ui::Contas) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);
  setAttribute(Qt::WA_DeleteOnClose);

  setupTables();

  showMaximized();

  connect(&modelPendentes, &QSqlTableModel::dataChanged, this, &Contas::preencher);
}

Contas::~Contas() { delete ui; }

void Contas::preencher(const QModelIndex &index) {
  if (index.column() == modelPendentes.fieldIndex("dataRealizado")) {
    modelPendentes.setData(index.row(), "status", tipo == Receber ? "RECEBIDO" : "PAGO");
    modelPendentes.setData(index.row(), "valorReal", modelPendentes.data(index.row(), "valor"));
    modelPendentes.setData(index.row(), "tipoReal", modelPendentes.data(index.row(), "tipo"));
    modelPendentes.setData(index.row(), "parcelaReal", modelPendentes.data(index.row(), "parcela"));
    modelPendentes.setData(index.row(), "contaDestino", 3);
    modelPendentes.setData(index.row(), "centroCusto", modelPendentes.data(index.row(), "idLoja"));

    //

    const QModelIndexList list =
        modelPendentes.match(modelPendentes.index(0, modelPendentes.fieldIndex("tipo")), Qt::DisplayRole,
                             modelPendentes.data(index.row(), "tipo").toString().left(1) + ". Taxa Cartão", -1);

    for (auto const &indexMatch : list) {
      if (modelPendentes.data(indexMatch.row(), "parcela") != modelPendentes.data(index.row(), "parcela")) continue;

      modelPendentes.setData(indexMatch.row(), "dataRealizado", modelPendentes.data(index.row(), "dataRealizado"));
      modelPendentes.setData(indexMatch.row(), "status", tipo == Receber ? "RECEBIDO" : "PAGO");
      modelPendentes.setData(indexMatch.row(), "valorReal", modelPendentes.data(indexMatch.row(), "valor"));
      modelPendentes.setData(indexMatch.row(), "tipoReal", modelPendentes.data(indexMatch.row(), "tipo"));
      modelPendentes.setData(indexMatch.row(), "parcelaReal", modelPendentes.data(indexMatch.row(), "parcela"));
      modelPendentes.setData(indexMatch.row(), "contaDestino", 3);
      modelPendentes.setData(indexMatch.row(), "centroCusto", modelPendentes.data(indexMatch.row(), "idLoja"));
    }
  }
}

void Contas::setupTables() {
  modelPendentes.setTable(tipo == Receber ? "conta_a_receber_has_pagamento" : "conta_a_pagar_has_pagamento");
  modelPendentes.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelPendentes.setHeaderData("dataEmissao", "Data Emissão");
  modelPendentes.setHeaderData("contraParte", "ContraParte");
  modelPendentes.setHeaderData("valor", "R$");
  modelPendentes.setHeaderData("tipo", "Tipo");
  modelPendentes.setHeaderData("parcela", "Parcela");
  modelPendentes.setHeaderData("dataPagamento", "Vencimento");
  modelPendentes.setHeaderData("observacao", "Obs.");
  modelPendentes.setHeaderData("status", "Status");
  modelPendentes.setHeaderData("dataRealizado", "Data Realizado");
  modelPendentes.setHeaderData("valorReal", "R$ Real");
  modelPendentes.setHeaderData("tipoReal", "Tipo Real");
  modelPendentes.setHeaderData("parcelaReal", "Parcela Real");
  modelPendentes.setHeaderData("contaDestino", "Conta Dest.");
  modelPendentes.setHeaderData("tipoDet", "Tipo Det");
  modelPendentes.setHeaderData("centroCusto", "Centro Custo");
  modelPendentes.setHeaderData("grupo", "Grupo");
  modelPendentes.setHeaderData("subGrupo", "SubGrupo");

  ui->tablePendentes->setModel(&modelPendentes);
  ui->tablePendentes->setItemDelegate(new DoubleDelegate(this));

  if (tipo == Receber) {
    ui->tablePendentes->setItemDelegateForColumn("contraParte",
                                                 new LineEditDelegate(LineEditDelegate::ContraParteReceber, this));
    ui->tablePendentes->setItemDelegateForColumn("status", new ComboBoxDelegate(ComboBoxDelegate::StatusReceber, this));
    ui->tablePendentes->setItemDelegateForColumn("contaDestino",
                                                 new ItemBoxDelegate(ItemBoxDelegate::Conta, false, this));
    ui->tablePendentes->setItemDelegateForColumn("representacao", new CheckBoxDelegate(this, true));
    ui->tablePendentes->setItemDelegateForColumn("centroCusto",
                                                 new ItemBoxDelegate(ItemBoxDelegate::Loja, false, this));
    ui->tablePendentes->setItemDelegateForColumn("grupo", new LineEditDelegate(LineEditDelegate::Grupo, this));
  }

  if (tipo == Pagar) {
    ui->tablePendentes->setItemDelegateForColumn("contraParte",
                                                 new LineEditDelegate(LineEditDelegate::ContraPartePagar, this));
    ui->tablePendentes->setItemDelegateForColumn("status", new ComboBoxDelegate(ComboBoxDelegate::StatusPagar, this));
    ui->tablePendentes->setItemDelegateForColumn("contaDestino",
                                                 new ItemBoxDelegate(ItemBoxDelegate::Conta, false, this));
    ui->tablePendentes->setItemDelegateForColumn("representacao", new CheckBoxDelegate(this, true));
    ui->tablePendentes->setItemDelegateForColumn("centroCusto",
                                                 new ItemBoxDelegate(ItemBoxDelegate::Loja, false, this));
    ui->tablePendentes->setItemDelegateForColumn("grupo", new LineEditDelegate(LineEditDelegate::Grupo, this));
  }

  ui->tablePendentes->hideColumn("idCompra");
  ui->tablePendentes->hideColumn("representacao");
  ui->tablePendentes->hideColumn("idPagamento");
  ui->tablePendentes->hideColumn("idVenda");
  ui->tablePendentes->hideColumn("idLoja");
  ui->tablePendentes->hideColumn("created");
  ui->tablePendentes->hideColumn("lastUpdated");
  ui->tablePendentes->hideColumn("comissao");
  ui->tablePendentes->hideColumn("taxa");

  ui->tablePendentes->resizeColumnsToContents();

  //

  modelProcessados.setTable(tipo == Receber ? "conta_a_receber_has_pagamento" : "conta_a_pagar_has_pagamento");
  modelProcessados.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelProcessados.setHeaderData("dataEmissao", "Data Emissão");
  modelProcessados.setHeaderData("contraParte", "ContraParte");
  modelProcessados.setHeaderData("valor", "R$");
  modelProcessados.setHeaderData("tipo", "Tipo");
  modelProcessados.setHeaderData("parcela", "Parcela");
  modelProcessados.setHeaderData("dataPagamento", "Vencimento");
  modelProcessados.setHeaderData("observacao", "Obs.");
  modelProcessados.setHeaderData("status", "Status");
  modelProcessados.setHeaderData("dataRealizado", "Data Realizado");
  modelProcessados.setHeaderData("valorReal", "R$ Real");
  modelProcessados.setHeaderData("tipoReal", "Tipo Real");
  modelProcessados.setHeaderData("parcelaReal", "Parcela Real");
  modelProcessados.setHeaderData("contaDestino", "Conta Dest.");
  modelProcessados.setHeaderData("tipoDet", "Tipo Det");
  modelProcessados.setHeaderData("centroCusto", "Centro Custo");
  modelProcessados.setHeaderData("grupo", "Grupo");
  modelProcessados.setHeaderData("subGrupo", "SubGrupo");

  ui->tableProcessados->setModel(&modelProcessados);
  ui->tableProcessados->setItemDelegate(new DoubleDelegate(this));
  ui->tableProcessados->setItemDelegateForColumn("status", new ComboBoxDelegate(ComboBoxDelegate::StatusReceber, this));
  ui->tableProcessados->setItemDelegateForColumn("contaDestino",
                                                 new ItemBoxDelegate(ItemBoxDelegate::Conta, true, this));
  ui->tableProcessados->setItemDelegateForColumn("representacao", new CheckBoxDelegate(this, true));
  ui->tableProcessados->setItemDelegateForColumn("centroCusto", new ItemBoxDelegate(ItemBoxDelegate::Loja, true, this));
  ui->tableProcessados->hideColumn("representacao");
  ui->tableProcessados->hideColumn("idPagamento");
  ui->tableProcessados->hideColumn("idVenda");
  ui->tableProcessados->hideColumn("idLoja");
  ui->tableProcessados->hideColumn("created");
  ui->tableProcessados->hideColumn("lastUpdated");
  ui->tableProcessados->hideColumn("comissao");
  ui->tableProcessados->hideColumn("taxa");

  ui->tableProcessados->resizeColumnsToContents();
}

bool Contas::verifyFields() {
  for (int row = 0; row < modelPendentes.rowCount(); ++row) {
    if ((tipo == Pagar and modelPendentes.data(row, "status").toString() == "PAGO") or
        (tipo == Receber and modelPendentes.data(row, "status").toString() == "RECEBIDO")) {
      if (modelPendentes.data(row, "dataRealizado").toString().isEmpty()) {
        QMessageBox::critical(this, "Erro!", "'Data Realizado' vazio!");
        return false;
      }

      if (modelPendentes.data(row, "valorReal").toString().isEmpty()) {
        QMessageBox::critical(this, "Erro!", "'R$ Real' vazio!");
        return false;
      }

      if (modelPendentes.data(row, "tipoReal").toString().isEmpty()) {
        QMessageBox::critical(this, "Erro!", "'Tipo Real' vazio!");
        return false;
      }

      if (modelPendentes.data(row, "parcelaReal").toString().isEmpty()) {
        QMessageBox::critical(this, "Erro!", "'Parcela Real' vazio!");
        return false;
      }

      if (modelPendentes.data(row, "contaDestino").toString().isEmpty()) {
        QMessageBox::critical(this, "Erro!", "'Conta Dest.' vazio!");
        return false;
      }

      if (modelPendentes.data(row, "centroCusto").toString().isEmpty()) {
        QMessageBox::critical(this, "Erro!", "'Centro Custo' vazio!");
        return false;
      }
    }
  }

  return true;
}

void Contas::on_pushButtonSalvar_clicked() {
  if (not verifyFields()) return;

  if (not modelPendentes.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados: " + modelPendentes.lastError().text());
    return;
  }

  close();
}

void Contas::viewConta(const QString &idPagamento, const QString &contraparte) {
  if (tipo == Receber) {
    QSqlQuery query;
    query.prepare("SELECT idVenda FROM conta_a_receber_has_pagamento WHERE idPagamento = :idPagamento");
    query.bindValue(":idPagamento", idPagamento);

    if (not query.exec() or not query.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando dados: " + query.lastError().text());
      return;
    }

    const QString idVenda = query.value("idVenda").toString();

    setWindowTitle("Contas A Receber - " + contraparte + " " + idVenda);

    modelPendentes.setFilter("idVenda LIKE '" + idVenda + "%' AND status = 'PENDENTE' AND representacao = 0");

    if (not modelPendentes.select()) {
      QMessageBox::critical(this, "Erro!",
                            "Erro lendo tabela conta_a_receber_has_pagamento: " + modelPendentes.lastError().text());
    }

    ui->tablePendentes->resizeColumnsToContents();

    for (int row = 0, rowCount = modelPendentes.rowCount(); row < rowCount; ++row) {
      ui->tablePendentes->openPersistentEditor(row, "status");
      ui->tablePendentes->openPersistentEditor(row, "contaDestino");
      ui->tablePendentes->openPersistentEditor(row, "centroCusto");
    }

    //

    modelProcessados.setFilter(idVenda == "0"
                                   ? "idPagamento = " + idPagamento +
                                         " AND status != 'PENDENTE' AND status != 'CANCELADO' AND representacao = 0"
                                   : "idVenda = '" + idVenda +
                                         "' AND status != 'PENDENTE' AND status != 'CANCELADO'AND representacao = 0");

    if (not modelProcessados.select()) {
      QMessageBox::critical(this, "Erro!",
                            "Erro lendo tabela conta_a_receber_has_pagamento: " + modelProcessados.lastError().text());
    }

    for (int row = 0, rowCount = modelProcessados.rowCount(); row < rowCount; ++row) {
      ui->tableProcessados->openPersistentEditor(row, "contaDestino");
      ui->tableProcessados->openPersistentEditor(row, "centroCusto");
    }

    ui->tableProcessados->resizeColumnsToContents();
  }

  if (tipo == Pagar) {
    QSqlQuery query;
    query.prepare("SELECT cp.idCompra, pf.ordemCompra FROM conta_a_pagar_has_pagamento cp LEFT JOIN "
                  "pedido_fornecedor_has_produto pf ON cp.idCompra = pf.idCompra WHERE idPagamento = :idPagamento");
    query.bindValue(":idPagamento", idPagamento);

    if (not query.exec() or not query.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando dados: " + query.lastError().text());
      return;
    }

    const QString idCompra = query.value("idCompra").toString();
    const QString ordemCompra = query.value("ordemCompra").toString();

    setWindowTitle("Contas A Pagar - " + contraparte + (ordemCompra == "0" ? "" : " OC " + ordemCompra));

    modelPendentes.setFilter(idCompra == "0" ? "idPagamento = " + idPagamento + " AND status = 'PENDENTE'"
                                             : "idCompra = '" + idCompra + "' AND status = 'PENDENTE'");

    if (not modelPendentes.select()) {
      QMessageBox::critical(this, "Erro!",
                            "Erro lendo tabela conta_a_receber_has_pagamento: " + modelPendentes.lastError().text());
    }

    ui->tablePendentes->resizeColumnsToContents();

    for (int row = 0, rowCount = modelPendentes.rowCount(); row < rowCount; ++row) {
      ui->tablePendentes->openPersistentEditor(row, "status");
      ui->tablePendentes->openPersistentEditor(row, "contaDestino");
      ui->tablePendentes->openPersistentEditor(row, "centroCusto");
    }

    //

    modelProcessados.setFilter(idCompra == "0" ? "idPagamento = " + idPagamento + " AND status != 'PENDENTE'"
                                               : "idCompra = '" + idCompra + "' AND status != 'PENDENTE'");

    if (not modelProcessados.select()) {
      QMessageBox::critical(this, "Erro!",
                            "Erro lendo tabela conta_a_receber_has_pagamento: " + modelProcessados.lastError().text());
    }

    for (int row = 0, rowCount = modelProcessados.rowCount(); row < rowCount; ++row) {
      ui->tableProcessados->openPersistentEditor(row, "contaDestino");
      ui->tableProcessados->openPersistentEditor(row, "centroCusto");
    }

    ui->tableProcessados->resizeColumnsToContents();
  }
}

void Contas::on_tablePendentes_entered(const QModelIndex &) { ui->tablePendentes->resizeColumnsToContents(); }

void Contas::on_tableProcessados_entered(const QModelIndex &) { ui->tableProcessados->resizeColumnsToContents(); }
