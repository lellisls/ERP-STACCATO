#include <QMessageBox>
#include <QShortcut>
#include <QStyleFactory>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cadastrocliente.h"
#include "cadastrofornecedor.h"
#include "cadastroloja.h"
#include "cadastroproduto.h"
#include "cadastroprofissional.h"
#include "cadastrotransportadora.h"
#include "cadastrousuario.h"
#include "importaprodutos.h"
#include "orcamento.h"
#include "usersession.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  defaultStyle = this->style()->objectName();
  defautPalette = qApp->palette();

  setWindowIcon(QIcon("Staccato.ico"));
  setWindowTitle("ERP Staccato");

  QShortcut *shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this);
  connect(shortcut, &QShortcut::activated, this, &QWidget::close);

  setWindowTitle(windowTitle() + " - " + UserSession::getNome() + " - " + UserSession::getTipoUsuario() + " - " +
                 settings("Login/hostname").toString() +
                 (settings("Login/homologacao").toBool() ? " - HOMOLOGACAO" : ""));

  if (UserSession::getTipoUsuario() != "ADMINISTRADOR") {
    ui->actionGerenciar_Lojas->setDisabled(true);
    ui->actionGerenciar_Transportadoras->setDisabled(true);
    ui->actionImportaProdutos->setDisabled(true);
    ui->actionCadastrarUsuario->setDisabled(true);
    ui->actionCadastrarProfissional->setDisabled(true);
    ui->actionCadastrarFornecedor->setDisabled(true);
  }

  if (UserSession::getTipoUsuario() == "VENDEDOR") {
    ui->tabWidget->setTabEnabled(2, false);
    ui->tabWidget->setTabEnabled(3, false);
    ui->tabWidget->setTabEnabled(4, false);
    ui->tabWidget->setTabEnabled(5, false);
    ui->tabWidget->setTabEnabled(6, false);
    ui->actionCadastrarUsuario->setVisible(false);
  }
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

void MainWindow::updateTables() {
  switch (ui->tabWidget->currentIndex()) {
    case 0: // Orcamentos
      ui->widgetOrcamento->updateTables();
      break;

    case 1: // Vendas
      ui->widgetVenda->updateTables();
      break;

    case 2: // Compras
      ui->widgetCompra->updateTables();
      break;

    case 3: // Logistica
      ui->widgetLogistica->updateTables();
      break;

    case 4: // NFe
      ui->widgetNfe->updateTables();
      break;

    case 5: // Estoque
      ui->widgetEstoque->updateTables();
      break;

    case 6: // Contas
      ui->widgetConta->updateTables();
      break;

    default:
      break;
  }
}

void MainWindow::on_actionCadastrarFornecedor_triggered() {
  CadastroFornecedor *cad = new CadastroFornecedor(this);
  cad->show();
}

QVariant MainWindow::settings(QString key) const { return UserSession::getSettings(key); }

void MainWindow::setSettings(QString key, QVariant value) const { UserSession::setSettings(key, value); }

void MainWindow::on_actionImportaProdutos_triggered() {
  ImportaProdutos *importa = new ImportaProdutos(this);
  importa->importar();
}

bool MainWindow::event(QEvent *e) {
  // TODO: usar um bool para verificar se deu erro e nao entrar em ciclo de updateTables
  switch (e->type()) {
    case QEvent::WindowActivate:
      updateTables();
      break;

    case QEvent::WindowDeactivate:
      break;

    default:
      break;
  };

  return QMainWindow::event(e);
}

void MainWindow::darkTheme() {
  // TODO: texto no campo amarelo nao visivel
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

void MainWindow::on_tabWidget_currentChanged(int) { updateTables(); }

void MainWindow::on_actionSobre_triggered() {
  // TODO: adicionar informacoes de contato do desenvolvedor (telefone/email)
  QMessageBox::about(this, "Sobre ERP Staccato", "Versão " + qApp->applicationVersion());
}

void MainWindow::on_actionClaro_triggered() {
  qApp->setStyle(defaultStyle);
  qApp->setPalette(defautPalette);
  qApp->setStyleSheet(styleSheet());
}

void MainWindow::on_actionEscuro_triggered() { darkTheme(); }

void MainWindow::on_actionConfigura_es_triggered() {
  // TODO: put screen to change user variables (userFolder etc)
}

// TODO: gerenciar lugares de estoque (cadastro/permissoes)
// TODO: a tabela de fornecedores em compra deve mostrar apenas os pedidos que estejam pendente/confirmar/faturar
// TODO: a tabela de fornecedores em logistica deve mostrar apenas os pedidos que estejam coleta/recebimento/entrega
// TODO: colocar logo da staccato na mainwindow
// TODO: renomear "Mostrar inativos" para mostrar removidos e adicionar esse checkbox na searchdialog
// TODO: add 'AND desativado = false' in tables where there is desativado
// TODO: colocar accessibleName em todas as telas de cadastro
// TODO: adicionar Qt::WA_deleteOnClose? para nao ficar segurando recursos??
