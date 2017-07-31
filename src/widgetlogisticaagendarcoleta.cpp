#include <QDate>
#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QInputDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardItemModel>
#include <QUrl>

#include "acbr.h"
#include "doubledelegate.h"
#include "estoqueprazoproxymodel.h"
#include "inputdialog.h"
#include "ui_widgetlogisticaagendarcoleta.h"
#include "usersession.h"
#include "venda.h"
#include "widgetlogisticaagendarcoleta.h"

WidgetLogisticaAgendarColeta::WidgetLogisticaAgendarColeta(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetLogisticaAgendarColeta) {
  ui->setupUi(this);

  ui->frameCaminhao->hide();
  ui->pushButtonCancelarCarga->hide();

  ui->dateTimeEdit->setDate(QDate::currentDate());
}

WidgetLogisticaAgendarColeta::~WidgetLogisticaAgendarColeta() { delete ui; }

void WidgetLogisticaAgendarColeta::setupTables() {
  modelEstoque.setTable("view_agendar_coleta");
  modelEstoque.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelEstoque.setFilter("0");

  if (not modelEstoque.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + modelEstoque.lastError().text());
    return;
  }

  modelEstoque.setHeaderData("dataRealFat", "Data Faturado");
  modelEstoque.setHeaderData("idEstoque", "Estoque");
  modelEstoque.setHeaderData("codComercial", "Cód. Com.");
  modelEstoque.setHeaderData("fornecedor", "Fornecedor");
  modelEstoque.setHeaderData("numeroNFe", "NFe");
  modelEstoque.setHeaderData("produto", "Produto");
  modelEstoque.setHeaderData("quant", "Quant.");
  modelEstoque.setHeaderData("un", "Un.");
  modelEstoque.setHeaderData("caixas", "Cx.");
  modelEstoque.setHeaderData("kgcx", "Kg./Cx.");
  modelEstoque.setHeaderData("idVenda", "Venda");
  modelEstoque.setHeaderData("ordemCompra", "OC");
  modelEstoque.setHeaderData("local", "Local");
  modelEstoque.setHeaderData("prazoEntrega", "Prazo Limite");

  ui->tableEstoque->setModel(new EstoquePrazoProxyModel(&modelEstoque, this));
  ui->tableEstoque->setItemDelegate(new DoubleDelegate(this));
  ui->tableEstoque->hideColumn("prazoEntrega");
  ui->tableEstoque->hideColumn("fornecedor");
  ui->tableEstoque->hideColumn("unCaixa");
  ui->tableEstoque->hideColumn("formComercial");
  ui->tableEstoque->hideColumn("idProduto");
  ui->tableEstoque->hideColumn("idNFe");
  ui->tableEstoque->hideColumn("ordemCompra");
  ui->tableEstoque->hideColumn("local");

  //

  modelTransp.setTable("veiculo_has_produto");
  modelTransp.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelTransp.setHeaderData("idEstoque", "Estoque");
  modelTransp.setHeaderData("status", "Status");
  modelTransp.setHeaderData("produto", "Produto");
  modelTransp.setHeaderData("caixas", "Cx.");
  modelTransp.setHeaderData("quant", "Quant.");
  modelTransp.setHeaderData("un", "Un.");
  modelTransp.setHeaderData("codComercial", "Cód. Com.");

  modelTransp.setFilter("0");

  if (not modelTransp.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela transportadora: " + modelTransp.lastError().text());
    return;
  }

  ui->tableTransp->setModel(&modelTransp);
  ui->tableTransp->hideColumn("id");
  ui->tableTransp->hideColumn("idEvento");
  ui->tableTransp->hideColumn("idVeiculo");
  ui->tableTransp->hideColumn("idVendaProduto");
  ui->tableTransp->hideColumn("idCompra");
  ui->tableTransp->hideColumn("idNfeSaida");
  ui->tableTransp->hideColumn("fornecedor");
  ui->tableTransp->hideColumn("idVenda");
  ui->tableTransp->hideColumn("idLoja");
  ui->tableTransp->hideColumn("idProduto");
  ui->tableTransp->hideColumn("obs");
  ui->tableTransp->hideColumn("unCaixa");
  ui->tableTransp->hideColumn("formComercial");
  ui->tableTransp->hideColumn("data");

  //

  modelTransp2.setTable("veiculo_has_produto");
  modelTransp2.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelTransp2.setHeaderData("idEstoque", "Estoque");
  modelTransp2.setHeaderData("data", "Agendado");
  modelTransp2.setHeaderData("status", "Status");
  modelTransp2.setHeaderData("produto", "Produto");
  modelTransp2.setHeaderData("caixas", "Cx.");
  modelTransp2.setHeaderData("quant", "Quant.");
  modelTransp2.setHeaderData("un", "Un.");
  modelTransp2.setHeaderData("codComercial", "Cód. Com.");

  modelTransp2.setFilter("0");

  if (not modelTransp2.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela transportadora: " + modelTransp2.lastError().text());
    return;
  }

  ui->tableTransp2->setModel(&modelTransp2);
  ui->tableTransp2->hideColumn("id");
  ui->tableTransp2->hideColumn("idEvento");
  ui->tableTransp2->hideColumn("idVeiculo");
  ui->tableTransp2->hideColumn("idVendaProduto");
  ui->tableTransp2->hideColumn("idCompra");
  ui->tableTransp2->hideColumn("idNfeSaida");
  ui->tableTransp2->hideColumn("fornecedor");
  ui->tableTransp2->hideColumn("idVenda");
  ui->tableTransp2->hideColumn("idLoja");
  ui->tableTransp2->hideColumn("idProduto");
  ui->tableTransp2->hideColumn("unCaixa");
  ui->tableTransp2->hideColumn("obs");
  ui->tableTransp2->hideColumn("formComercial");
}

