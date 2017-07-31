#include <QDate>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QShortcut>
#include <QSqlError>
#include <QStyleFactory>
#include <QTimer>
#include <QUrl>

#include "acbr.h"
#include "cadastrarnfe.h"
#include "cadastrocliente.h"
#include "cadastrofornecedor.h"
#include "cadastroloja.h"
#include "cadastroproduto.h"
#include "cadastroprofissional.h"
#include "cadastrotransportadora.h"
#include "cadastrousuario.h"
#include "importaprodutos.h"
#include "mainwindow.h"
#include "orcamento.h"
#include "sendmail.h"
#include "ui_mainwindow.h"
#include "userconfig.h"
#include "usersession.h"
#include "xlsxdocument.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  defaultStyle = style()->objectName();
  defautPalette = qApp->palette();

  if (UserSession::settings("User/tema").toString() == "escuro") darkTheme();

  setWindowIcon(QIcon("Staccato.ico"));
  setWindowTitle("ERP Staccato");

  QShortcut *shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this);
  connect(shortcut, &QShortcut::activated, this, &QWidget::close);

  setWindowTitle(windowTitle() + " - " + UserSession::nome() + " - " + UserSession::tipoUsuario() + " - " + UserSession::settings("Login/hostname").toString() +
                 (UserSession::settings("Login/homologacao").toBool() ? " - HOMOLOGACAO" : ""));

  if (UserSession::tipoUsuario() != "ADMINISTRADOR" and UserSession::tipoUsuario() != "GERENTE LOJA") {
    ui->actionGerenciar_Lojas->setDisabled(true);
    ui->actionGerenciar_Transportadoras->setDisabled(true);
    ui->menuImportar_tabela_fornecedor->setDisabled(true);
    ui->actionCadastrarUsuario->setDisabled(true);
    ui->actionCadastrarProfissional->setDisabled(true);
    ui->actionCadastrarFornecedor->setDisabled(true);
  }

  //

  QSqlQuery query;
  query.prepare("SELECT * FROM usuario_has_permissao WHERE idUsuario = :idUsuario");
  query.bindValue(":idUsuario", UserSession::idUsuario());

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo permissões: " + query.lastError().text());
  }

  ui->tabWidget->setTabEnabled(0, query.value("view_tab_orcamento").toBool());
  ui->tabWidget->setTabEnabled(1, query.value("view_tab_venda").toBool());
  ui->tabWidget->setTabEnabled(2, query.value("view_tab_compra").toBool());
  ui->tabWidget->setTabEnabled(3, query.value("view_tab_logistica").toBool());
  ui->tabWidget->setTabEnabled(4, query.value("view_tab_nfe").toBool());
  ui->tabWidget->setTabEnabled(5, query.value("view_tab_estoque").toBool());
  ui->tabWidget->setTabEnabled(6, query.value("view_tab_financeiro").toBool());
  ui->tabWidget->setTabEnabled(7, query.value("view_tab_relatorio").toBool());
  //

  connect(ui->widgetOrcamento, &WidgetOrcamento::errorSignal, this, &MainWindow::timerStatusBar);
  connect(ui->widgetVenda, &WidgetVenda::errorSignal, this, &MainWindow::timerStatusBar);
  connect(ui->widgetCompra, &WidgetCompra::errorSignal, this, &MainWindow::timerStatusBar);
  connect(ui->widgetLogistica, &WidgetLogistica::errorSignal, this, &MainWindow::timerStatusBar);
  connect(ui->widgetNfe, &WidgetNfe::errorSignal, this, &MainWindow::timerStatusBar);
  connect(ui->widgetEstoque, &WidgetEstoque::errorSignal, this, &MainWindow::timerStatusBar);
  connect(ui->widgetFinanceiro, &WidgetFinanceiro::errorSignal, this, &MainWindow::timerStatusBar);
  connect(ui->widgetRelatorio, &WidgetRelatorio::errorSignal, this, &MainWindow::timerStatusBar);

  timer = new QTimer(this);

  ui->pushButton->hide();

  gerarEnviarRelatorio();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::gerarEnviarRelatorio() {
  // TODO: finish
  // verificar em que etapa eu guardo a linha do dia seguinte no BD

  QSqlQuery query;
  query.prepare("SELECT * FROM jobs WHERE dataReferente = :dataReferente AND status = 'PENDENTE'");
  query.bindValue(":dataAgendado", QDate::currentDate());

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando relatórios agendados: " + query.lastError().text());
    return;
  }

  while (query.next()) {
    const QString relatorioPagar = "C:/temp/pagar.xlsx";     // guardar direto no servidor?
    const QString relatorioReceber = "C:/temp/receber.xlsx"; // e se o computador nao tiver o servidor mapeado?

    //

    QXlsx::Document xlsxPagar(relatorioPagar);

    QSqlQuery queryView;

    if (not queryView.exec("SELECT * FROM view_relatorio_pagar")) {
      QMessageBox::critical(this, "Erro!", "Erro lendo relatorio pagar: " + queryView.lastError().text());
      return;
    }

    xlsxPagar.write("A1", "Data Emissão");
    xlsxPagar.write("B1", "Data Realizado");
    xlsxPagar.write("C1", "Valor R$");
    xlsxPagar.write("D1", "Conta");
    xlsxPagar.write("E1", "Obs.");
    xlsxPagar.write("F1", "Contraparte");
    xlsxPagar.write("G1", "Grupo");
    xlsxPagar.write("H1", "Subgrupo");

    int row = 1;

    while (queryView.next()) {
      xlsxPagar.write("A" + QString::number(row), queryView.value("dataEmissao"));
      xlsxPagar.write("B" + QString::number(row), queryView.value("dataRealizado"));
      xlsxPagar.write("C" + QString::number(row), queryView.value("valorReal"));
      xlsxPagar.write("D" + QString::number(row), queryView.value("Conta"));
      xlsxPagar.write("E" + QString::number(row), queryView.value("observacao"));
      xlsxPagar.write("F" + QString::number(row), queryView.value("contraParte"));
      xlsxPagar.write("G" + QString::number(row), queryView.value("grupo"));
      xlsxPagar.write("H" + QString::number(row), queryView.value("subGrupo"));

      ++row;
    }

    //

    QXlsx::Document xlsxReceber(relatorioReceber);

    if (not queryView.exec("SELECT * FROM view_relatorio_receber")) {
      QMessageBox::critical(this, "Erro!", "Erro lendo relatorio receber: " + queryView.lastError().text());
      return;
    }

    xlsxReceber.write("A1", "dataEmissao");
    xlsxReceber.write("B1", "dataRealizado");
    xlsxReceber.write("C1", "valorReal");
    xlsxReceber.write("D1", "Conta");
    xlsxReceber.write("E1", "observacao");
    xlsxReceber.write("F1", "contraParte");
    xlsxReceber.write("G1", "grupo");
    xlsxReceber.write("H1", "subGrupo");

    row = 1;

    while (queryView.next()) {
      xlsxReceber.write("A" + QString::number(row), queryView.value("dataEmissao"));
      xlsxReceber.write("B" + QString::number(row), queryView.value("dataRealizado"));
      xlsxReceber.write("C" + QString::number(row), queryView.value("valorReal"));
      xlsxReceber.write("D" + QString::number(row), queryView.value("Conta"));
      xlsxReceber.write("E" + QString::number(row), queryView.value("observacao"));
      xlsxReceber.write("F" + QString::number(row), queryView.value("contraParte"));
      xlsxReceber.write("G" + QString::number(row), queryView.value("grupo"));
      xlsxReceber.write("H" + QString::number(row), queryView.value("subGrupo"));

      ++row;
    }

    //

    QSqlQuery query2;
    query2.prepare("INSERT INTO jobs (dataEnviado, dataReferente, status) VALUES (:dataEnviado, :dataReferente, 'ENVIADO')");

    const int diaSemana = QDate::currentDate().dayOfWeek();

    query2.bindValue(":dataReferente", QDate::currentDate().addDays(diaSemana < 4 ? 5 : diaSemana - 3));
    query2.bindValue(":dataEnviado", QDate::currentDate());

    if (not query2.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro guardando relatórios financeiro: " + query2.lastError().text());
      return;
    }

    //    SendMail *mail = new SendMail(this, anexo, fornecedor);
    //    mail->setAttribute(Qt::WA_DeleteOnClose);

    //    mail->exec();
  }
}

