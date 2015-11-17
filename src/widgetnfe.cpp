#include "widgetnfe.h"
#include "ui_widgetnfe.h"
#include "doubledelegate.h"
#include "usersession.h"
#include "venda.h"
#include "xml_viewer.h"

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>

WidgetNfe::WidgetNfe(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetNfe) {
  ui->setupUi(this);

  setupTables();

  if (UserSession::getTipoUsuario() == "VENDEDOR") {
    ui->tableNfeSaida->hide();
    ui->labelNfeSaida->hide();
  }
}

WidgetNfe::~WidgetNfe() { delete ui; }

void WidgetNfe::setupTables() {
  DoubleDelegate *doubledelegate = new DoubleDelegate(this);

  // NFe Entrada -------------------------------------------------------------------------------------------------------
  modelNfeEntrada = new SqlTableModel(this);
  modelNfeEntrada->setTable("nfe");
  modelNfeEntrada->setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelNfeEntrada->setFilter("tipo = 'ENTRADA'");

  ui->tableNfeEntrada->setModel(modelNfeEntrada);
  ui->tableNfeEntrada->hideColumn("NFe");
  ui->tableNfeEntrada->setItemDelegate(doubledelegate);

  // NFe Saida ---------------------------------------------------------------------------------------------------------
  modelNfeSaida = new SqlTableModel(this);
  modelNfeSaida->setTable("nfe");
  modelNfeSaida->setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelNfeSaida->setFilter("tipo = 'SAIDA'");

  ui->tableNfeSaida->setModel(modelNfeSaida);
  ui->tableNfeSaida->hideColumn("NFe");
  ui->tableNfeSaida->setItemDelegate(doubledelegate);
}

void WidgetNfe::updateTables() {
  switch (ui->tabWidgetNfe->currentIndex()) {
    case 0: // Entrada
      if (not modelNfeEntrada->select()) {
        QMessageBox::critical(this, "Erro!", "Erro lendo tabela NFe: " + modelNfeEntrada->lastError().text());
        return;
      }

      ui->tableNfeEntrada->resizeColumnsToContents();
      break;

    case 1: // Saida
      if (not modelNfeSaida->select()) {
        QMessageBox::critical(this, "Erro!", "Erro lendo tabela NFe: " + modelNfeSaida->lastError().text());
        return;
      }

      ui->tableNfeSaida->resizeColumnsToContents();
      break;
    default:
      break;
  }
}

void WidgetNfe::on_radioButtonNFeAutorizado_clicked() {
  modelNfeSaida->setFilter("status = 'autorizado'");
  ui->tableNfeSaida->resizeColumnsToContents();
}

void WidgetNfe::on_radioButtonNFeEnviado_clicked() {
  modelNfeSaida->setFilter("status = 'enviado'");
  ui->tableNfeSaida->resizeColumnsToContents();
}

void WidgetNfe::on_radioButtonNFeLimpar_clicked() {
  modelNfeSaida->setFilter("");
  ui->tableNfeSaida->resizeColumnsToContents();
}

void WidgetNfe::on_lineEditBuscaNFe_textChanged(const QString &text) {
  modelNfeSaida->setFilter(text.isEmpty() ? "" : "(idVenda LIKE '%" + text + "%') OR (status LIKE '%" + text + "%')");

  ui->tableNfeSaida->resizeColumnsToContents();
}

void WidgetNfe::on_tableNfeSaida_activated(const QModelIndex &index) {
  Venda *vendas = new Venda(this);
  vendas->viewRegisterById(modelNfeSaida->data(index.row(), "idVenda"));
}

void WidgetNfe::on_tableNfeEntrada_activated(const QModelIndex &index) {
  XML_Viewer *viewer = new XML_Viewer(this);

  viewer->exibirXML(modelNfeEntrada->data(index.row(), "xml").toString());
  viewer->show();
}

void WidgetNfe::on_tabWidgetNfe_currentChanged(int) { updateTables(); }

void WidgetNfe::on_pushButtonExibirXML_clicked() {
  QString xml = QFileDialog::getOpenFileName(this, "Arquivo XML", QDir::currentPath(), "*.xml");

  if (xml.isEmpty()) {
    return;
  }

  QFile file(xml);

  if (not file.open(QFile::ReadOnly)) {
    QMessageBox::critical(this, "Erro!", "Erro abrindo arquivo: " + file.errorString());
    return;
  }

  xml = file.readAll();

  XML_Viewer *viewer = new XML_Viewer;
  viewer->exibirXML(xml);
  viewer->show();
}