void WidgetLogisticaAgendarColeta::calcularPeso() {
  double peso = 0;
  double caixas = 0;

  const auto list = ui->tableEstoque->selectionModel()->selectedRows();

  for (auto const &item : list) {
    const double kg = modelEstoque.data(item.row(), "kgcx").toDouble();
    const double caixa = modelEstoque.data(item.row(), "caixas").toDouble();
    peso += kg * caixa;
    caixas += caixa;
  }

  ui->doubleSpinBoxPeso->setValue(peso);
  ui->doubleSpinBoxPeso->setSuffix(" Kg (" + QString::number(caixas) + " Cx.)");
}

bool WidgetLogisticaAgendarColeta::updateTables() {
  if (modelEstoque.tableName().isEmpty()) {
    setupTables();

    ui->itemBoxVeiculo->setSearchDialog(SearchDialog::veiculo(this));

    connect(ui->tableEstoque->selectionModel(), &QItemSelectionModel::selectionChanged, this, &WidgetLogisticaAgendarColeta::calcularPeso);
  }

  if (not modelEstoque.select()) {
    emit errorSignal("Erro lendo tabela estoque: " + modelEstoque.lastError().text());
    return false;
  }

  ui->tableEstoque->resizeColumnsToContents();

  if (not modelTransp2.select()) {
    emit errorSignal("Erro lendo tabela veiculo: " + modelTransp2.lastError().text());
    return false;
  }

  ui->tableTransp2->resizeColumnsToContents();

  return true;
}

void WidgetLogisticaAgendarColeta::tableFornLogistica_activated(const QString &fornecedor) {
  this->fornecedor = fornecedor;

  ui->lineEditBusca->clear();

  modelEstoque.setFilter("fornecedor = '" + fornecedor + "' AND idVenda IS " + QString(ui->checkBoxEstoque->isChecked() ? "NULL" : "NOT NULL"));

  if (not modelEstoque.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + modelEstoque.lastError().text());
    return;
  }

  ui->tableEstoque->sortByColumn("prazoEntrega");

  ui->tableEstoque->resizeColumnsToContents();
}

