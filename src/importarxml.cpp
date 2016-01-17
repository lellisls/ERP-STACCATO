#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "checkboxdelegate.h"
#include "estoqueproxymodel.h"
#include "importarxml.h"
#include "ui_importarxml.h"
#include "xml.h"
#include "xml_viewer.h"

ImportarXML::ImportarXML(const QString &fornecedor, QWidget *parent) : QDialog(parent), ui(new Ui::ImportarXML) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables(fornecedor);

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();
}

ImportarXML::~ImportarXML() { delete ui; }

void ImportarXML::setupTables(const QString &fornecedor) {
  modelEstoque.setTable("estoque");
  modelEstoque.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelEstoque.setFilter("temp = TRUE");

  if (not modelEstoque.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + modelEstoque.lastError().text());
  }

  mapper.setModel(&modelEstoque);
  mapper.setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

  ui->tableEstoque->setModel(new EstoqueProxyModel(&modelEstoque, "quantUpd", this));

  modelCompra.setTable("pedido_fornecedor_has_produto");
  modelCompra.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelCompra.setFilter("fornecedor = '" + fornecedor + "' AND status = 'EM FATURAMENTO'");

  if (not modelCompra.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + modelCompra.lastError().text());
  }

  ui->tableCompra->setModel(&modelCompra);
  ui->tableCompra->hideColumn("selecionado");

  ui->tableCompra->resizeColumnsToContents();
}

QString ImportarXML::getIdCompra() { return idCompra; }


void ImportarXML::on_pushButtonImportar_clicked() {
  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    modelEstoque.setData(row, "temp", 0);
  }

  if (not modelEstoque.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados da tabela estoque: " + modelEstoque.lastError().text());
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  //  bool ok = true;

  //  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
  //    int status = modelEstoque.data(row, "quantUpd").toInt();

  //    if (status == Yellow or status == Red) {
  //      ok = false;
  //    }
  //  }

  //  if (not ok) {
  //    QMessageBox::critical(this, "Erro!", "Nem todos os produtos estão ok!");
  //    return;
  //    QMessageBox msgBox(QMessageBox::Question, "Continuar?", "Nem todos os produtos estão ok. Deseja continuar?",
  //                       QMessageBox::Yes | QMessageBox::No, this);
  //    msgBox.setButtonText(QMessageBox::Yes, "Sim");
  //    msgBox.setButtonText(QMessageBox::No, "Não");

  //    if (msgBox.exec() == QMessageBox::No) {
  //      return;
  //    }
  //  }

  QSqlQuery("COMMIT").exec();

  QDialog::accept();
  close();
}