void MainWindow::on_actionCriarOrcamento_triggered() {
  auto *orcamento = new Orcamento(this);
  orcamento->setAttribute(Qt::WA_DeleteOnClose);
  orcamento->show();
}

void MainWindow::on_actionCadastrarProdutos_triggered() {
  auto *cad = new CadastroProduto(this);
  cad->setAttribute(Qt::WA_DeleteOnClose);
  cad->show();
}

void MainWindow::on_actionCadastrarCliente_triggered() {
  auto *cad = new CadastroCliente(this);
  cad->setAttribute(Qt::WA_DeleteOnClose);
  cad->show();
}

void MainWindow::on_actionCadastrarUsuario_triggered() {
  auto *cad = new CadastroUsuario(this);
  cad->setAttribute(Qt::WA_DeleteOnClose);
  cad->show();
}

void MainWindow::on_actionCadastrarProfissional_triggered() {
  auto *cad = new CadastroProfissional(this);
  cad->setAttribute(Qt::WA_DeleteOnClose);
  cad->show();
}

void MainWindow::on_actionGerenciar_Transportadoras_triggered() {
  auto *cad = new CadastroTransportadora(this);
  cad->setAttribute(Qt::WA_DeleteOnClose);
  cad->show();
}

void MainWindow::on_actionGerenciar_Lojas_triggered() {
  auto *cad = new CadastroLoja(this);
  cad->setAttribute(Qt::WA_DeleteOnClose);
  cad->show();
}