void WidgetLogisticaAgendarColeta::on_pushButtonMontarCarga_clicked() {
  if (not ui->frameCaminhao->isVisible()) {
    ui->frameCaminhao->setVisible(true);
    ui->pushButtonAgendarColeta->hide();
    ui->pushButtonCancelarCarga->show();
    return;
  }

  if (modelTransp.rowCount() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item no veículo!");
    return;
  }

  QModelIndexList list;

  for (int row = 0; row < modelTransp.rowCount(); ++row) list << modelTransp.index(row, 0);

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not processRows(list, ui->dateTimeEdit->date(), true)) {
    QSqlQuery("ROLLBACK").exec();
    if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
    return;
  }

  QSqlQuery("COMMIT").exec();

  updateTables();
  QMessageBox::information(this, "Aviso!", "Agendado com sucesso!");

  ui->frameCaminhao->setVisible(false);
}

void WidgetLogisticaAgendarColeta::on_pushButtonAgendarColeta_clicked() {
  const auto list = ui->tableEstoque->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  InputDialog input(InputDialog::AgendarColeta, this);
  // TODO: colocar qual a linha/id esta sendo trabalhada para o usuario nao se perder ao trocar de janela e voltar

  if (input.exec() != InputDialog::Accepted) return;

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not processRows(list, input.getNextDate())) {
    QSqlQuery("ROLLBACK").exec();
    if (not error.isEmpty()) QMessageBox::critical(this, "Erro!", error);
    return;
  }

  QSqlQuery("COMMIT").exec();

  updateTables();
  QMessageBox::information(this, "Aviso!", "Agendado com sucesso!");
}

bool WidgetLogisticaAgendarColeta::processRows(const QModelIndexList &list, const QDate &dataPrevColeta, const bool montarCarga) {
  for (auto const &item : list) {
    int idEstoque;
    QString codComercial;

    if (montarCarga) {
      QSqlQuery query;
      query.exec("SELECT COALESCE(MAX(idEvento), 0) + 1 FROM veiculo_has_produto");
      query.first();

      const int idEvento = query.value(0).toInt();

      if (not modelTransp.setData(item.row(), "data", dataPrevColeta)) return false;
      if (not modelTransp.setData(item.row(), "idEvento", idEvento)) return false;

      idEstoque = modelTransp.data(item.row(), "idEstoque").toInt();

      QSqlQuery queryTemp;
      queryTemp.prepare("SELECT codComercial FROM estoque WHERE idEstoque = :idEstoque");
      queryTemp.bindValue(":idEstoque", idEstoque);

      if (not queryTemp.exec() or not queryTemp.first()) {
        error = "Erro buscando codComercial: " + queryTemp.lastError().text();
        return false;
      }

      codComercial = queryTemp.value("codComercial").toString();
    } else {
      idEstoque = modelEstoque.data(item.row(), "idEstoque").toInt();
      codComercial = modelEstoque.data(item.row(), "codComercial").toString();
    }

    QSqlQuery query;
    query.prepare("UPDATE estoque SET status = 'EM COLETA' WHERE idEstoque = :idEstoque");
    query.bindValue(":idEstoque", idEstoque);

    if (not query.exec()) {
      error = "Erro salvando status no estoque: " + query.lastError().text();
      return false;
    }

    query.prepare("UPDATE pedido_fornecedor_has_produto SET dataPrevColeta = :dataPrevColeta WHERE idCompra IN (SELECT "
                  "idCompra FROM estoque_has_compra WHERE idEstoque = :idEstoque) AND codComercial = :codComercial");
    query.bindValue(":dataPrevColeta", dataPrevColeta);
    query.bindValue(":idEstoque", idEstoque);
    query.bindValue(":codComercial", codComercial);

    if (not query.exec()) {
      error = "Erro salvando status no pedido_fornecedor: " + query.lastError().text();
      return false;
    }

    query.prepare("UPDATE venda_has_produto SET dataPrevColeta = :dataPrevColeta WHERE idVendaProduto IN (SELECT "
                  "idVendaProduto FROM estoque_has_consumo WHERE idEstoque = :idEstoque)");
    query.bindValue(":dataPrevColeta", dataPrevColeta);
    query.bindValue(":idEstoque", idEstoque);

    if (not query.exec()) {
      error = "Erro salvando status na venda_produto: " + query.lastError().text();
      return false;
    }
  }

  if (not modelTransp.submitAll()) {
    error = "Erro salvando carga veiculo: " + modelTransp.lastError().text();
    return false;
  }

  return true;
}

