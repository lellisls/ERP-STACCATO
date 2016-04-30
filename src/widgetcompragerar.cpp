#include <QDate>
#include <QDebug>
#include <QDesktopServices>
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

  connect(ui->tableProdutos->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this,
          &WidgetCompraGerar::fixPersistente);
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
  modelProdutos.setHeaderData("dataPrevCompra", "Prev. Compra");
  modelProdutos.setHeaderData("dataCompra", "Data Compra");
  modelProdutos.setHeaderData("status", "Status");

  modelProdutos.setFilter("status = 'PENDENTE'");

  ui->tableProdutos->setModel(&modelProdutos);
  ui->tableProdutos->setItemDelegateForColumn("selecionado", new CheckBoxDelegate(this));
  ui->tableProdutos->hideColumn("idCompra");
  ui->tableProdutos->hideColumn("quantConsumida");
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

bool WidgetCompraGerar::updateTables(QString &error) {
  if (modelForn.tableName().isEmpty()) setupTables();

  auto selection = ui->tableForn->selectionModel()->selectedRows();

  auto index = selection.size() > 0 ? selection.first() : QModelIndex();

  if (not modelForn.select()) {
    error = "Erro lendo tabela fornecedores: " + modelForn.lastError().text();
    return false;
  }

  ui->tableForn->resizeColumnsToContents();

  modelProdutos.setFilter("0");

  if (not modelProdutos.select()) {
    error = "Erro lendo tabela pedido_fornecedor_has_produto: " + modelProdutos.lastError().text();
    return false;
  }

  if (selection.size() > 0) {
    on_tableForn_activated(index);
    ui->tableForn->selectRow(index.row());
  }

  return true;
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

  QString anexo;

  if (not gerarExcel(lista, anexo)) return;

  QString textoPadrao = "Texto Padrao Lorem Ipsum";

  QMessageBox msgBox(QMessageBox::Question, "Enviar E-mail?", "Deseja enviar e-mail?",
                     QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Enviar");
  msgBox.setButtonText(QMessageBox::No, "Pular");

  if (msgBox.exec() == QMessageBox::Yes) {
    SendMail *mail = new SendMail(this, textoPadrao, anexo);

    if (mail->exec() != SendMail::Accepted) {
      QSqlQuery("ROLLBACK").exec();
      return;
    }
  }

  modelProdutos.setFilter(filtro);
  modelProdutos.select();

  QString id;

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

    id = queryId.first() ? QString::number(queryId.value("idCompra").toInt() + 1) : "1";

    if (not modelProdutos.setData(row, "idCompra", id)) {
      QMessageBox::critical(this, "Erro!", "Erro guardando idCompra: " + modelProdutos.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }

    // salvar status na venda
    QSqlQuery query;
    query.prepare("UPDATE venda_has_produto SET idCompra = :idCompra, dataRealCompra = :dataRealCompra, dataPrevConf = "
                  ":dataPrevConf, status = 'EM COMPRA' WHERE idProduto = :idProduto AND idVenda = :idVenda");
    query.bindValue(":idCompra", id);
    query.bindValue(":dataRealCompra", dataCompra);
    query.bindValue(":dataPrevConf", dataPrevista);
    query.bindValue(":idProduto", modelProdutos.data(row, "idProduto"));
    query.bindValue(":idVenda", modelProdutos.data(row, "idVenda"));

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

    if (not modelProdutos.setData(row, "dataRealCompra", dataCompra)) {
      QMessageBox::critical(this, "Erro!", "Erro guardando data da compra: " + modelProdutos.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }

    if (not modelProdutos.setData(row, "dataPrevConf", dataPrevista)) {
      QMessageBox::critical(this, "Erro!", "Erro guardando data prevista: " + modelProdutos.lastError().text());
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

  QString error;

  if (not updateTables(error)) QMessageBox::critical(this, "Erro!", error);
}

bool WidgetCompraGerar::gerarExcel(QList<int> lista, QString &anexo) {
  QString fornecedor = modelProdutos.data(0, "fornecedor").toString();

  QString arquivoModelo = "OUTROS.xlsx";

  if (fornecedor == "INTERFLOOR") {
    QMessageBox::critical(this, "Erro!", "Interfloor ainda não disponível, gere o arquivo manualmente.");
    return false;
  }

  //  arquivoModelo = fornecedor == "PORTINARI" ? "PORTINARI.xlsx" : "OUTROS.xlsx";

  QFile modelo(QDir::currentPath() + "/" + arquivoModelo);

  if (not modelo.exists()) {
    QMessageBox::critical(this, "Erro!", "Não encontrou o modelo do Excel!");
    return false;
  }

  // set querys?

  QString fileName = QDir::currentPath() + "/arquivo.xlsx";

  QFile file(fileName);

  if (not file.open(QFile::WriteOnly)) {
    QMessageBox::critical(this, "Erro!", "Arquivo bloqueado! Por favor feche o arquivo.");
    return false;
  }

  file.close();

  QLocale locale;

  QXlsx::Document xlsx(arquivoModelo);

  //  QList<int> lista;

  //  for (const auto index :
  //       modelProdutos.match(modelProdutos.index(0, modelProdutos.fieldIndex("selecionado")), Qt::DisplayRole, true,
  //       -1,
  //                           Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap))) {
  //    lista.append(index.row());
  //  }

  //  if(lista.size() == 0){
  //      QMessageBox::critical(this, "Erro!", "Lista vazia!");
  //      return QString();
  //  }

  //  if (fornecedor == "PORTINARI") {
  //    // xlsx write...
  //    xlsx.write("E5", QDate::currentDate().toString("dd-MM-yyyy")); // dd//mm/yyyy
  //    xlsx.write("F5", "cód. Ped. xyz"); // cod pedido

  //    // for(0..produtos.size
  //    for (int row = 0; row < lista.size(); ++row) {
  //      xlsx.write("A" + QString::number(7 + row), QString::number(row + 1)); // item 1,2,3...
  ////      xlsx.write("B" + QString::number(7 + row), modelProdutos.data(row, "ui")); // fabrica
  //      xlsx.write("C" + QString::number(7 + row), modelProdutos.data(row, "codComercial")); // cod produto
  //      xlsx.write("D" + QString::number(7 + row), modelProdutos.data(row, "descricao")); // descricao produto
  //      xlsx.write("E" + QString::number(7 + row), modelProdutos.data(row, "formComercial")); // formato
  //      xlsx.write("F" + QString::number(7 + row), ""); // ret/bold
  //      xlsx.write("G" + QString::number(7 + row), modelProdutos.data(row, "quant")); // quant.
  //      xlsx.write("H" + QString::number(7 + row), modelProdutos.data(row, "prcUnitario")); // prec. unitario
  //      xlsx.write("I" + QString::number(7 + row), ""); // promocao
  //    }
  //  } else {
  // xlsx write...
  xlsx.write("F4", "");                                            // numero pedido
  xlsx.write("E6", modelProdutos.data(lista.at(0), "fornecedor")); // fornecedor
  xlsx.write("E8", "");                                            // representante
  xlsx.write("D10", QDate::currentDate().toString("dd-MM-yyyy"));  // Data: dd//mm/yyyy

  //  for(0..produtos.size)
  for (int row = 0; row < lista.size(); ++row) {
    xlsx.write("A" + QString::number(13 + row), QString::number(row + 1));                // item n. 1,2,3...
    xlsx.write("B" + QString::number(13 + row), modelProdutos.data(row, "codComercial")); // cod produto
    xlsx.write("C" + QString::number(13 + row), modelProdutos.data(row, "descricao"));    // descricao produto
    xlsx.write("E" + QString::number(13 + row), modelProdutos.data(row, "prcUnitario"));  // prc. unitario
    xlsx.write("F" + QString::number(13 + row), modelProdutos.data(row, "un"));           // un.
    xlsx.write("G" + QString::number(13 + row), modelProdutos.data(row, "quant"));        // qtd.
    xlsx.write("H" + QString::number(13 + row), modelProdutos.data(row, "preco"));        // valor
  }

  //    xlsx.write("H36", ""); // total
  //  }

  if (not xlsx.saveAs(fileName)) {
    QMessageBox::critical(this, "Erro!", "Ocorreu algum erro ao salvar o arquivo.");
    return false;
  }

  QMessageBox::information(this, "Ok!", "Arquivo salvo como " + fileName);
  QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));

  anexo = fileName;

  return true;
}

void WidgetCompraGerar::on_checkBoxMarcarTodos_clicked(const bool &checked) {
  for (int row = 0, rowCount = modelProdutos.rowCount(); row < rowCount; ++row) {
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

  fixPersistente();

  ui->tableProdutos->resizeColumnsToContents();
}

void WidgetCompraGerar::fixPersistente() {
  for (int row = 0, rowCount = ui->tableProdutos->model()->rowCount(); row < rowCount; ++row) {
    ui->tableProdutos->openPersistentEditor(row, "selecionado");
  }

  ui->checkBoxMarcarTodos->setChecked(false);
}

void WidgetCompraGerar::on_tableProdutos_entered(const QModelIndex &) { ui->tableProdutos->resizeColumnsToContents(); }

void WidgetCompraGerar::on_pushButtonTeste_clicked() {
  QList<int> lista;
  QStringList ids;

  for (const auto index :
       modelProdutos.match(modelProdutos.index(0, modelProdutos.fieldIndex("selecionado")), Qt::DisplayRole, true, -1,
                           Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap))) {
    lista.append(index.row());
    ids.append(modelProdutos.data(index.row(), "idPedido").toString());
  }

  if (lista.size() == 0) {
    QMessageBox::critical(this, "Aviso!", "Nenhum item selecionado!");
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  QString anexo;

  if (not gerarExcel(lista, anexo)) return;
}
