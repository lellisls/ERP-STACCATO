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
  modelForn.setTable("view_fornecedor_compra");

  modelForn.setHeaderData("fornecedor", "Fornecedor");
  modelForn.setHeaderData("COUNT(fornecedor)", "Itens");

  ui->tableForn->setModel(&modelForn);

  modelProdutos.setTable("pedido_fornecedor_has_produto");
  modelProdutos.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelProdutos.setHeaderData("selecionado", "");
  modelProdutos.setHeaderData("fornecedor", "Fornecedor");
  modelProdutos.setHeaderData("descricao", "Descrição");
  modelProdutos.setHeaderData("colecao", "Coleção");
  modelProdutos.setHeaderData("caixas", "Caixas");
  modelProdutos.setHeaderData("quant", "Quant.");
  modelProdutos.setHeaderData("un", "Un.");
  modelProdutos.setHeaderData("un2", "Un.2");
  modelProdutos.setHeaderData("preco", "Preço");
  modelProdutos.setHeaderData("kgcx", "Kg./Cx.");
  modelProdutos.setHeaderData("formComercial", "Form. Com.");
  modelProdutos.setHeaderData("codComercial", "Cód. Com.");
  modelProdutos.setHeaderData("codBarras", "Cód. Bar.");
  modelProdutos.setHeaderData("idCompra", "Compra");
  modelProdutos.setHeaderData("dataPrevCompra", "Prev. Compra");
  modelProdutos.setHeaderData("dataCompra", "Data Compra");
  modelProdutos.setHeaderData("status", "Status");

  modelProdutos.setFilter("status = 'PENDENTE'");

  ui->tableProdutos->setModel(&modelProdutos);
  ui->tableProdutos->setItemDelegateForColumn("selecionado", new CheckBoxDelegate(this));
  ui->tableProdutos->hideColumn("idNfe");
  ui->tableProdutos->hideColumn("idEstoque");
  ui->tableProdutos->hideColumn("quantUpd");
  ui->tableProdutos->hideColumn("idPedido");
  ui->tableProdutos->hideColumn("idLoja");
  ui->tableProdutos->hideColumn("item");
  ui->tableProdutos->hideColumn("idProduto");
  ui->tableProdutos->hideColumn("prcUnitario");
  ui->tableProdutos->hideColumn("parcial");
  ui->tableProdutos->hideColumn("desconto");
  ui->tableProdutos->hideColumn("parcialDesc");
  ui->tableProdutos->hideColumn("descGlobal");
  ui->tableProdutos->hideColumn("dataRealCompra");
  ui->tableProdutos->hideColumn("dataPrevConf");
  ui->tableProdutos->hideColumn("dataRealConf");
  ui->tableProdutos->hideColumn("dataPrevFat");
  ui->tableProdutos->hideColumn("dataRealFat");
  ui->tableProdutos->hideColumn("dataPrevColeta");
  ui->tableProdutos->hideColumn("dataRealColeta");
  ui->tableProdutos->hideColumn("dataPrevEnt");
  ui->tableProdutos->hideColumn("dataRealEnt");
  ui->tableProdutos->hideColumn("dataPrevReceb");
  ui->tableProdutos->hideColumn("dataRealReceb");
}

QString WidgetCompraGerar::updateTables() {
  modelForn.select();

  ui->tableForn->resizeColumnsToContents();

  modelProdutos.setFilter("0");

  if (not modelProdutos.select()) {
    return "Erro lendo tabela pedido_fornecedor_has_produto: " + modelProdutos.lastError().text();
  }

  for (int row = 0; row < modelProdutos.rowCount(); ++row) {
    ui->tableProdutos->openPersistentEditor(modelProdutos.index(row, modelProdutos.fieldIndex("selecionado")));
  }

  ui->tableProdutos->resizeColumnsToContents();

  return QString();
}

