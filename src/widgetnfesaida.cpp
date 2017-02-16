#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QUrl>

#include "acbr.h"
#include "doubledelegate.h"
#include "lrreportengine.h"
#include "ui_widgetnfesaida.h"
#include "usersession.h"
#include "widgetnfesaida.h"
#include "xml_viewer.h"

WidgetNfeSaida::WidgetNfeSaida(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetNfeSaida) { ui->setupUi(this); }

WidgetNfeSaida::~WidgetNfeSaida() { delete ui; }

bool WidgetNfeSaida::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela NFe: " + model.lastError().text());
    return false;
  }

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetNfeSaida::setupTables() {
  model.setTable("view_nfe_saida");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  model.setHeaderData("created", "Criado em");

  ui->table->setModel(&model);
  ui->table->hideColumn("chaveAcesso");
  ui->table->setItemDelegate(new DoubleDelegate(this));
}

void WidgetNfeSaida::on_table_activated(const QModelIndex &index) {
  QSqlQuery query;
  query.prepare("SELECT xml FROM nfe WHERE numeroNFe = :numeroNFe");
  query.bindValue(":numeroNFe", model.data(index.row(), "NFe"));

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando xml da nota: " + query.lastError().text());
    return;
  }

  XML_Viewer *viewer = new XML_Viewer(this);
  viewer->setAttribute(Qt::WA_DeleteOnClose);
  viewer->exibirXML(query.value("xml").toByteArray());
}

void WidgetNfeSaida::on_lineEditBusca_textChanged(const QString &text) {
  model.setFilter("NFe LIKE '%" + text + "%' OR Venda LIKE '%" + text + "%' OR `CPF/CNPJ` LIKE '%" + text +
                  "%' OR Cliente LIKE '%" + text + "%'");

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());

  ui->table->resizeColumnsToContents();
}

void WidgetNfeSaida::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetNfeSaida::on_pushButtonCancelarNFe_clicked() {
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhuma linha selecionada!");
    return;
  }

  const QString justificativa = QInputDialog::getText(this, "Justificativa", "Entre 15 e 200 caracteres: ");

  if (justificativa.size() < 15 or justificativa.size() > 200) {
    QMessageBox::critical(this, "Erro!", "Justificativa fora do tamanho!");
    return;
  }

  const QString chaveAcesso = model.data(list.first().row(), "chaveAcesso").toString();

  const QString comando = "NFE.CancelarNFe(" + chaveAcesso + ", " + justificativa + ")";

  QString resposta;
  ACBr::enviarComando(comando, resposta);

  if (resposta.contains("XMotivo=Evento registrado e vinculado a NF-e")) {
    QSqlQuery query;
    query.prepare("UPDATE nfe SET status = 'CANCELADO' WHERE chaveAcesso = :chaveAcesso");
    query.bindValue(":chaveAcesso", chaveAcesso);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro marcando nota como cancelada: " + query.lastError().text());
    }
  }

  QMessageBox::information(this, "Resposta", resposta);
}

// TODO: 1verificar se ao cancelar nota ela é removida do venda_produto/veiculo_produto
// TODO: 1botao para gerar relatorio igual ao da receita

void WidgetNfeSaida::on_pushButtonRelatorio_clicked() {
  // TODO: selecionar quais itens vao no relatorio
  // gerar relatorio pelo limereport?

  LimeReport::ReportEngine report;

  SqlTableModel view;
  view.setTable("view_relatorio_nfe");
  view.select();

  qDebug() << report.dataManager()->addModel("view", &view, true);

  qDebug() << report.loadFromFile("view.lrxml");

  report.printToPDF("C:/temp/relatorio.pdf");
  QDesktopServices::openUrl(QUrl::fromLocalFile("C:/temp/relatorio.pdf"));
}

void WidgetNfeSaida::on_pushButtonExportar_clicked() {
  //
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  for (const auto &item : list) {
    // TODO: 3wrap this in a function so if connection to acbr is dropped the erp retries

    if (model.data(item.row(), "status").toString() != "AUTORIZADO") continue;

    // pegar xml do bd e salvar em arquivo

    const QString chaveAcesso = model.data(item.row(), "chaveAcesso").toString();

    QSqlQuery query;
    query.prepare("SELECT xml FROM nfe WHERE chaveAcesso = :chaveAcesso");
    query.bindValue(":chaveAcesso", chaveAcesso);

    if (not query.exec() or not query.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando xml: " + query.lastError().text());
      return;
    }

    QFile fileXml(UserSession::settings("User/EntregasXmlFolder").toString() + "/" + chaveAcesso + ".xml");

    if (not fileXml.open(QFile::WriteOnly)) {
      QMessageBox::critical(this, "Erro!", "Erro abrindo arquivo para escrita xml: " + fileXml.errorString());
      return;
    }

    fileXml.write(query.value("xml").toByteArray());

    fileXml.flush();
    fileXml.close();

    // mandar xml para acbr gerar pdf

    QString pdfOrigem;
    ACBr::gerarDanfe(query.value("xml").toByteArray(), pdfOrigem, false);

    if (pdfOrigem.isEmpty()) {
      QMessageBox::critical(this, "Erro!", "pdf is empty!");
      return;
    }

    // copiar para pasta predefinida

    const QString pdfDestino = UserSession::settings("User/EntregasPdfFolder").toString() + "/" + chaveAcesso + ".pdf";

    QFile filePdf(pdfDestino);

    if (filePdf.exists()) filePdf.remove();

    if (not QFile::copy(pdfOrigem, pdfDestino)) {
      QMessageBox::critical(this, "Erro!", "Erro copiando pdf!");
      return;
    }
  }
}

// TODO: 2tela para importar notas de amostra (aba separada)
// TODO: colocar filtros
