#include <QDate>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>

#include "widgetcompra.h"
#include "checkboxdelegate.h"
#include "importarxml.h"
#include "inputdialog.h"
#include "produtospendentes.h"
#include "sendmail.h"
#include "ui_widgetcompra.h"
#include "usersession.h"
#include "xlsxdocument.h"

WidgetCompra::WidgetCompra(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompra) {
  ui->setupUi(this);

  setupTables();

  ui->splitter_5->setStretchFactor(0, 0);
  ui->splitter_5->setStretchFactor(1, 1);

  if (UserSession::getTipoUsuario() == "VENDEDOR") {
    ui->labelPedidosCompra->hide();
  }

  ui->radioButtonProdPendPend->click();
}

WidgetCompra::~WidgetCompra() { delete ui; }

void WidgetCompra::setupTables() {
  modelProdPend = new SqlTableModel(this);
  modelProdPend->setTable("view_produtos_pendentes");

  modelProdPend->setHeaderData("Form", "Form.");
  modelProdPend->setHeaderData("Quant", "Quant.");
  modelProdPend->setHeaderData("Un", "Un.");
  modelProdPend->setHeaderData("Cód Com", "Cód. Com.");

  ui->tableProdutosPend->setModel(modelProdPend);

  modelPedForn = new SqlTableModel(this);
  modelPedForn->setTable("view_fornecedor_compra");

  modelPedForn->setHeaderData("fornecedor", "Fornecedor");
  modelPedForn->setHeaderData("COUNT(fornecedor)", "Itens");

  ui->tableFornCompras->setModel(modelPedForn);

  modelItemPedidosPend = new SqlTableModel(this);
  modelItemPedidosPend->setTable("pedido_fornecedor_has_produto");
  modelItemPedidosPend->setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelItemPedidosPend->setHeaderData("selecionado", "");
  modelItemPedidosPend->setHeaderData("fornecedor", "Fornecedor");
  modelItemPedidosPend->setHeaderData("descricao", "Descrição");
  modelItemPedidosPend->setHeaderData("colecao", "Coleção");
  modelItemPedidosPend->setHeaderData("quant", "Quant.");
  modelItemPedidosPend->setHeaderData("un", "Un.");
  modelItemPedidosPend->setHeaderData("preco", "Preço");
  modelItemPedidosPend->setHeaderData("formComercial", "Form. Com.");
  modelItemPedidosPend->setHeaderData("codComercial", "Cód. Com.");
  modelItemPedidosPend->setHeaderData("codBarras", "Cód. Bar.");
  modelItemPedidosPend->setHeaderData("idCompra", "Compra");
  modelItemPedidosPend->setHeaderData("dataPrevCompra", "Prev. Compra");
  modelItemPedidosPend->setHeaderData("dataCompra", "Data Compra");
  modelItemPedidosPend->setHeaderData("status", "Status");

  modelItemPedidosPend->setFilter("status = 'PENDENTE'");

  ui->tablePedidosPend->setModel(modelItemPedidosPend);
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

  // Compras - A Confirmar ---------------------------------------------------------------------------------------------
  modelItemPedidosComp = new SqlTableModel(this);
  modelItemPedidosComp->setTable("view_compras");

  modelItemPedidosComp->setHeaderData("fornecedor", "Fornecedor");
  modelItemPedidosComp->setHeaderData("idCompra", "Compra");
  modelItemPedidosComp->setHeaderData("COUNT(idProduto)", "Itens");
  modelItemPedidosComp->setHeaderData("SUM(preco)", "Preço");
  modelItemPedidosComp->setHeaderData("status", "Status");

  ui->tablePedidosComp->setModel(modelItemPedidosComp);

  // Faturamentos ------------------------------------------------------------------------------------------------------
  modelFat = new SqlTableModel(this);
  modelFat->setTable("view_faturamento");

  modelFat->setHeaderData("fornecedor", "Fornecedor");
  modelFat->setHeaderData("idCompra", "Compra");
  modelFat->setHeaderData("COUNT(idProduto)", "Itens");
  modelFat->setHeaderData("SUM(preco)", "Preço");
  modelFat->setHeaderData("status", "Status");

  ui->tableFaturamento->setModel(modelFat);
}

