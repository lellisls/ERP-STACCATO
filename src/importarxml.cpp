#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "importarxml.h"
#include "ui_importarxml.h"
#include "xml.h"
#include "xml_viewer.h"
#include "checkboxdelegate.h"
#include "estoqueproxymodel.h"

ImportarXML::ImportarXML(QList<int> rows, QWidget *parent) : QDialog(parent), ui(new Ui::ImportarXML) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  modelEstoque.setTable("estoque");
  modelEstoque.setEditStrategy(QSqlTableModel::OnManualSubmit);

  mapper.setModel(&modelEstoque);
  mapper.setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

  mapper.addMapping(ui->lineEdit_2, modelEstoque.fieldIndex("descricao"));
  mapper.addMapping(ui->lineEdit_3, modelEstoque.fieldIndex("codComercial"));

  ui->lineEdit_2->setSearchDialog(SearchDialog::produto(this));

  ui->tableEstoque->setModel(new EstoqueProxyModel(&modelEstoque, modelEstoque.fieldIndex("quantUpd"), this));
  ui->tableEstoque->horizontalHeader()->setResizeContentsPrecision(0);
  ui->tableEstoque->verticalHeader()->setResizeContentsPrecision(0);

  modelCompra.setTable("pedido_fornecedor_has_produto");
  modelCompra.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelCompra.setHeaderData(modelCompra.fieldIndex("selecionado"), Qt::Horizontal, "");

  if (not rows.isEmpty()) {
    QString ids;

    for (const int row : rows) {
      if (ids.isEmpty()) {
        ids = "idCompra = '" + QString::number(row) + "'";
      } else {
        ids += " OR idCompra = '" + QString::number(row) + "'";
      }
    }

    modelCompra.setFilter(ids);

    if (not modelCompra.select()) {
      QMessageBox::critical(this, "Erro!",
                            "Erro lendo tabela pedido_fornecedor_has_produto: " + modelCompra.lastError().text());
    }
  }

  ui->tableCompra->setModel(&modelCompra);
  ui->tableCompra->setItemDelegateForColumn(modelCompra.fieldIndex("selecionado"), new CheckBoxDelegate(this));
  ui->tableCompra->horizontalHeader()->setResizeContentsPrecision(0);
  ui->tableCompra->verticalHeader()->setResizeContentsPrecision(0);

  ui->tableCompra->resizeColumnsToContents();
}

ImportarXML::~ImportarXML() { delete ui; }

void ImportarXML::show() {
  for (int row = 0; row < modelCompra.rowCount(); ++row) {
    ui->tableCompra->openPersistentEditor(modelCompra.index(row, modelCompra.fieldIndex("selecionado")));
  }

  QDialog::show();
}

void ImportarXML::on_pushButtonCancelar_clicked() {
  QSqlQuery("ROLLBACK").exec();

  QDialog::reject();
  close();
}

void ImportarXML::on_pushButtonImportar_clicked() {
  bool ok = true;

  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    int status = modelEstoque.data(row, "quantUpd").toInt();

    if (status == 2 or status == 3) {
      ok = false;
    }
  }

  if (not ok) {
    if (QMessageBox::question(this, "Continuar?", "Nem todos os produtos estão ok. Deseja continuar?") !=
        QMessageBox::Yes) {
      return;
    }
  }

  QSqlQuery("COMMIT").exec();

  QDialog::accept();
  close();
}