void WidgetLogisticaAgendarColeta::on_tableEstoque_entered(const QModelIndex &) { ui->tableEstoque->resizeColumnsToContents(); }

void WidgetLogisticaAgendarColeta::on_itemBoxVeiculo_textChanged(const QString &) {
  QSqlQuery query;
  query.prepare("SELECT capacidade FROM transportadora_has_veiculo WHERE idVeiculo = :idVeiculo");
  query.bindValue(":idVeiculo", ui->itemBoxVeiculo->getValue());

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados veiculo: " + query.lastError().text());
    return;
  }

  modelTransp2.setFilter("idVeiculo = " + ui->itemBoxVeiculo->getValue().toString() + " AND status != 'FINALIZADO' AND DATE(data) = '" + ui->dateTimeEdit->date().toString("yyyy-MM-dd") + "'");

  if (not modelTransp2.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela veiculo: " + modelTransp2.lastError().text());
    return;
  }

  ui->tableTransp2->resizeColumnsToContents();

  ui->doubleSpinBoxCapacidade->setValue(query.value("capacidade").toDouble());
}

bool WidgetLogisticaAgendarColeta::adicionarProduto(const QModelIndexList &list) {
  for (auto const &item : list) {
    const int row = modelTransp.rowCount();
    modelTransp.insertRow(row);

    //

    const double kg = modelEstoque.data(item.row(), "kgcx").toDouble();
    const double caixa = modelEstoque.data(item.row(), "caixas").toDouble();
    const double peso = kg * caixa;

    //

    if (not modelTransp.setData(row, "fornecedor", modelEstoque.data(item.row(), "fornecedor"))) return false;
    if (not modelTransp.setData(row, "unCaixa", modelEstoque.data(item.row(), "unCaixa"))) return false;
    if (not modelTransp.setData(row, "formComercial", modelEstoque.data(item.row(), "formComercial"))) return false;
    if (not modelTransp.setData(row, "idVeiculo", ui->itemBoxVeiculo->getValue())) return false;
    if (not modelTransp.setData(row, "idEstoque", modelEstoque.data(item.row(), "idEstoque"))) return false;
    if (not modelTransp.setData(row, "idProduto", modelEstoque.data(item.row(), "idProduto"))) return false;
    if (not modelTransp.setData(row, "produto", modelEstoque.data(item.row(), "produto"))) return false;
    if (not modelTransp.setData(row, "codComercial", modelEstoque.data(item.row(), "codComercial"))) return false;
    if (not modelTransp.setData(row, "un", modelEstoque.data(item.row(), "un"))) return false;
    if (not modelTransp.setData(row, "caixas", modelEstoque.data(item.row(), "caixas"))) return false;
    if (not modelTransp.setData(row, "kg", peso)) return false;
    if (not modelTransp.setData(row, "quant", modelEstoque.data(item.row(), "quant"))) return false;
    if (not modelTransp.setData(row, "status", "EM COLETA")) return false;
  }

  ui->tableTransp->resizeColumnsToContents();

  return true;
}