bool WidgetCompra::updateTables() {
  switch (ui->tabWidget->currentIndex()) {
    case 0: // Pendentes
      if (not modelProdPend->select()) {
        QMessageBox::critical(this, "Erro!",
                              "Erro lendo tabela produtos pendentes: " + modelProdPend->lastError().text());
        return false;
      }

      if (not modelPedForn->select()) {
        QMessageBox::critical(this, "Erro!",
                              "Erro lendo tabela fornecedores compra: " + modelPedForn->lastError().text());
        return false;
      }

      ui->tableProdutosPend->resizeColumnsToContents();
      ui->tableFornCompras->resizeColumnsToContents();

      break;
    case 1: // Ativas
      switch (ui->tabWidgetCompra->currentIndex()) {
        case 0: // Gerar Compra
          modelItemPedidosPend->setFilter("0");

          if (not modelItemPedidosPend->select()) {
            QMessageBox::critical(this, "Erro!", "Erro lendo tabela pedido_fornecedor_has_produto: " +
                                  modelItemPedidosPend->lastError().text());
            return false;
          }

          for (int i = 0; i < modelItemPedidosPend->rowCount(); ++i) {
            ui->tablePedidosPend->openPersistentEditor(
                  modelItemPedidosPend->index(i, modelItemPedidosPend->fieldIndex("selecionado")));
          }

          ui->tablePedidosPend->resizeColumnsToContents();
          break;

        case 1: // Confirmar Compra
          if (not modelItemPedidosComp->select()) {
            QMessageBox::critical(this, "Erro!", "Erro lendo tabela compras: " + modelItemPedidosComp->lastError().text());
            return false;
          }

          ui->tablePedidosComp->resizeColumnsToContents();
          break;

        case 2: // Faturamento
          if (not modelFat->select()) {
            QMessageBox::critical(this, "Erro!", "Erro lendo tabela faturamento: " + modelFat->lastError().text());
            return false;
          }

          ui->tableFaturamento->resizeColumnsToContents();
          break;

        default:
          return true;
      }

    default:
      return true;
  }

  return true;
}

void WidgetCompra::on_tableFornCompras_activated(const QModelIndex &index) {
  int row = index.row();

  QString fornecedor = modelPedForn->data(row, "fornecedor").toString();

  modelItemPedidosPend->setFilter("fornecedor = '" + fornecedor + "' AND status = 'PENDENTE'");

  if (not modelItemPedidosPend->select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela pedido_fornecedor_has_produto: " +
                          modelItemPedidosPend->lastError().text());
    return;
  }

  for (int i = 0; i < modelItemPedidosPend->rowCount(); ++i) {
    ui->tablePedidosPend->openPersistentEditor(
          modelItemPedidosPend->index(i, modelItemPedidosPend->fieldIndex("selecionado")));
  }

  ui->tablePedidosPend->resizeColumnsToContents();

  // TODO: filter compras e faturamentos?

  //  updateTables();

  //  ui->tableFornCompras->selectRow(row);
}

void WidgetCompra::on_pushButtonConfirmarCompra_clicked() {
  QString idCompra;

  if (ui->tablePedidosComp->selectionModel()->selectedRows().size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  int row = ui->tablePedidosComp->selectionModel()->selectedRows().first().row();
  idCompra = modelItemPedidosComp->data(row, "idCompra").toString();

  InputDialog *inputDlg = new InputDialog(InputDialog::ConfirmarCompra, this);
  inputDlg->setFilter(idCompra);
  QDate dataConf, dataPrevista;

  if (inputDlg->exec() != InputDialog::Accepted) {
    return;
  }

  dataConf = inputDlg->getDate();
  dataPrevista = inputDlg->getNextDate();

  QSqlQuery query;
  query.prepare("UPDATE pedido_fornecedor_has_produto SET dataRealConf = :dataRealConf, dataPrevFat = :dataPrevFat, "
                "status = 'EM FATURAMENTO' WHERE idCompra = :idCompra");
  query.bindValue(":dataRealConf", dataConf);
  query.bindValue(":dataPrevFat", dataPrevista);
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status da compra: " + query.lastError().text());
    return;
  }

  // salvar status na venda
  query.prepare("UPDATE venda_has_produto SET dataRealConf = :dataRealConf, dataPrevFat = :dataPrevFat, status = 'EM "
                "FATURAMENTO' WHERE idCompra = :idCompra");
  query.bindValue(":dataRealConf", dataConf);
  query.bindValue(":dataPrevFat", dataPrevista);
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando status da venda: " + query.lastError().text());
    return;
  }
  //

  updateTables();

  QMessageBox::information(this, "Aviso!", "Confirmado compra.");
}

