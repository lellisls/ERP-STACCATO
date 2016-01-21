#include <QDebug>
#include <QDesktopServices>
#include <QLabel>
#include <QMessageBox>
#include <QShortcut>
#include <QStyleFactory>
#include <QTimer>
#include <QUrl>

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
#include "ui_mainwindow.h"
#include "userconfig.h"
#include "usersession.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  defaultStyle = this->style()->objectName();
  defautPalette = qApp->palette();

  setWindowIcon(QIcon("Staccato.ico"));
  setWindowTitle("ERP Staccato");

  QShortcut *shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this);
  connect(shortcut, &QShortcut::activated, this, &QWidget::close);

  setWindowTitle(windowTitle() + " - " + UserSession::nome() + " - " + UserSession::tipoUsuario() + " - " +
                 UserSession::settings("Login/hostname").toString() +
                 (UserSession::settings("Login/homologacao").toBool() ? " - HOMOLOGACAO" : ""));

  if (UserSession::tipoUsuario() != "ADMINISTRADOR" and UserSession::tipoUsuario() != "GERENTE LOJA") {
    ui->actionGerenciar_Lojas->setDisabled(true);
    ui->actionGerenciar_Transportadoras->setDisabled(true);
    ui->actionImportaProdutos->setDisabled(true);
    ui->actionCadastrarUsuario->setDisabled(true);
    ui->actionCadastrarProfissional->setDisabled(true);
    ui->actionCadastrarFornecedor->setDisabled(true);
  }

  if (UserSession::tipoUsuario() == "VENDEDOR") {
    ui->tabWidget->setTabEnabled(2, false);
    ui->tabWidget->setTabEnabled(3, false);
    ui->tabWidget->setTabEnabled(4, false);
    ui->tabWidget->setTabEnabled(5, false);
    ui->tabWidget->setTabEnabled(6, false);
    ui->actionCadastrarUsuario->setVisible(false);
  }

  timer = new QTimer(this);
}

MainWindow::~MainWindow() {
  delete ui;
  UserSession::free();
}

void MainWindow::on_actionCriarOrcamento_triggered() {
  Orcamento *orcamento = new Orcamento(this);
  orcamento->show();
}

void MainWindow::on_actionCadastrarProdutos_triggered() {
  CadastroProduto *cad = new CadastroProduto(this);
  cad->show();
}

void MainWindow::on_actionCadastrarCliente_triggered() {
  CadastroCliente *cad = new CadastroCliente(this);
  cad->show();
}

void MainWindow::on_actionCadastrarUsuario_triggered() {
  CadastroUsuario *cad = new CadastroUsuario(this);
  cad->show();
}

void MainWindow::on_actionCadastrarProfissional_triggered() {
  CadastroProfissional *cad = new CadastroProfissional(this);
  cad->show();
}

void MainWindow::on_actionGerenciar_Transportadoras_triggered() {
  CadastroTransportadora *cad = new CadastroTransportadora(this);
  cad->show();
}

void MainWindow::on_actionGerenciar_Lojas_triggered() {
  CadastroLoja *cad = new CadastroLoja(this);
  cad->show();
}

void MainWindow::showStatusBarMessage() { ui->statusBar->showMessage(error, 125); }

void MainWindow::timerStatusBar() {
  disconnect(timer, &QTimer::timeout, this, &MainWindow::showStatusBarMessage);
  connect(timer, &QTimer::timeout, this, &MainWindow::showStatusBarMessage);
  timer->start(250);
}

void MainWindow::updateTables() {
  switch (ui->tabWidget->currentIndex()) {
    case 0: { // Orcamentos
        error = ui->widgetOrcamento->updateTables();
        break;
      }

    case 1: { // Vendas
        error = ui->widgetVenda->updateTables();
        break;
      }

    case 2: { // Compras
        error = ui->widgetCompra->updateTables();
        break;
      }

    case 3: { // Logistica
        error = ui->widgetLogistica->updateTables();
        break;
      }

    case 4: { // NFe
        error = ui->widgetNfe->updateTables();
        break;
      }

    case 5: { // Estoque
        error = ui->widgetEstoque->updateTables();
        break;
      }

    case 6: { // Contas
        error = ui->widgetPagar->updateTables();
        error = ui->widgetReceber->updateTables();
        break;
      }
  }

  if (not error.isEmpty()) timerStatusBar();
}

void MainWindow::on_actionCadastrarFornecedor_triggered() {
  CadastroFornecedor *cad = new CadastroFornecedor(this);
  cad->show();
}

void MainWindow::on_actionImportaProdutos_triggered() {
  ImportaProdutos *importa = new ImportaProdutos(this);
  importa->importar();
}

bool MainWindow::event(QEvent *event) {
  switch (event->type()) {
    case QEvent::WindowActivate: {
        updateTables();

        break;
      }

    case QEvent::WindowDeactivate: {
        break;
      }

    default:
      break;
  };

  return QMainWindow::event(event);
}

void MainWindow::darkTheme() {
  // FIXME: texto no campo amarelo nao visivel
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

  darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
  darkPalette.setColor(QPalette::HighlightedText, Qt::black);

  qApp->setPalette(darkPalette);

  qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
}

void MainWindow::on_tabWidget_currentChanged(const int &) { updateTables(); }

void MainWindow::on_actionSobre_triggered() {
  QMessageBox::about(
        this, "Sobre ERP Staccato",
        "Versão " + qApp->applicationVersion() +
        "\nDesenvolvedor: Rodrigo Torres\nCelular/WhatsApp: (12)98138-3504\nE-mail: torres.dark@gmail.com");
}

void MainWindow::on_actionClaro_triggered() {
  qApp->setStyle(defaultStyle);
  qApp->setPalette(defautPalette);
  qApp->setStyleSheet(styleSheet());
}

void MainWindow::on_actionEscuro_triggered() { darkTheme(); }

void MainWindow::on_actionConfiguracoes_triggered() {
  UserConfig *config = new UserConfig(this);
  config->show();
}

void MainWindow::on_actionCalculadora_triggered() {
  QDesktopServices::openUrl(QUrl::fromLocalFile("C:\\Windows\\System32\\calc.exe"));
}

// NOTE: colocar logo da staccato na mainwindow