void WidgetLogisticaAgendarColeta::on_pushButtonAdicionarProduto_clicked() {
  if (ui->itemBoxVeiculo->getValue().isNull()) {
    QMessageBox::critical(this, "Erro!", "Deve escolher uma transportadora antes!");
    return;
  }

  const auto list = ui->tableEstoque->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  if (ui->doubleSpinBoxPeso->value() > ui->doubleSpinBoxCapacidade->value()) {
    QMessageBox::critical(this, "Erro!", "Peso maior que capacidade do veículo!");
    return;
  }

  if (not adicionarProduto(list)) modelTransp.select();
}

void WidgetLogisticaAgendarColeta::on_pushButtonRemoverProduto_clicked() {
  const auto list = ui->tableTransp->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  for (auto const &item : list) modelTransp.removeRow(item.row());

  modelTransp.submitAll();
}

void WidgetLogisticaAgendarColeta::on_pushButtonCancelarCarga_clicked() {
  QMessageBox msgBox(QMessageBox::Question, "Cancelar?", "Tem certeza que deseja cancelar?", QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Cancelar");
  msgBox.setButtonText(QMessageBox::No, "Voltar");

  if (msgBox.exec() == QMessageBox::No) return;

  ui->frameCaminhao->hide();
  ui->pushButtonAgendarColeta->show();
  ui->pushButtonCancelarCarga->hide();

  modelTransp.select();
}

void WidgetLogisticaAgendarColeta::on_pushButtonDanfe_clicked() {
  const auto list = ui->tableEstoque->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  ACBr::gerarDanfe(modelEstoque.data(list.first().row(), "idNFe").toInt());
}

void WidgetLogisticaAgendarColeta::on_lineEditBusca_textChanged(const QString &text) {
  modelEstoque.setFilter("(numeroNFe LIKE '%" + text + "%' OR produto LIKE '%" + text + "%' OR idVenda LIKE '%" + text + "%' OR ordemCompra LIKE '%" + text + "%')");

  if (not modelEstoque.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + modelEstoque.lastError().text());
}

void WidgetLogisticaAgendarColeta::on_dateTimeEdit_dateChanged(const QDate &date) {
  if (ui->itemBoxVeiculo->text().isEmpty()) return;

  modelTransp2.setFilter("idVeiculo = " + ui->itemBoxVeiculo->getValue().toString() + " AND status != 'FINALIZADO' AND DATE(data) = '" + date.toString("yyyy-MM-dd") + "'");

  if (not modelTransp2.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela veiculo: " + modelTransp2.lastError().text());
    return;
  }

  ui->tableTransp2->resizeColumnsToContents();
}

void WidgetLogisticaAgendarColeta::on_checkBoxEstoque_toggled(bool checked) {
  modelEstoque.setFilter("fornecedor = '" + fornecedor + "' AND idVenda IS " + QString(checked ? "NULL" : "NOT NULL"));

  if (not modelEstoque.select()) {
    QMessageBox::critical(this, "Erro!", "Erro: " + modelEstoque.lastError().text());
    return;
  }
}

void WidgetLogisticaAgendarColeta::on_pushButtonVenda_clicked() {
  const auto list = ui->tableEstoque->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  for (auto const &item : list) {
    const QString idVenda = modelEstoque.data(item.row(), "idVenda").toString();
    const QStringList ids = idVenda.split(", ");

    if (ids.isEmpty()) return;

    for (auto const &id : ids) {
      auto *venda = new Venda(this);
      venda->setAttribute(Qt::WA_DeleteOnClose);
      venda->viewRegisterById(id);
    }
  }
}

// TODO: 1poder marcar nota de entrada como cancelada (talvez direto na tela de nfe's e retirar dos fluxos os estoques?)
// TODO: importar nota de amostra nesta tela dizendo para qual loja ela vai e no final do fluxo gerar nota de
// tranferencia