void WidgetCompra::on_radioButtonProdPendTodos_clicked() {
  modelProdPend->setFilter("");

  ui->tableProdutosPend->resizeColumnsToContents();
}

void WidgetCompra::on_radioButtonProdPendPend_clicked() {
  modelProdPend->setFilter("status = 'PENDENTE'");

  ui->tableProdutosPend->resizeColumnsToContents();
}

void WidgetCompra::on_radioButtonProdPendEmCompra_clicked() {
  // TODO: mostrar caixas e un2
  modelProdPend->setFilter("status != 'PENDENTE'");

  ui->tableProdutosPend->resizeColumnsToContents();
}

void WidgetCompra::on_pushButtonMarcarFaturado_clicked() {
  QString idCompra;
  QList<int> rows;

  if (ui->tableFaturamento->selectionModel()->selectedRows().size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado.");
    return;
  }

  for (auto const &index : ui->tableFaturamento->selectionModel()->selectedRows()) {
    rows.append(modelFat->data(index.row(), "idCompra").toInt());
  }

  int row = ui->tableFaturamento->selectionModel()->selectedRows().first().row();
  idCompra = modelFat->data(row, "idCompra").toString();

  ImportarXML *import = new ImportarXML(rows, this);
  import->show();

  if (import->exec() != QDialog::Accepted) {
    return;
  }

  //----------------------------------------------------------//

  InputDialog *inputDlg = new InputDialog(InputDialog::ConfirmarCompra, this);
  inputDlg->setFilter(idCompra);

  if (inputDlg->exec() != InputDialog::Accepted) {
    return;
  }

  QDate dataFat = inputDlg->getDate();
  QDate dataPrevista = inputDlg->getNextDate();

  QSqlQuery query;
  query.prepare("UPDATE pedido_fornecedor_has_produto SET dataRealFat = :dataRealFat, dataPrevColeta = "
                ":dataPrevColeta, status = 'EM COLETA' WHERE idCompra = :idCompra");
  query.bindValue(":dataRealFat", dataFat);
  query.bindValue(":dataPrevColeta", dataPrevista);
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status da compra: " + query.lastError().text());
    return;
  }

  // salvar status na venda
  query.prepare("UPDATE venda_has_produto SET dataRealFat = :dataRealFat, dataPrevColeta = :dataPrevColeta, status = "
                "'EM COLETA' WHERE idCompra = :idCompra");
  query.bindValue(":dataRealFat", dataFat);
  query.bindValue(":dataPrevColeta", dataPrevista);
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando status da venda: " + query.lastError().text());
    return;
  }
  //

  updateTables();

  QMessageBox::information(this, "Aviso!", "Confirmado faturamento.");
}

void WidgetCompra::on_tableProdutosPend_activated(const QModelIndex &index) {
  ProdutosPendentes *produtos = new ProdutosPendentes(this);

  QString codComercial = modelProdPend->data(index.row(), "codComercial").toString();
  QString status = modelProdPend->data(index.row(), "status").toString();

  produtos->viewProduto(codComercial, status);
}