void WidgetCompraGerar::on_pushButtonGerarCompra_clicked() {
  // NOTE: refactor this function into smaller functions

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  QList<int> lista;
  QStringList ids;

  for (const auto index :
       modelProdutos.match(modelProdutos.index(0, modelProdutos.fieldIndex("selecionado")), Qt::DisplayRole, true, -1,
                           Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap))) {
    lista.append(index.row());
    ids.append(modelProdutos.data(index.row(), "idPedido").toString());

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

  QString filtro = modelProdutos.filter();

  InputDialog *inputDlg = new InputDialog(InputDialog::GerarCompra, this);
  inputDlg->setFilter(ids);

  if (inputDlg->exec() != InputDialog::Accepted) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  const QDate dataCompra = inputDlg->getDate();
  const QDate dataPrevista = inputDlg->getNextDate();

  modelProdutos.setFilter(filtro);
  modelProdutos.select();

  QStringList produtos;

  for (const auto row : lista) {
    QString produto = modelProdutos.data(row, "descricao").toString() + ", Quant: " +
                      modelProdutos.data(row, "quant").toString() + ", R$ " +
                      modelProdutos.data(row, "preco").toString().replace(".", ",");
    produtos.append(produto);
  }

  //------------------------------
  QString idVenda = gerarExcel(lista);

  if (idVenda.isEmpty()) {
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  //------------------------------
  QString path = UserSession::settings("User/userFolder").toString();

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

    if (mail->exec() != SendMail::Accepted) return;
  }

  modelProdutos.setFilter(filtro);
  modelProdutos.select();

  for (const auto row : lista) {
    if (not modelProdutos.setData(row, "status", "EM COMPRA")) {
      QMessageBox::critical(this, "Erro!", "Erro marcando status EM COMPRA: " + modelProdutos.lastError().text());
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

    if (not modelProdutos.setData(row, "idCompra", id)) {
      QMessageBox::critical(this, "Erro!", "Erro guardando idCompra: " + modelProdutos.lastError().text());
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
    query.bindValue(":idProduto", modelProdutos.data(row, "idProduto"));

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

    if (not modelProdutos.setData(row, "dataRealCompra", dataCompra.toString("yyyy-MM-dd"))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando data da compra: " + modelProdutos.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }

    if (not modelProdutos.setData(row, "dataPrevConf", dataPrevista.toString("yyyy-MM-dd"))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando data prevista: " + modelProdutos.lastError().text());
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
    query.bindValue(":idLoja", UserSession::loja());
    query.bindValue(":valor", modelProdutos.data(row, "preco"));
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

  if (not modelProdutos.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados da tabela pedido_fornecedor_has_produto: " +
                          modelProdutos.lastError().text());
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

  if (settings("User/userFolder").toString().isEmpty()) return QString();

  QString path = settings("User/userFolder").toString();

  QDir dir(path);

  if (not dir.exists()) dir.mkdir(path);

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
  queryVenda.bindValue(":idProduto", modelProdutos.data(lista.first(), "idProduto"));

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
    xlsx.write("B" + QString::number(13 + i), modelProdutos.data(row, "codComercial"));
    xlsx.write("C" + QString::number(13 + i), modelProdutos.data(row, "descricao"));
    xlsx.write("E" + QString::number(13 + i),
               (modelProdutos.data(row, "preco").toDouble() / modelProdutos.data(row, "quant").toDouble()));
    xlsx.write("F" + QString::number(13 + i), modelProdutos.data(row, "un"));
    xlsx.write("G" + QString::number(13 + i), modelProdutos.data(row, "quant"));

    ++i;
  }

  if (not xlsx.saveAs(path + "/" + idVenda + ".xlsx")) {
    QMessageBox::critical(this, "Erro!", "Ocorreu algum erro ao salvar o arquivo.");
    return QString();
  }

  QMessageBox::information(this, "Ok!", "Arquivo salvo como " + path + "/" + idVenda + ".xlsx");
  return idVenda;
}

// TODO: remove these
QVariant WidgetCompraGerar::settings(const QString &key) const { return UserSession::settings(key); }

void WidgetCompraGerar::setSettings(const QString &key, const QVariant &value) const {
  UserSession::setSettings(key, value);
}

void WidgetCompraGerar::on_checkBoxTodosGerar_clicked(const bool &checked) {
  for (int row = 0; row < modelProdutos.rowCount(); ++row) {
    modelProdutos.setData(row, "selecionado", checked);
  }
}

void WidgetCompraGerar::on_tableForn_activated(const QModelIndex &index) {
  const QString fornecedor = modelForn.data(index.row(), "fornecedor").toString();

  modelProdutos.setFilter("fornecedor = '" + fornecedor + "' AND status = 'PENDENTE'");

  if (not modelProdutos.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + modelProdutos.lastError().text());
    return;
  }

  for (int row = 0; row < modelProdutos.rowCount(); ++row) {
    ui->tableProdutos->openPersistentEditor(modelProdutos.index(row, modelProdutos.fieldIndex("selecionado")));
  }

  ui->checkBoxTodosGerar->setChecked(false);

  ui->tableProdutos->resizeColumnsToContents();
}

void WidgetCompraGerar::on_tableProdutos_entered(const QModelIndex &) {
  for (int row = 0; row < modelProdutos.rowCount(); ++row) {
    ui->tableProdutos->openPersistentEditor(modelProdutos.index(row, modelProdutos.fieldIndex("selecionado")));
  }

  ui->tableProdutos->resizeColumnsToContents();
}

// NOTE: arrumar openPersistent quando ordenar colunas (tableEntered é temp)
// TODO: alterar logica para nao resetar tabela fornecedores se existir linha selecionada