void MainWindow::showStatusBarMessage() { ui->statusBar->showMessage(error, 125); }

void MainWindow::timerStatusBar(const QString &error) {
  this->error = error;
  disconnect(timer, &QTimer::timeout, this, &MainWindow::showStatusBarMessage);
  connect(timer, &QTimer::timeout, this, &MainWindow::showStatusBarMessage);
  timer->start(250);
}

void MainWindow::updateTables() {
  const QString currentText = ui->tabWidget->tabText(ui->tabWidget->currentIndex());

  if (currentText == "Orçamentos") ui->widgetOrcamento->updateTables();
  if (currentText == "Vendas") ui->widgetVenda->updateTables();
  if (currentText == "Compras") ui->widgetCompra->updateTables();
  if (currentText == "Logística") ui->widgetLogistica->updateTables();
  if (currentText == "NFe") ui->widgetNfe->updateTables();
  if (currentText == "Estoque") ui->widgetEstoque->updateTables();
  if (currentText == "Financeiro") ui->widgetFinanceiro->updateTables();
  if (currentText == "Relatórios") ui->widgetRelatorio->updateTables();
}

void MainWindow::on_actionCadastrarFornecedor_triggered() {
  auto *cad = new CadastroFornecedor(this);
  cad->setAttribute(Qt::WA_DeleteOnClose);
  cad->show();
}

bool MainWindow::event(QEvent *event) {
  switch (event->type()) {
  case QEvent::WindowActivate:
    updateTables();
    break;

  case QEvent::WindowDeactivate:
    break;

  default:
    break;
  }

  return QMainWindow::event(event);
}

void MainWindow::darkTheme() {
  qApp->setStyle(QStyleFactory::create("Fusion"));

  QPalette darkPalette;
  darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::WindowText, Qt::white);
  darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
  darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
  darkPalette.setColor(QPalette::ToolTipText, Qt::white);
  darkPalette.setColor(QPalette::Text, Qt::white);
  darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::ButtonText, Qt::white);
  darkPalette.setColor(QPalette::BrightText, Qt::red);
  darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
  darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(120, 120, 120));

  darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
  darkPalette.setColor(QPalette::HighlightedText, Qt::black);

  qApp->setPalette(darkPalette);

  qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
}

void MainWindow::on_tabWidget_currentChanged(const int) { updateTables(); }

void MainWindow::on_actionSobre_triggered() {
  QMessageBox::about(this, "Sobre ERP Staccato", "Versão " + qApp->applicationVersion() + "\nDesenvolvedor: Rodrigo Torres\nCelular/WhatsApp: (12)98138-3504\nE-mail: torres.dark@gmail.com");
}

void MainWindow::on_actionClaro_triggered() {
  qApp->setStyle(defaultStyle);
  qApp->setPalette(defautPalette);
  qApp->setStyleSheet(styleSheet());
  UserSession::setSettings("User/tema", "claro");
}

void MainWindow::on_actionEscuro_triggered() {
  darkTheme();
  UserSession::setSettings("User/tema", "escuro");
}

void MainWindow::on_actionConfiguracoes_triggered() {
  auto *config = new UserConfig(this);
  config->setAttribute(Qt::WA_DeleteOnClose);
  config->show();
}