void ImportarXML::on_pushButtonProcurar_clicked() {
  const QString filePath = QFileDialog::getOpenFileName(this, "Arquivo XML", QDir::currentPath(), "XML (*.xml)");

  if (filePath.isEmpty()) {
    ui->lineEdit->setText(QString());
    return;
  }

  ui->lineEdit->setText(filePath);

  QFile file(filePath);

  if (not file.open(QFile::ReadOnly)) {
    QMessageBox::critical(this, "Erro!", "Erro lendo arquivo: " + file.errorString());
    return;
  }

  XML xml(file.readAll(), file.fileName());
  xml.mostrarNoSqlModel(modelEstoque);

  int idNFe = xml.cadastrarNFe("ENTRADA");

  if (idNFe == 0) return;

  for (int row = 0, rowCount = modelEstoque.rowCount(); row < rowCount; ++row) {
    const double quant = modelEstoque.data(row, "quant").toDouble();

    const QString codComercial = modelEstoque.data(row, "codComercial").toString();
    const QString codBarras = modelEstoque.data(row, "codBarras").toString();

    QSqlQuery query;
    query.prepare(
          "SELECT * FROM pedido_fornecedor_has_produto WHERE codComercial = :codComercial OR codBarras = :codBarras");
    query.bindValue(":codComercial", codComercial);
    query.bindValue(":codBarras", codBarras);

    if (not query.exec() or not query.first()) {
      // TODO: só mostrar mensagem para os produtos importados na nota atual
      QMessageBox::critical(this, "Erro!", "Não encontrou produto: " + modelEstoque.data(row, "descricao").toString());

      if (not modelEstoque.setData(row, "quantUpd", Red)) QSqlQuery("ROLLBACK").exec();

      return;
    }

    //    int idCompra = query.value("idCompra").toInt();
    QString idCompra = query.value("idCompra").toString();

    double quant2 = query.value("quant").toDouble();

    while (query.next()) {
      quant2 += query.value("quant").toDouble();
      idCompra += "," + query.value("idCompra").toString();
    }

    if (not modelEstoque.setData(row, "idCompra", idCompra)) {
      QSqlQuery("ROLLBACK").exec();
      return;
    }

    if (not modelEstoque.setData(row, "idNFe", idNFe)) {
      QSqlQuery("ROLLBACK").exec();
      return;
    }

    if (not modelEstoque.setData(row, "quantUpd", quant == quant2 ? Green : Yellow)) {
      QSqlQuery("ROLLBACK").exec();
      return;
    }

    this->idCompra = idCompra.replace(",", " OR idCompra = ");

    // ------------------------------------------------------------------
    // TODO: pré-consumo de estoque deve ser feito mais para a frente

    query.prepare("SELECT * FROM venda_has_produto AS v LEFT JOIN produto AS p ON v.idProduto = p.idProduto WHERE "
                  "(p.codComercial = :codComercial OR p.codBarras = :codBarras) AND idCompra = :idCompra");
    query.bindValue(":codComercial", codComercial);
    query.bindValue(":codBarras", codBarras);
    query.bindValue(":idCompra", idCompra);

    if (not query.exec() or not query.first()) {
      qDebug() << "erro: " << modelEstoque.lastError();
      qDebug() << "query: " << modelEstoque.query().lastQuery();
      qDebug() << "cod: " << codComercial;
      qDebug() << "id: " << idCompra;
      qDebug() << "barras: " << codBarras;
      QMessageBox::critical(this, "Erro!", "Erro buscando em venda_has_produto: " + modelEstoque.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }

    // parear produto a produto (esta pareando um unico consumo com tudo)

    for (int i = 0; i < query.size(); ++i) {
      int newRow = modelEstoque.rowCount();

      if (not modelEstoque.insertRow(newRow)) {
        QMessageBox::critical(this, "Erro!", "Erro criando nova linha na tabela: " + modelEstoque.lastError().text());
        QSqlQuery("ROLLBACK").exec();
        return;
      }

      for (int column = 0; column < modelEstoque.columnCount(); ++column) {
        if (not modelEstoque.setData(newRow, column, modelEstoque.data(row, column))) {
          QSqlQuery("ROLLBACK").exec();
          return;
        }
      }

      modelEstoque.setData(newRow, "status", "PRÉ-CONSUMO");

      double quant = query.value("quant").toDouble() * -1;

      if (not modelEstoque.setData(newRow, "quant", quant)) {
        QSqlQuery("ROLLBACK").exec();
        return;
      }

      if (not modelEstoque.setData(newRow, "quantUpd", false)) {
        QSqlQuery("ROLLBACK").exec();
        return;
      }

      if (not modelEstoque.setData(newRow, "idVendaProduto", query.value("idVendaProduto"))) {
        QSqlQuery("ROLLBACK").exec();
        return;
      }
    }
  }

  if (not modelEstoque.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados da tabela estoque: " + modelEstoque.lastError().text());
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    if (modelEstoque.data(row, "idVendaProduto").toInt() != 0) {
      QModelIndexList list = modelEstoque.match(modelEstoque.index(0, modelEstoque.fieldIndex("idProduto")),
                                                Qt::DisplayRole, modelEstoque.data(row, "idProduto"));

      int idEstoque = modelEstoque.data(list.first().row(), "idEstoque").toInt();

      modelEstoque.setData(row, "idEstoqueConsumido", idEstoque);
    }
  }

  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    int idProduto = modelEstoque.data(row, "idProduto").toInt();
    int idEstoque = modelEstoque.data(row, "idEstoque").toInt();

    QModelIndexList list =
        modelCompra.match(modelCompra.index(0, modelCompra.fieldIndex("idProduto")), Qt::DisplayRole, idProduto);

    if (list.size() == 0) break;

    if (modelCompra.data(list.first().row(), "idEstoque").isNull()) {
      modelCompra.setData(list.first().row(), "idEstoque", idEstoque);
      modelCompra.setData(list.first().row(), "idNFe", idNFe);
    }
  }

  if (not modelEstoque.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados da tabela estoque: " + modelEstoque.lastError().text());
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  ui->tableEstoque->resizeColumnsToContents();
}

void ImportarXML::on_tableEstoque_clicked(const QModelIndex &index) { mapper.setCurrentModelIndex(index); }

void ImportarXML::closeEvent(QCloseEvent *event) {
  QSqlQuery("ROLLBACK").exec();

  QDialog::closeEvent(event);
}

// TODO: parear tabela de baixo com a de cima (no lugar de toda a tabela de produtos)
// TODO: filtrar por fornecedor, se não achar nenhum remover o filtro
// TODO: parear por drag&drop
// TODO: relacionar n-n (multiplas compras x multiplas notas)
// TODO: colocar uma coluna na tabela de baixo para dizer qual a linha correspondente na tabela de cima e ir pareando,
// somando as quantidades
// TODO: perguntar local de estoque (cadastro de loja)
// TODO: legenda de cores
// TODO: relacionar consumo direto com estoque, relacionar produto com consumo
