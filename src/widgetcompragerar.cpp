#include <QDate>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>

#include "checkboxdelegate.h"
#include "inputdialog.h"
#include "sendmail.h"
#include "ui_widgetcompragerar.h"
#include "usersession.h"
#include "widgetcompragerar.h"
#include "xlsxdocument.h"

WidgetCompraGerar::WidgetCompraGerar(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompraGerar) {
  ui->setupUi(this);

  setupTables();
}

WidgetCompraGerar::~WidgetCompraGerar() { delete ui; }

void WidgetCompraGerar::setupTables() {
  modelPedForn.setTable("view_fornecedor_compra");

  modelPedForn.setHeaderData("fornecedor", "Fornecedor");
  modelPedForn.setHeaderData("COUNT(fornecedor)", "Itens");

  ui->tableFornCompras->setModel(&modelPedForn);

  modelItemPedidosPend.setTable("pedido_fornecedor_has_produto");
  modelItemPedidosPend.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelItemPedidosPend.setHeaderData("selecionado", "");
  modelItemPedidosPend.setHeaderData("fornecedor", "Fornecedor");
  modelItemPedidosPend.setHeaderData("descricao", "Descrição");
  modelItemPedidosPend.setHeaderData("colecao", "Coleção");
  modelItemPedidosPend.setHeaderData("quant", "Quant.");
  modelItemPedidosPend.setHeaderData("un", "Un.");
  modelItemPedidosPend.setHeaderData("preco", "Preço");
  modelItemPedidosPend.setHeaderData("formComercial", "Form. Com.");
  modelItemPedidosPend.setHeaderData("codComercial", "Cód. Com.");
  modelItemPedidosPend.setHeaderData("codBarras", "Cód. Bar.");
  modelItemPedidosPend.setHeaderData("idCompra", "Compra");
  modelItemPedidosPend.setHeaderData("dataPrevCompra", "Prev. Compra");
  modelItemPedidosPend.setHeaderData("dataCompra", "Data Compra");
  modelItemPedidosPend.setHeaderData("status", "Status");

  modelItemPedidosPend.setFilter("status = 'PENDENTE'");

  ui->tablePedidosPend->setModel(&modelItemPedidosPend);
  ui->tablePedidosPend->setItemDelegateForColumn("selecionado", new CheckBoxDelegate(this));
  ui->tablePedidosPend->hideColumn("quantUpd");
  ui->tablePedidosPend->hideColumn("idPedido");
  ui->tablePedidosPend->hideColumn("idLoja");
  ui->tablePedidosPend->hideColumn("item");
  ui->tablePedidosPend->hideColumn("idProduto");
  ui->tablePedidosPend->hideColumn("prcUnitario");
  ui->tablePedidosPend->hideColumn("parcial");
  ui->tablePedidosPend->hideColumn("desconto");
  ui->tablePedidosPend->hideColumn("parcialDesc");
  ui->tablePedidosPend->hideColumn("descGlobal");
  ui->tablePedidosPend->hideColumn("dataRealCompra");
  ui->tablePedidosPend->hideColumn("dataPrevConf");
  ui->tablePedidosPend->hideColumn("dataRealConf");
  ui->tablePedidosPend->hideColumn("dataPrevFat");
  ui->tablePedidosPend->hideColumn("dataRealFat");
  ui->tablePedidosPend->hideColumn("dataPrevColeta");
  ui->tablePedidosPend->hideColumn("dataRealColeta");
  ui->tablePedidosPend->hideColumn("dataPrevEnt");
  ui->tablePedidosPend->hideColumn("dataRealEnt");
  ui->tablePedidosPend->hideColumn("dataPrevReceb");
  ui->tablePedidosPend->hideColumn("dataRealReceb");
}

QString WidgetCompraGerar::updateTables() {
  modelPedForn.select();

  ui->tableFornCompras->resizeColumnsToContents();

  modelItemPedidosPend.setFilter("0");

  if (not modelItemPedidosPend.select()) {
    return "Erro lendo tabela pedido_fornecedor_has_produto: " + modelItemPedidosPend.lastError().text();
  }

  for (int i = 0; i < modelItemPedidosPend.rowCount(); ++i) {
    ui->tablePedidosPend->openPersistentEditor(
          modelItemPedidosPend.index(i, modelItemPedidosPend.fieldIndex("selecionado")));
  }

  ui->tablePedidosPend->resizeColumnsToContents();

  return QString();
}