void ImportarXML::on_pushButtonProcurar_clicked() {
  const QString file = QFileDialog::getOpenFileName(this, "Arquivo XML", QDir::currentPath(), "XML (*.xml)");

  if (file.isEmpty()) {
    ui->lineEdit->setText(QString());
    xml = nullptr;
    return;
  }

  ui->lineEdit->setText(file);

  QSqlQuery("SET SESSION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  xml = new XML();
  xml->mostrarNoModel(file, modelEstoque);

  // TODO: retornar da funçao se cadastrarNFe emitir erro (nota ja cadastrada)
  int idNFe = xml->cadastrarNFe();
  int idCompra = 0;

  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    QString codComercial = modelEstoque.data(row, "codComercial").toString();
    double quant = modelEstoque.data(row, "quant").toDouble();

    QModelIndexList list =
        modelCompra.match(modelCompra.index(0, modelCompra.fieldIndex("codComercial")), Qt::DisplayRole, codComercial,
                          -1, Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap));

    if (list.size() > 0) {
      qDebug() << "Encontrou produto: " << codComercial;

      // TODO: improve this
      idCompra = modelCompra.data(list.first().row(), "idCompra").toInt();

      if (not modelEstoque.setData(row, "idCompra", idCompra)) {
        QMessageBox::critical(this, "Erro!", "Erro guardando idCompra: " + modelEstoque.lastError().text());
        QSqlQuery("ROLLBACK").exec();
        return;
      }

      if (not modelEstoque.setData(row, "idNFe", idNFe)) {
        QMessageBox::critical(this, "Erro!", "Erro guardando idNFe: " + modelEstoque.lastError().text());
        QSqlQuery("ROLLBACK").exec();
        return;
      }

      double quant2 = 0;

      for (auto index : list) {
        quant2 += modelCompra.data(index.row(), "quant").toDouble();
      }

      // TODO: create enum to use in place of 1 and 2
      if (not modelEstoque.setData(row, "quantUpd", (quant == quant2) ? 1 : 2)) {
        QMessageBox::critical(this, "Erro!", "Erro guardando quantUpd: " + modelEstoque.lastError().text());
        QSqlQuery("ROLLBACK").exec();
        return;
      }
    } else {
      qDebug() << "Não encontrou produto: " << codComercial;

      if (not modelEstoque.setData(row, "quantUpd", 3)) {
        QMessageBox::critical(this, "Erro!", "Erro guardando quantUpd 3: " + modelEstoque.lastError().text());
        QSqlQuery("ROLLBACK").exec();
        return;
      }
    }
  }

  if (not modelEstoque.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando tabela estoque: " + modelEstoque.lastError().text());
    QSqlQuery("ROLLBACK").exec();
    return;
  }

  for (int row = 0, rowCount = modelEstoque.rowCount(); row < rowCount; ++row) {
    // TODO: buscar na venda_has_produto todos os produtos com idCompra/codComercial igual do estoque
    // TODO: set id do venda_has_produto e -quant
    QString codComercial = modelEstoque.data(row, "codComercial").toString();

    QSqlQuery query;
    query.prepare("SELECT * FROM venda_has_produto WHERE codComercial = :codComercial AND idCompra = :idCompra");
    query.bindValue(":codComercial", codComercial);
    query.bindValue(":idCompra", idCompra);

    if (not query.exec() or not query.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando em venda_has_produto: " + modelEstoque.lastError().text());
      QSqlQuery("ROLLBACK").exec();
      return;
    }

    for (int i = 0; i < query.size(); ++i) {
      int newRow = modelEstoque.rowCount();

      if (not modelEstoque.insertRow(newRow)) {
        QMessageBox::critical(this, "Erro!", "Erro criando nova linha na tabela: " + modelEstoque.lastError().text());
        QSqlQuery("ROLLBACK").exec();
        return;
      }

      for (int column = 0; column < modelEstoque.columnCount(); ++column) {
        if (not modelEstoque.setData(newRow, column, modelEstoque.data(row, column))) {
          QMessageBox::critical(this, "Erro!", "Erro guardando " + QString::number(column) + ": " + modelEstoque.lastError().text());
          QSqlQuery("ROLLBACK").exec();
          return;
        }
      }

      double quant = query.value("quant").toDouble() * -1;

      if (not modelEstoque.setData(newRow, "quant", quant)) {
        QMessageBox::critical(this, "Erro!", "Erro guardando quant: " + modelEstoque.lastError().text());
        QSqlQuery("ROLLBACK").exec();
        return;
      }

      if (not modelEstoque.setData(newRow, "quantUpd", 0)) {
        QMessageBox::critical(this, "Erro!", "Erro guardando quantUpd: " + modelEstoque.lastError().text());
        QSqlQuery("ROLLBACK").exec();
        return;
      }

      if (not modelEstoque.setData(newRow, "idVendaProduto", query.value("idVendaProduto"))) {
        QMessageBox::critical(this, "Erro!", "Erro guardando idVendaProduto: " + modelEstoque.lastError().text());
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

  ui->tableEstoque->resizeColumnsToContents();
}

void ImportarXML::on_tableEstoque_clicked(const QModelIndex &index) { mapper.setCurrentModelIndex(index); }