void WidgetCompra::on_pushButtonGerarCompra_clicked() {
  // TODO: refactor this function into smaller functions
  // TODO: colocar uma transaction
  QFile modelo(QDir::currentPath() + "/modelo.xlsx");

  if (not modelo.exists()) {
    QMessageBox::critical(this, "Erro!", "Não encontrou o modelo do Excel!");
    return;
  }

  //  if(modelItem.rowCount() > 17){
  //    QMessageBox::critical(this, "Erro!", "Mais itens do que cabe no modelo!");
  //    return;
  //  }

  if (settings("User/userFolder").toString().isEmpty()) {
    QMessageBox::warning(this, "Aviso!", "Não há uma pasta definida para salvar PDF/Excel. Por favor escolha uma.");
    setSettings("User/userFolder", QFileDialog::getExistingDirectory(this, "Pasta PDF/Excel"));
    return;
  }

  if (UserSession::getFromLoja("emailCompra").isEmpty()) {
    // TODO: set value here or open window to set
    QMessageBox::warning(this, "Aviso!",
                         "Não há um email de compras definido, favor cadastrar nas configurações da loja.");
    return;
  }

  if (modelPedForn->rowCount() > 1) {
    if (not modelItemPedidosPend->filter().contains("fornecedor")) {
      QMessageBox::critical(this, "Erro!", "Selecione o fornecedor na tabela à esquerda.");
      return;
    }
  }

  QList<int> lista;
  QStringList ids;
  QStringList produtos;

  for (const auto index :
       modelItemPedidosPend->match(modelItemPedidosPend->index(0, modelItemPedidosPend->fieldIndex("selecionado")),
                                   Qt::DisplayRole, true, -1, Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap))) {
    lista.append(index.row());
    ids.append(modelItemPedidosPend->data(index.row(), "idPedido").toString());
  }

  if (lista.size() == 0) {
    QMessageBox::warning(this, "Aviso!", "Nenhum item selecionado!");
    return;
  }

  InputDialog *inputDlg = new InputDialog(InputDialog::GerarCompra, this);
  inputDlg->setFilter(ids);
  QDate dataCompra, dataPrevista;

  QString filtro = modelItemPedidosPend->filter();

  if (inputDlg->exec() != InputDialog::Accepted) {
    return;
  }

  dataCompra = inputDlg->getDate();
  dataPrevista = inputDlg->getNextDate();

  modelItemPedidosPend->setFilter(filtro);
  modelItemPedidosPend->select();

  for (const auto row : lista) {
    QString produto = modelItemPedidosPend->data(row, "descricao").toString() + ", Quant: " +
                      modelItemPedidosPend->data(row, "quant").toString() + ", R$ " +
                      modelItemPedidosPend->data(row, "preco").toString().replace(".", ",");
    produtos.append(produto);
  }

  //------------------------------
  // TODO: refactor this from venda or orcamento
  QXlsx::Document xlsx("modelo.xlsx");

  QSqlQuery queryVenda;
  queryVenda.prepare("SELECT * FROM venda_has_produto WHERE idProduto = :idProduto");
  queryVenda.bindValue(":idProduto", modelItemPedidosPend->data(lista.first(), "idProduto"));

  if (not queryVenda.exec() or not queryVenda.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados da venda:" + queryVenda.lastError().text());
    return;
  }

  QString idVenda = queryVenda.value("idVenda").toString();

  QSqlQuery queryForn;
  queryForn.prepare("SELECT * FROM fornecedor WHERE razaoSocial = (SELECT fornecedor FROM venda_has_produto WHERE "
                    "idVenda = :idVenda LIMIT 1)");
  queryForn.bindValue(":idVenda", idVenda);

  if (not queryForn.exec() or not queryForn.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados do fornecedor: " + queryForn.lastError().text());
    return;
  }

  xlsx.write("F4", idVenda); // TODO: temp, replace with correct id
  xlsx.write("E6", queryForn.value("razaoSocial"));
  xlsx.write("E8", queryForn.value("contatoNome"));
  xlsx.write("D10", "Data: " + QDate::currentDate().toString("dd/MM/yy"));

  for (auto const &row : lista) {
    static int i = 0;

    xlsx.write("A" + QString::number(13 + i), i + 1);
    xlsx.write("B" + QString::number(13 + i), modelItemPedidosPend->data(row, "codComercial"));
    xlsx.write("C" + QString::number(13 + i), modelItemPedidosPend->data(row, "descricao"));
    xlsx.write("E" + QString::number(13 + i), (modelItemPedidosPend->data(row, "preco").toDouble() /
                                               modelItemPedidosPend->data(row, "quant").toDouble()));
    xlsx.write("F" + QString::number(13 + i), modelItemPedidosPend->data(row, "un"));
    xlsx.write("G" + QString::number(13 + i), modelItemPedidosPend->data(row, "quant"));

    ++i;
  }

  QString path = settings("User/userFolder").toString();

  QDir dir(path);

  if (not dir.exists()) {
    dir.mkdir(path);
  }

  if (not xlsx.saveAs(path + "/" + idVenda + ".xlsx")) {
    return;
  }

  QMessageBox::information(this, "Ok!", "Arquivo salvo como " + path + "/" + idVenda + ".xlsx");

  modelItemPedidosPend->setFilter(filtro);

  //------------------------------
  QString arquivo = path + "/" + idVenda + ".xlsx";

  QFile file(arquivo);

  if (not file.exists()) {
    QMessageBox::critical(this, "Erro!", "Arquivo não encontrado");
    return;
  }

  SendMail *mail = new SendMail(this, produtos.join("\n"), arquivo);

  if (mail->exec() != SendMail::Accepted) {
    return;
  }

  for (const auto row : lista) {
    modelItemPedidosPend->setData(row, "selecionado", false);

    // TODO: place this in the beginning
    if (modelItemPedidosPend->data(row, "status").toString() != "PENDENTE") {
      modelItemPedidosPend->select();
      QMessageBox::critical(this, "Erro!", "Produto não estava pendente!");
      return;
    }

    if (not modelItemPedidosPend->setData(row, "status", "EM COMPRA")) {
      QMessageBox::critical(this, "Erro!",
                            "Erro marcando status EM COMPRA: " + modelItemPedidosPend->lastError().text());
      return;
    }

    // TODO: gerar e guardar idCompra
    QSqlQuery queryId;

    if (not queryId.exec("SELECT idCompra FROM pedido_fornecedor_has_produto ORDER BY idCompra DESC")) {
      QMessageBox::critical(this, "Erro!", "Erro buscando idCompra: " + queryId.lastError().text());
      return;
    }

    QString id = "1";

    if (queryId.first()) {
      id = QString::number(queryId.value(0).toInt() + 1);
    }

    if (not modelItemPedidosPend->setData(row, "idCompra", id)) {
      QMessageBox::critical(this, "Erro!", "Erro guardando idCompra: " + modelItemPedidosPend->lastError().text());
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
    query.bindValue(":idProduto", modelItemPedidosPend->data(row, "idProduto"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status da venda: " + query.lastError().text());
      return;
    }
    //

    if (not modelItemPedidosPend->setData(row, "dataRealCompra", dataCompra.toString("yyyy-MM-dd"))) {
      QMessageBox::critical(this, "Erro!",
                            "Erro guardando data da compra: " + modelItemPedidosPend->lastError().text());
      return;
    }

    if (not modelItemPedidosPend->setData(row, "dataPrevConf", dataPrevista.toString("yyyy-MM-dd"))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando data prevista: " + modelItemPedidosPend->lastError().text());
      return;
    }
  }

  QSqlQuery query;
  query.prepare("INSERT INTO conta_a_pagar (idVenda, dataEmissao, pago) VALUES (:idVenda, :dataEmissao, :pago)");
  query.bindValue(":idVenda", idVenda);
  query.bindValue(":dataEmissao", QDate::currentDate().toString("yyyy-MM-dd"));
  query.bindValue(":pago", "NÃO");

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro guardando conta a pagar: " + query.lastError().text());
    return;
  }

  for (auto const &row : lista) {
    query.prepare("INSERT INTO conta_a_pagar_has_pagamento (idVenda, idLoja, tipo, parcela, valor, data, observacao, "
                  "status) VALUES (:idVenda, :idLoja, :tipo, :parcela, :valor, :data, :observacao, :status)");
    query.bindValue(":idVenda", idVenda);
    query.bindValue(":idLoja", UserSession::getLoja());
    query.bindValue(":tipo", "A CONFIRMAR");
    query.bindValue(":parcela", 1);
    qDebug() << "row: " << row;
    qDebug() << "valor: " << modelItemPedidosPend->data(row, "preco");
    query.bindValue(":valor", modelItemPedidosPend->data(row, "preco"));
    query.bindValue(":data", QDate::currentDate().toString("yyyy-MM-dd"));
    query.bindValue(":observacao", "");
    query.bindValue(":status", "PENDENTE");

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro guardando conta a pagar pagamento: " + query.lastError().text());
      return;
    }
  }

  if (not modelItemPedidosPend->submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados da tabela pedido_fornecedor_has_produto: " +
                          modelItemPedidosPend->lastError().text());
    return;
  }

  updateTables();
}

void WidgetCompra::on_pushButtonTodosFornCompras_clicked() {
  modelItemPedidosPend->setFilter("status = 'PENDENTE'");

  updateTables();
}

QVariant WidgetCompra::settings(QString key) const { return UserSession::getSettings(key); }

void WidgetCompra::setSettings(QString key, QVariant value) const { UserSession::setSettings(key, value); }

void WidgetCompra::on_tabWidgetCompra_currentChanged(int) { updateTables(); }

void WidgetCompra::on_pushButtonTesteEmail_clicked() {
  SendMail *mail = new SendMail(this);
  mail->show();
}