void MainWindow::on_actionCalculadora_triggered() { QDesktopServices::openUrl(QUrl::fromLocalFile(R"(C:\Windows\System32\calc.exe)")); }

void MainWindow::on_actionProdutos_triggered() {
  auto *importa = new ImportaProdutos(this);
  importa->setAttribute(Qt::WA_DeleteOnClose);
  importa->importarProduto();
}

void MainWindow::on_actionEstoque_triggered() {
  auto *importa = new ImportaProdutos(this);
  importa->setAttribute(Qt::WA_DeleteOnClose);
  importa->importarEstoque();
}

void MainWindow::on_actionPromocao_triggered() {
  auto *importa = new ImportaProdutos(this);
  importa->setAttribute(Qt::WA_DeleteOnClose);
  importa->importarPromocao();
}

void MainWindow::on_pushButton_clicked() {
  // TODO: funcao usada para gerar nota de transferencia, guardar codigo em arquivo e apagar funcao
  const QString filePath = QFileDialog::getOpenFileName(this, "Transferencia", QDir::currentPath(), "Excel (*.xlsx)");

  if (filePath.isEmpty()) {
    return;
  }

  QFile file(filePath);

  if (not file.open(QFile::ReadOnly)) {
    QMessageBox::critical(this, "Erro!", "Erro lendo arquivo: " + file.errorString());
    return;
  }

  file.close();

  QXlsx::Document xlsx(filePath);

  if (not xlsx.selectSheet("TRANSFERIR")) {
    QMessageBox::critical(this, "Erro!", "Erro selecionando planilha 'TRANSFERIR'");
    return;
  }

  // A - ncm
  // B - codigo
  // C - descricao
  // D - un
  // E - quant
  // F - valorUnid
  // G - valor

  QFile txt("test.txt");

  if (not txt.open(QFile::WriteOnly)) {
    QMessageBox::critical(this, "Erro!", "Erro lendo arquivo: " + txt.errorString());
    return;
  }

  QTextStream stream(&txt);

  for (int row = 1; row < 90; ++row) {
    const QString numProd = QString("%1").arg(row, 3, 10, QChar('0')); // padding with zeros
    stream << "[Produto" + numProd + "]" << endl;

    stream << "CFOP = 5409" << endl;
    stream << "CEST = 1003001" << endl;
    stream << "NCM = " << xlsx.read("A" + QString::number(row)).toString() << endl;
    stream << "Codigo = " << xlsx.read("B" + QString::number(row)).toString() << endl;
    stream << "Descricao = " << xlsx.read("C" + QString::number(row)).toString() << endl;
    stream << "Unidade = " << xlsx.read("D" + QString::number(row)).toString() << endl;
    stream << "Quantidade = " << xlsx.read("E" + QString::number(row)).toString() << endl;
    stream << "ValorUnitario = " << xlsx.read("F" + QString::number(row)).toString() << endl;
    stream << "ValorTotal = " << xlsx.read("G" + QString::number(row)).toString() << endl;
    stream << "vFrete = 0" << endl;

    stream << endl;

    stream << "[ICMS" + numProd + "]" << endl;
    stream << "CST = 60" << endl;
    stream << "Modalidade = 0" << endl;
    stream << "ValorBase = 0" << endl;
    stream << "Aliquota = 0" << endl;
    stream << "Valor = 0" << endl;

    stream << endl;

    stream << "[IPI" + numProd + "]" << endl;
    stream << "ClasseEnquadramento = 0" << endl;
    stream << "CST = 0" << endl;

    stream << endl;

    stream << "[PIS" + numProd + "]" << endl;
    stream << "CST = 49" << endl;
    stream << "ValorBase = 0" << endl;
    stream << "Aliquota = 0" << endl;
    stream << "Valor = 0" << endl;

    stream << endl;

    stream << "[COFINS" + numProd + "]" << endl;
    stream << "CST = 49" << endl;
    stream << "ValorBase = 0" << endl;
    stream << "Aliquota = 0" << endl;
    stream << "Valor = 0" << endl;

    stream << endl;
  }

  stream.flush();

  txt.close();
}

// TODO: montar relatorio dos caminhoes com graficos e total semanal, mensal, custos etc
// NOTE: colocar logo da staccato na mainwindow

// NOTE: prioridades atuais:
// NOTE: -logistica da devolucao
// NOTE: -email/nfe acbr
// TODO: alguns elementos graficos estao incorretos com o tema escuro