void WidgetCompraGerar::on_pushButtonGerarCompra_clicked() {
  // NOTE: refactor this function into smaller functions

  QSqlQuery("SET SESSION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  QList<int> lista;
  QStringList ids;

  for (const auto index :
       modelItemPedidosPend.match(modelItemPedidosPend.index(0, modelItemPedidosPend.fieldIndex("selecionado")),
                                  Qt::DisplayRole, true, -1, Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap))) {
    lista.append(index.row());
    ids.append(modelItemPedidosPend.data(index.row(), "idPedido").toString());

    // is this necessary?
    //    QString status = modelItemPedidosPend.data(index.row(), "status").toString();

    //    if (status != "PENDENTE") {
    //      modelItemPedidosPend.select();
    //      QMessageBox::critical(this, "Erro!", "Produto não estava pendente: " + status);
    //      QSqlQuery("ROLLBACK").exec();
    //      return;
    //    }
  }

  if (lista.size() == 0) {
    QMessageBox::critical(this, "Aviso!", "Nenhum item selecionado!");
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QString filtro = modelItemPedidosPend.filter();

  InputDialog *inputDlg = new InputDialog(InputDialog::GerarCompra, this);
  inputDlg->setFilter(ids);

  if (inputDlg->exec() != InputDialog::Accepted) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  const QDate dataCompra = inputDlg->getDate();
  const QDate dataPrevista = inputDlg->getNextDate();

  modelItemPedidosPend.setFilter(filtro);
  modelItemPedidosPend.select();

  QStringList produtos;

  for (const auto row : lista) {
    QString produto = modelItemPedidosPend.data(row, "descricao").toString() + ", Quant: " +
                      modelItemPedidosPend.data(row, "quant").toString() + ", R$ " +
                      modelItemPedidosPend.data(row, "preco").toString().replace(".", ",");
    produtos.append(produto);
  }

  //------------------------------
  QString idVenda = gerarExcel(lista);

  if (idVenda.isEmpty()) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  //------------------------------
  QString path = settings("User/userFolder").toString();

  QString arquivo = path + "/" + idVenda + ".xlsx";

  QFile file(arquivo);

  if (not file.exists()) {
    QMessageBox::critical(this, "Erro!", "Arquivo não encontrado");
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QMessageBox msgBox(QMessageBox::Question, "Enviar E-mail?", "Deseja enviar e-mail?",
                     QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Sim");
  msgBox.setButtonText(QMessageBox::No, "Não");

  if (msgBox.exec() == QMessageBox::Yes) {
    SendMail *mail = new SendMail(this, produtos.join("\n"), arquivo);

    if (mail->exec() != SendMail::Accepted) {
      return;
    }
  }

  modelItemPedidosPend.setFilter(filtro);
  modelItemPedidosPend.select();

  for (const auto row : lista) {
    if (not modelItemPedidosPend.setData(row, "status", "EM COMPRA")) {
      QMessageBox::critical(this, "Erro!",
                            "Erro marcando status EM COMPRA: " + modelItemPedidosPend.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }

    QSqlQuery queryId;

    if (not queryId.exec("SELECT idCompra FROM pedido_fornecedor_has_produto ORDER BY idCompra DESC")) {
      QMessageBox::critical(this, "Erro!", "Erro buscando idCompra: " + queryId.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }

    QString id = queryId.first() ? QString::number(queryId.value("idCompra").toInt() + 1) : "1";

    if (not modelItemPedidosPend.setData(row, "idCompra", id)) {
      QMessageBox::critical(this, "Erro!", "Erro guardando idCompra: " + modelItemPedidosPend.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }

    // salvar status na venda
    QSqlQuery query;
    query.prepare("UPDATE venda_has_produto SET idCompra = :idCompra, dataRealCompra = :dataRealCompra, dataPrevConf = "
                  ":dataPrevConf, "
                  "status = 'EM COMPRA' WHERE idProduto = :idProduto");
    query.bindValue(":idCompra", id);
    query.bindValue(":dataRealCompra", dataCompra);
    query.bindValue(":dataPrevConf", dataPrevista);
    query.bindValue(":idProduto", modelItemPedidosPend.data(row, "idProduto"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status da venda: " + query.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }

    if (not query.exec("CALL update_venda_status()")) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }
    //

    if (not modelItemPedidosPend.setData(row, "dataRealCompra", dataCompra.toString("yyyy-MM-dd"))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando data da compra: " + modelItemPedidosPend.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }

    if (not modelItemPedidosPend.setData(row, "dataPrevConf", dataPrevista.toString("yyyy-MM-dd"))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando data prevista: " + modelItemPedidosPend.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }
  }

  QSqlQuery query;

  for (auto const &row : lista) {
    query.prepare("INSERT INTO conta_a_pagar_has_pagamento (dataEmissao, idVenda, idLoja, valor, tipo, parcela, "
                  "observacao, status) VALUES (:dataEmissao, :idVenda, :idLoja, :valor, :tipo, :parcela, :observacao, "
                  ":status)");
    query.bindValue(":dataEmissao", QDate::currentDate().toString("yyyy-MM-dd"));
    query.bindValue(":idVenda", idVenda);
    query.bindValue(":idLoja", UserSession::getLoja());
    query.bindValue(":valor", modelItemPedidosPend.data(row, "preco"));
    query.bindValue(":tipo", "A CONFIRMAR");
    query.bindValue(":parcela", 1);
    query.bindValue(":observacao", "");
    query.bindValue(":status", "PENDENTE");

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro guardando conta a pagar pagamento: " + query.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }
  }

  if (not modelItemPedidosPend.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados da tabela pedido_fornecedor_has_produto: " +
                          modelItemPedidosPend.lastError().text());
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QSqlQuery("COMMIT").exec();

  updateTables();
}

QString WidgetCompraGerar::gerarExcel(QList<int> lista) {
  // NOTE: refactor this to use excel class (verify what should be in this file)
  if (settings("User/userFolder").toString().isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Não há uma pasta definida para salvar PDF/Excel. Por favor escolha uma.");
    setSettings("User/userFolder", QFileDialog::getExistingDirectory(this, "Pasta PDF/Excel"));
    return QString();
  }

  if (settings("User/userFolder").toString().isEmpty()) {
    return QString();
  }

  QString path = settings("User/userFolder").toString();

  QDir dir(path);

  if (not dir.exists()) {
    dir.mkdir(path);
  }

  QFile modelo(QDir::currentPath() + "/modelo.xlsx");

  if (not modelo.exists()) {
    QMessageBox::critical(this, "Erro!", "Não encontrou o modelo do Excel!");
    return QString();
  }

  // NOTE: split in two pages?
  if (lista.size() > 17) {
    QMessageBox::critical(this, "Erro!", "Mais itens do que cabe no modelo!");
    return QString();
  }

  QSqlQuery queryVenda;
  queryVenda.prepare("SELECT * FROM venda_has_produto WHERE idProduto = :idProduto");
  queryVenda.bindValue(":idProduto", modelItemPedidosPend.data(lista.first(), "idProduto"));

  if (not queryVenda.exec() or not queryVenda.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados da venda:" + queryVenda.lastError().text());
    return QString();
  }

  QString idVenda = queryVenda.value("idVenda").toString();

  QSqlQuery queryForn;
  queryForn.prepare("SELECT * FROM fornecedor WHERE razaoSocial = (SELECT fornecedor FROM venda_has_produto WHERE "
                    "idVenda = :idVenda LIMIT 1)");
  queryForn.bindValue(":idVenda", idVenda);

  if (not queryForn.exec() or not queryForn.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados do fornecedor: " + queryForn.lastError().text());
    return QString();
  }

  QXlsx::Document xlsx("modelo.xlsx");

  xlsx.write("F4", idVenda); // NOTE: temp, replace with correct id
  xlsx.write("E6", queryForn.value("razaoSocial"));
  xlsx.write("E8", queryForn.value("contatoNome"));
  xlsx.write("D10", "Data: " + QDate::currentDate().toString("dd/MM/yy"));

  for (auto const &row : lista) {
    static int i = 0;

    xlsx.write("A" + QString::number(13 + i), i + 1);
    xlsx.write("B" + QString::number(13 + i), modelItemPedidosPend.data(row, "codComercial"));
    xlsx.write("C" + QString::number(13 + i), modelItemPedidosPend.data(row, "descricao"));
    xlsx.write("E" + QString::number(13 + i), (modelItemPedidosPend.data(row, "preco").toDouble() /
                                               modelItemPedidosPend.data(row, "quant").toDouble()));
    xlsx.write("F" + QString::number(13 + i), modelItemPedidosPend.data(row, "un"));
    xlsx.write("G" + QString::number(13 + i), modelItemPedidosPend.data(row, "quant"));

    ++i;
  }

  if (not xlsx.saveAs(path + "/" + idVenda + ".xlsx")) {
    QMessageBox::critical(this, "Erro!", "Ocorreu algum erro ao salvar o arquivo.");
    return QString();
  }

  QMessageBox::information(this, "Ok!", "Arquivo salvo como " + path + "/" + idVenda + ".xlsx");
  return idVenda;
}

QVariant WidgetCompraGerar::settings(const QString &key) const { return UserSession::getSettings(key); }

void WidgetCompraGerar::setSettings(const QString &key, const QVariant &value) const {
  UserSession::setSettings(key, value);
}

void WidgetCompraGerar::on_checkBoxTodosGerar_clicked(const bool &checked) {
  for (int row = 0; row < modelItemPedidosPend.rowCount(); ++row) {
    modelItemPedidosPend.setData(row, "selecionado", checked);
  }
}

void WidgetCompraGerar::on_tableFornCompras_activated(const QModelIndex &index) {
  const QString fornecedor = modelPedForn.data(index.row(), "fornecedor").toString();

  modelItemPedidosPend.setFilter("fornecedor = '" + fornecedor + "' AND status = 'PENDENTE'");

  if (not modelItemPedidosPend.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela pedido_fornecedor_has_produto: " +
                          modelItemPedidosPend.lastError().text());
    return;
  }

  for (int row = 0; row < modelItemPedidosPend.rowCount(); ++row) {
    ui->tablePedidosPend->openPersistentEditor(
          modelItemPedidosPend.index(row, modelItemPedidosPend.fieldIndex("selecionado")));
  }

  ui->checkBoxTodosGerar->setChecked(false);

  ui->tablePedidosPend->resizeColumnsToContents();
}
