#include <QShortcut>
#include <QStyleFactory>
#include <QFileDialog>
#include <QInputDialog>
#include <QSqlError>

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
#include "logindialog.h"
#include "orcamento.h"
#include "QSimpleUpdater"
#include "usersession.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  defaultStyle = this->style()->objectName();
  defautPalette = qApp->palette();

  qApp->setApplicationVersion("0.6");

  //  setSettings("Login/hostname", ""); //to test store selection

  if (settings("Login/hostname").toString().isEmpty()) {
    QStringList items;
    items << "Alphaville"
          << "Gabriel";

    QString loja = QInputDialog::getItem(this, "Escolha a loja", "Qual a sua loja?", items, 0, false);

    // TODO: add granja
    if (loja == "Alphaville") {
      setSettings("Login/hostname", "192.168.2.144");
    } else if (loja == "Gabriel") {
      setSettings("Login/hostname", "192.168.1.101");
    }

    setSettings("Login/username", "user");
    setSettings("Login/password", "1234");
    setSettings("Login/port", "3306");
    setSettings("Login/homologacao", false);
  }

  hostname = settings("Login/hostname").toString();
  username = settings("Login/username").toString();
  password = settings("Login/password").toString();
  port = settings("Login/port").toString();
  homologacao = settings("Login/homologacao").toBool();

  if (settings("User/userFolder").toString().isEmpty()) {
    QMessageBox::warning(this, "Aviso!", "Não há uma pasta definida para salvar PDF/Excel. Por favor escolha uma.");
    setSettings("User/userFolder", QFileDialog::getExistingDirectory(this, "Pasta PDF/Excel"));
  }

  QSimpleUpdater *updater = new QSimpleUpdater(this);
  updater->setApplicationVersion(qApp->applicationVersion());
  updater->setReferenceUrl("http://" + hostname + "/versao.txt");
  updater->setDownloadUrl("http://" + hostname + "/Instalador.exe");
  updater->setSilent(true);
  updater->setShowNewestVersionMessage(true);
  updater->checkForUpdates();

  LoginDialog *dialog = new LoginDialog(this);

  if (dialog->exec() == QDialog::Rejected) {
    exit(1);
  }

  ui->setupUi(this);

  setWindowIcon(QIcon("Staccato.ico"));
  setWindowTitle("ERP Staccato");
  readSettings();

  QShortcut *shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this);
  connect(shortcut, &QShortcut::activated, this, &QWidget::close);

  setWindowTitle(windowTitle() + " - " + UserSession::getNome() + " - " + UserSession::getTipoUsuario() + " - " +
                 hostname + (homologacao ? " - HOMOLOGACAO" : ""));

  if (UserSession::getTipoUsuario() != "ADMINISTRADOR") {
    ui->actionGerenciar_Lojas->setDisabled(true);
    ui->actionGerenciar_Transportadoras->setDisabled(true);
    ui->actionImportaProdutos->setDisabled(true);
    ui->actionCadastrarUsuario->setDisabled(true);
    ui->actionCadastrarProfissional->setDisabled(true);
    ui->actionCadastrarFornecedor->setDisabled(true);
  }

  if (UserSession::getTipoUsuario() == "VENDEDOR") {
    //    ui->tableRecebimentosFornecedor->hide();
    //    ui->labelRecebimentosFornecedor->hide();
    ui->tabWidget->setTabEnabled(2, false);
    ui->tabWidget->setTabEnabled(3, false);
    ui->tabWidget->setTabEnabled(4, false);
    ui->tabWidget->setTabEnabled(5, false);
    ui->tabWidget->setTabEnabled(6, false);
    // ui->tablePedidosCompra->hide();
    ui->actionCadastrarUsuario->setVisible(false);

    //    ui->radioButtonOrcValido->setChecked(true);
    //    on_radioButtonOrcValido_clicked();
  }
}

bool MainWindow::dbConnect() {
  if (not QSqlDatabase::drivers().contains("QMYSQL")) {
    QMessageBox::critical(this, "Não foi possível carregar o banco de dados.",
                          "Este aplicativo requer o driver QMYSQL.");
    exit(1);
  }

  QSqlDatabase db = QSqlDatabase::contains() ? QSqlDatabase::database() : QSqlDatabase::addDatabase("QMYSQL");

  db.setHostName(hostname);
  db.setUserName(username);
  db.setPassword(password);
  db.setDatabaseName("mysql");

  db.setConnectOptions("CLIENT_COMPRESS=1;MYSQL_OPT_RECONNECT=1");

  if (not db.open()) {
    QString message;

    switch (db.lastError().number()) {
      case 1045:
        message = "Verifique se o usuário e senha do banco de dados estão corretos.";
        break;
      case 2002:
        message = "Verifique se o servidor está ligado, e acessível pela rede.";
        break;
      case 2003:
        message = "Verifique se o servidor está ligado, e acessível pela rede.";
        break;
      case 2005:
        message = "Verifique se o IP do servidor foi escrito corretamente.";
        break;
      default:
        message = "Erro conectando no banco de dados: " + db.lastError().text();
        break;
    }

    QMessageBox::critical(this, "Erro: Banco de dados inacessível!", message);

    return false;
  }

  QSqlQuery query = db.exec("SHOW SCHEMAS");
  bool hasMydb = false;

  while (query.next()) {
    if (query.value(0).toString() == "mydb") {
      hasMydb = true;
    }
  }

  if (not hasMydb) {
    QMessageBox::critical(
          this, "Erro!",
          "Não encontrou as tabelas do bando de dados, verifique se o servidor está funcionando corretamente.");
    return false;
  }

  db.close();

  db.setDatabaseName(homologacao ? "mydb_test" : "mydb");

  if (not db.open()) {
    QMessageBox::critical(this, "Erro", "Erro conectando no banco de dados: " + db.lastError().text());
    return false;
  }

  if (not query.exec("CALL invalidate_expired()")) {
    QMessageBox::critical(this, "Erro!", "Erro executando InvalidarExpirados: " + query.lastError().text());
    return false;
  }

  return true;
}

MainWindow::~MainWindow() {
  delete ui;
  UserSession::free();
}

void MainWindow::on_actionCriarOrcamento_triggered() {
  Orcamento *orcamento = new Orcamento(this);
  orcamento->show();
}

void MainWindow::setPort(const QString &value) { port = value; }

void MainWindow::setPassword(const QString &value) { password = value; }

void MainWindow::setUsername(const QString &value) { username = value; }

void MainWindow::setHostname(const QString &value) { hostname = value; }

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

void MainWindow::readSettings() {
  hostname = settings("Login/hostname").toString();
  username = settings("Login/username").toString();
  password = settings("Login/password").toString();
  port = settings("Login/port").toString();
  homologacao = settings("Login/homologacao").toBool();
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

void MainWindow::setHomologacao(bool value) { homologacao = value; }

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
