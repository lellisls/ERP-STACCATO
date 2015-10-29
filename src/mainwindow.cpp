#include <QShortcut>
#include <QSettings>
#include <QStyleFactory>
#include <QFileDialog>
#include <QSqlRecord>
#include <QDate>
#include <QSqlDriver>
#include <QTimer>
#include <xlsxdocument.h>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "orcamentoproxymodel.h"
#include "cadastrocliente.h"
#include "cadastrofornecedor.h"
#include "cadastroloja.h"
#include "cadastroproduto.h"
#include "cadastroprofissional.h"
#include "cadastrotransportadora.h"
#include "cadastrousuario.h"
#include "checkboxdelegate.h"
#include "comboboxdelegate.h"
#include "contasapagar.h"
#include "contasareceber.h"
#include "doubledelegate.h"
#include "entregascliente.h"
#include "estoque.h"
#include "importaprodutos.h"
#include "importarxml.h"
#include "initdb.h"
#include "inputdialog.h"
#include "logindialog.h"
#include "orcamento.h"
#include "porcentagemdelegate.h"
#include "produtospendentes.h"
#include "usersession.h"
#include "venda.h"
#include "xml.h"
#include "xml_viewer.h"
#include "sendmail.h"

#include "QSimpleUpdater"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  qApp->setApplicationVersion("0.1");
  qDebug() << "version: " << qApp->applicationVersion();

  QSimpleUpdater *updater = new QSimpleUpdater(this);
  updater->setApplicationVersion(qApp->applicationVersion());
  updater->setReferenceUrl("http://192.168.2.144/versao.txt");
  updater->setDownloadUrl("http://192.168.2.144/Loja.exe");
//  connect(updater, &QSimpleUpdater::checkingFinished, this, &MainWindow::on_actionCadastrarCliente_triggered);
  updater->setSilent(true);
  updater->setShowNewestVersionMessage(true);
  updater->checkForUpdates();
  QSettings settings("ERP", "Staccato");
  settings.beginGroup("Login");
  hostname = settings.value("hostname").toString();
  username = settings.value("username").toString();
  password = settings.value("password").toString();
  port = settings.value("port").toString();

#ifdef QT_DEBUG
  //  if (not dbConnect()) {
  //    exit(1);
  //  } else if (not UserSession::login("admin", "1234")) {
  //    QMessageBox::critical(this, "Atenção!", "Login inválido!", QMessageBox::Ok, QMessageBox::NoButton);
  //    exit(1);
  //  }

  LoginDialog *dialog = new LoginDialog(this);

  if (dialog->exec() == QDialog::Rejected) {
    exit(1);
  }
#else
  LoginDialog *dialog = new LoginDialog(this);

  if (dialog->exec() == QDialog::Rejected) {
    exit(1);
  }
  //  if (not dbConnect()) {
  //    exit(1);
  //  } else if (not UserSession::login("admin", "1234")) {
  //    QMessageBox::critical(this, "Atenção!", "Login inválido!", QMessageBox::Ok, QMessageBox::NoButton);
  //    exit(1);
  //  }
#endif

  ui->splitter_5->setStretchFactor(0, 0);
  ui->splitter_5->setStretchFactor(1, 1);

  ui->splitter_6->setStretchFactor(0, 0);
  ui->splitter_6->setStretchFactor(1, 1);

  setWindowIcon(QIcon("Staccato.ico"));

  //  darkTheme();

  setWindowTitle("ERP Staccato");
  readSettings();

  // TODO: instead of auto updating (disrupt user selections) put a button for updating, so the user can update when he
  // wants

  QShortcut *shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this);
  connect(shortcut, &QShortcut::activated, this, &QWidget::close);

  setupTables();

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
    ui->tableContasPagar->hide();
    ui->labelContasPagar->hide();
    ui->tableContasReceber->hide();
    ui->labelContasReceber->hide();
    ui->tableEntregasCliente->hide();
    ui->labelEntregasCliente->hide();
    //    ui->tableRecebimentosFornecedor->hide();
    //    ui->labelRecebimentosFornecedor->hide();
    ui->tableNfeSaida->hide();
    ui->labelNfeSaida->hide();
    ui->tabWidget->setTabEnabled(2, false);
    ui->tabWidget->setTabEnabled(3, false);
    ui->tabWidget->setTabEnabled(4, false);
    ui->tabWidget->setTabEnabled(5, false);
    ui->tabWidget->setTabEnabled(6, false);
    // ui->tablePedidosCompra->hide();
    ui->labelPedidosCompra->hide();
    ui->actionCadastrarUsuario->setVisible(false);

    ui->radioButtonOrcValido->setChecked(true);
    on_radioButtonOrcValido_clicked();
  }

  ui->radioButtonProdPendPend->click();

  updateTables();

  // TODO: sort directly in setupTables
  //  ui->tableOrcamentos->sortByColumn(modelOrcamento->fieldIndex("Código"));
  //  ui->tableVendas->sortByColumn(modelVendas->fieldIndex("idVenda"));
}

bool MainWindow::dbConnect() {
  if (not QSqlDatabase::drivers().contains("QMYSQL")) {
    QMessageBox::critical(this, "Não foi possível carregar o banco de dados.",
                          "Este aplicativo requer o driver QMYSQL.");
    exit(1);
  }

  QSqlDatabase db;

  if (QSqlDatabase::contains()) {
    db = QSqlDatabase::database();
  } else {
    db = QSqlDatabase::addDatabase("QMYSQL");
  }

  db.setHostName(hostname);
  db.setUserName(username);
  db.setPassword(password);
  db.setDatabaseName("mysql");

  if (db.open()) {
    QSqlQuery query = db.exec("SHOW SCHEMAS");
    bool hasMydb = false;

    while (query.next()) {
      if (query.value(0).toString() == "mydb") {
        hasMydb = true;
      }
    }

    if (not hasMydb) {
      QMessageBox::warning(
            this, "Aviso!",
            "Não encontrou as tabelas do bando de dados, verifique se o servidor está funcionando corretamente.");
      return false;
    }

    db.close();

    if (not homologacao) {
      db.setDatabaseName("mydb");
    } else {
      db.setDatabaseName("mydb_test");
    }

    if (db.open()) {
      QSqlQuery query;

      if (not query.exec("CALL invalidate_expired()")) {
        qDebug() << "Erro executando procedure InvalidateExpired: " << query.lastError();
      }

      return true;

    } else {
      showError(db.lastError());
      return false;
    }

  } else {
    switch (db.lastError().number()) {
      case 1045:
        QMessageBox::critical(this, "ERRO: Banco de dados inacessível!",
                              "Verifique se o usuário e senha do banco de dados estão corretos.");
        break;
      case 2002:
        QMessageBox::critical(this, "ERRO: Banco de dados inacessível!",
                              "Verifique se o servidor está ligado, e acessível pela rede.");
        break;
      case 2003:
        QMessageBox::critical(this, "ERRO: Banco de dados inacessível!",
                              "Verifique se o servidor está ligado, e acessível pela rede.");
        break;
      case 2005:
        QMessageBox::critical(this, "ERRO: Banco de dados inacessível!",
                              "Verifique se o IP do servidor foi escrito corretamente.");
        break;
      default:
        showError(db.lastError());
        break;
    }

    return false;
  }

  return false;
}

MainWindow::~MainWindow() {
  delete ui;
  UserSession::free();
}

void MainWindow::showError(const QSqlError &err) {
  QMessageBox::critical(this, "Erro", "Erro inicializando o banco de dados: " + err.text());
}

void MainWindow::on_actionCriarOrcamento_triggered() {
  Orcamento *orcamento = new Orcamento(this);
  orcamento->show();
}

QString MainWindow::getPort() const { return port; }

void MainWindow::setPort(const QString &value) { port = value; }

QString MainWindow::getPassword() const { return password; }

void MainWindow::setPassword(const QString &value) { password = value; }

QString MainWindow::getUsername() const { return username; }

void MainWindow::setUsername(const QString &value) { username = value; }

QString MainWindow::getHostname() const { return hostname; }

void MainWindow::setHostname(const QString &value) { hostname = value; }

void MainWindow::on_actionCadastrarProdutos_triggered() {
  CadastroProduto *cad = new CadastroProduto(this);
  cad->show();
}

void MainWindow::on_actionCadastrarCliente_triggered() {
  CadastroCliente *cad = new CadastroCliente(this);
  cad->show();
}

void MainWindow::setupTables() {
  DoubleDelegate *doubledelegate = new DoubleDelegate(this);

  // Orçamentos --------------------------------------------------------------------------------------------------------
  modelOrcamento = new SqlTableModel(this);
  modelOrcamento->setTable("view_orcamento");

  if (not modelOrcamento->select()) {
    qDebug() << "Failed to populate TableOrcamento: " << modelOrcamento->lastError();
    return;
  }

  ui->tableOrcamentos->setModel(
        new OrcamentoProxyModel(modelOrcamento, modelOrcamento->fieldIndex("Dias restantes"), this));
  ui->tableOrcamentos->setItemDelegate(doubledelegate);
  ui->tableOrcamentos->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableOrcamentos->horizontalHeader()->setResizeContentsPrecision(0);

  // Vendas ------------------------------------------------------------------------------------------------------------
  modelVendas = new SqlTableModel(this);
  modelVendas->setTable("view_venda");
  modelVendas->setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelVendas->setSort(modelVendas->fieldIndex("Dias restantes"), Qt::DescendingOrder);

  if (not modelVendas->select()) {
    qDebug() << "Failed to populate TableVendas: " << modelVendas->lastError();
    return;
  }

  ui->tableVendas->setModel(new OrcamentoProxyModel(modelVendas, modelVendas->fieldIndex("Dias restantes"), this));
  ui->tableVendas->setItemDelegateForColumn(modelVendas->fieldIndex("Bruto"), doubledelegate);
  ui->tableVendas->setItemDelegateForColumn(modelVendas->fieldIndex("Líquido"), doubledelegate);
  ui->tableVendas->setItemDelegateForColumn(modelVendas->fieldIndex("Frete"), doubledelegate);
  ui->tableVendas->setItemDelegateForColumn(modelVendas->fieldIndex("Total R$"), doubledelegate);
  ui->tableVendas->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableVendas->horizontalHeader()->setResizeContentsPrecision(0);

  // Produtos Pendentes ------------------------------------------------------------------------------------------------
  modelProdPend = new QSqlQueryModel(this);

  // TODO: convert this to view to be able to set filters properly
  // model is set by the filter buttons

  ui->tableProdutosPend->setModel(modelProdPend);
  ui->tableProdutosPend->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableProdutosPend->horizontalHeader()->setResizeContentsPrecision(0);

  // Fornecedores Compras ----------------------------------------------------------------------------------------------
  modelPedForn = new QSqlQueryModel(this);
  modelPedForn->setQuery("SELECT fornecedor, COUNT(fornecedor) FROM pedido_fornecedor_has_produto WHERE status != "
                         "'FINALIZADO' GROUP BY fornecedor");
  modelPedForn->setHeaderData(modelPedForn->record().indexOf("fornecedor"), Qt::Horizontal, "Fornecedor");
  modelPedForn->setHeaderData(modelPedForn->record().indexOf("COUNT(fornecedor)"), Qt::Horizontal, "Itens");

  ui->tableFornCompras->setModel(modelPedForn);
  ui->tableFornCompras->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableFornCompras->horizontalHeader()->setResizeContentsPrecision(0);

  // Fornecedores Logística --------------------------------------------------------------------------------------------
  modelPedForn2 = new QSqlQueryModel(this);
  modelPedForn2->setQuery("SELECT fornecedor, COUNT(fornecedor) FROM pedido_fornecedor_has_produto WHERE status != "
                          "'FINALIZADO' GROUP BY fornecedor");
  modelPedForn2->setHeaderData(modelPedForn2->record().indexOf("fornecedor"), Qt::Horizontal, "Fornecedor");
  modelPedForn2->setHeaderData(modelPedForn2->record().indexOf("COUNT(fornecedor)"), Qt::Horizontal, "Itens");

  ui->tableFornLogistica->setModel(modelPedForn2);
  ui->tableFornLogistica->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableFornLogistica->horizontalHeader()->setResizeContentsPrecision(0);

  // Compras - Pendentes -----------------------------------------------------------------------------------------------
  modelItemPedidosPend = new SqlTableModel(this);
  modelItemPedidosPend->setTable("pedido_fornecedor_has_produto");
  modelItemPedidosPend->setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelItemPedidosPend->setHeaderData(modelItemPedidosPend->fieldIndex("selecionado"), Qt::Horizontal, "");
  modelItemPedidosPend->setHeaderData(modelItemPedidosPend->fieldIndex("fornecedor"), Qt::Horizontal, "Fornecedor");
  modelItemPedidosPend->setHeaderData(modelItemPedidosPend->fieldIndex("descricao"), Qt::Horizontal, "Descrição");
  modelItemPedidosPend->setHeaderData(modelItemPedidosPend->fieldIndex("colecao"), Qt::Horizontal, "Coleção");
  modelItemPedidosPend->setHeaderData(modelItemPedidosPend->fieldIndex("quant"), Qt::Horizontal, "Quant.");
  modelItemPedidosPend->setHeaderData(modelItemPedidosPend->fieldIndex("un"), Qt::Horizontal, "Un.");
  modelItemPedidosPend->setHeaderData(modelItemPedidosPend->fieldIndex("preco"), Qt::Horizontal, "Preço");
  modelItemPedidosPend->setHeaderData(modelItemPedidosPend->fieldIndex("formComercial"), Qt::Horizontal, "Form. Com.");
  modelItemPedidosPend->setHeaderData(modelItemPedidosPend->fieldIndex("codComercial"), Qt::Horizontal, "Cód. Com.");
  modelItemPedidosPend->setHeaderData(modelItemPedidosPend->fieldIndex("codBarras"), Qt::Horizontal, "Cód. Bar.");
  modelItemPedidosPend->setHeaderData(modelItemPedidosPend->fieldIndex("idCompra"), Qt::Horizontal, "Compra");
  modelItemPedidosPend->setHeaderData(modelItemPedidosPend->fieldIndex("dataPrevCompra"), Qt::Horizontal,
                                      "Prev. Compra");
  modelItemPedidosPend->setHeaderData(modelItemPedidosPend->fieldIndex("dataCompra"), Qt::Horizontal, "Data Compra");
  modelItemPedidosPend->setHeaderData(modelItemPedidosPend->fieldIndex("status"), Qt::Horizontal, "Status");

  modelItemPedidosPend->setFilter("status = 'PENDENTE'");

  if (not modelItemPedidosPend->select()) {
    qDebug() << "Failed to populate pedido_fornecedor_has_produto pend: " << modelItemPedidosPend->lastError();
    return;
  }

  ui->tablePedidosPend->setModel(modelItemPedidosPend);
  ui->tablePedidosPend->hideColumn(modelItemPedidosPend->fieldIndex("idPedido"));
  ui->tablePedidosPend->hideColumn(modelItemPedidosPend->fieldIndex("idLoja"));
  ui->tablePedidosPend->hideColumn(modelItemPedidosPend->fieldIndex("item"));
  ui->tablePedidosPend->hideColumn(modelItemPedidosPend->fieldIndex("idProduto"));
  ui->tablePedidosPend->hideColumn(modelItemPedidosPend->fieldIndex("prcUnitario"));
  ui->tablePedidosPend->hideColumn(modelItemPedidosPend->fieldIndex("parcial"));
  ui->tablePedidosPend->hideColumn(modelItemPedidosPend->fieldIndex("desconto"));
  ui->tablePedidosPend->hideColumn(modelItemPedidosPend->fieldIndex("parcialDesc"));
  ui->tablePedidosPend->hideColumn(modelItemPedidosPend->fieldIndex("descGlobal"));
  ui->tablePedidosPend->hideColumn(modelItemPedidosPend->fieldIndex("dataRealCompra"));
  ui->tablePedidosPend->hideColumn(modelItemPedidosPend->fieldIndex("dataPrevConf"));
  ui->tablePedidosPend->hideColumn(modelItemPedidosPend->fieldIndex("dataRealConf"));
  ui->tablePedidosPend->hideColumn(modelItemPedidosPend->fieldIndex("dataPrevFat"));
  ui->tablePedidosPend->hideColumn(modelItemPedidosPend->fieldIndex("dataRealFat"));
  ui->tablePedidosPend->hideColumn(modelItemPedidosPend->fieldIndex("dataPrevColeta"));
  ui->tablePedidosPend->hideColumn(modelItemPedidosPend->fieldIndex("dataRealColeta"));
  ui->tablePedidosPend->hideColumn(modelItemPedidosPend->fieldIndex("dataPrevEnt"));
  ui->tablePedidosPend->hideColumn(modelItemPedidosPend->fieldIndex("dataRealEnt"));
  ui->tablePedidosPend->hideColumn(modelItemPedidosPend->fieldIndex("dataPrevReceb"));
  ui->tablePedidosPend->hideColumn(modelItemPedidosPend->fieldIndex("dataRealReceb"));
  ui->tablePedidosPend->horizontalHeader()->setResizeContentsPrecision(0);
  ui->tablePedidosPend->verticalHeader()->setResizeContentsPrecision(0);

  ui->tablePedidosPend->setItemDelegateForColumn(modelItemPedidosPend->fieldIndex("selecionado"),
                                                 new CheckBoxDelegate(this));

  // Compras - A Confirmar ---------------------------------------------------------------------------------------------
  modelItemPedidosComp = new QSqlQueryModel(this);
  modelItemPedidosComp->setQuery("SELECT fornecedor, idCompra, COUNT(idProduto), SUM(preco), status FROM "
                                 "pedido_fornecedor_has_produto WHERE status = 'EM COMPRA' GROUP BY idCompra");

  modelItemPedidosComp->setHeaderData(modelItemPedidosComp->record().indexOf("fornecedor"), Qt::Horizontal,
                                      "Fornecedor");
  modelItemPedidosComp->setHeaderData(modelItemPedidosComp->record().indexOf("idCompra"), Qt::Horizontal, "Compra");
  modelItemPedidosComp->setHeaderData(modelItemPedidosComp->record().indexOf("COUNT(idProduto)"), Qt::Horizontal,
                                      "Itens");
  modelItemPedidosComp->setHeaderData(modelItemPedidosComp->record().indexOf("SUM(preco)"), Qt::Horizontal, "Preço");
  modelItemPedidosComp->setHeaderData(modelItemPedidosComp->record().indexOf("status"), Qt::Horizontal, "Status");

  ui->tablePedidosComp->setModel(modelItemPedidosComp);
  ui->tablePedidosComp->horizontalHeader()->setResizeContentsPrecision(0);
  ui->tablePedidosComp->verticalHeader()->setResizeContentsPrecision(0);

  // Faturamentos ------------------------------------------------------------------------------------------------------
  modelFat = new QSqlQueryModel(this);
  modelFat->setQuery("SELECT fornecedor, idCompra, COUNT(idProduto), SUM(preco), status FROM "
                     "pedido_fornecedor_has_produto WHERE status = 'EM FATURAMENTO' GROUP BY idCompra");

  modelFat->setHeaderData(modelFat->record().indexOf("fornecedor"), Qt::Horizontal, "Fornecedor");
  modelFat->setHeaderData(modelFat->record().indexOf("idCompra"), Qt::Horizontal, "Compra");
  modelFat->setHeaderData(modelFat->record().indexOf("COUNT(idProduto)"), Qt::Horizontal, "Itens");
  modelFat->setHeaderData(modelFat->record().indexOf("SUM(preco)"), Qt::Horizontal, "Preço");
  modelFat->setHeaderData(modelFat->record().indexOf("status"), Qt::Horizontal, "Status");

  ui->tableFaturamento->setModel(modelFat);
  ui->tableFaturamento->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableFaturamento->horizontalHeader()->setResizeContentsPrecision(0);

  // Coletas -----------------------------------------------------------------------------------------------------------
  modelColeta = new SqlTableModel(this);
  modelColeta->setTable("pedido_fornecedor_has_produto");
  modelColeta->setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelColeta->setHeaderData(modelColeta->fieldIndex("selecionado"), Qt::Horizontal, "");
  modelColeta->setHeaderData(modelColeta->fieldIndex("fornecedor"), Qt::Horizontal, "Fornecedor");
  modelColeta->setHeaderData(modelColeta->fieldIndex("descricao"), Qt::Horizontal, "Descrição");
  modelColeta->setHeaderData(modelColeta->fieldIndex("colecao"), Qt::Horizontal, "Coleção");
  modelColeta->setHeaderData(modelColeta->fieldIndex("quant"), Qt::Horizontal, "Quant.");
  modelColeta->setHeaderData(modelColeta->fieldIndex("un"), Qt::Horizontal, "Un.");
  modelColeta->setHeaderData(modelColeta->fieldIndex("preco"), Qt::Horizontal, "Preço");
  modelColeta->setHeaderData(modelColeta->fieldIndex("formComercial"), Qt::Horizontal, "Form. Com.");
  modelColeta->setHeaderData(modelColeta->fieldIndex("codComercial"), Qt::Horizontal, "Cód. Com.");
  modelColeta->setHeaderData(modelColeta->fieldIndex("codBarras"), Qt::Horizontal, "Cód. Bar.");
  modelColeta->setHeaderData(modelColeta->fieldIndex("idCompra"), Qt::Horizontal, "Compra");
  modelColeta->setHeaderData(modelColeta->fieldIndex("dataRealFat"), Qt::Horizontal, "Data Fat.");
  modelColeta->setHeaderData(modelColeta->fieldIndex("dataPrevColeta"), Qt::Horizontal, "Prev. Coleta");
  modelColeta->setHeaderData(modelColeta->fieldIndex("status"), Qt::Horizontal, "Status");

  modelColeta->setFilter("status = 'EM COLETA'");

  if (not modelColeta->select()) {
    qDebug() << "Failed to populate pedido_fornecedor_has_produto coleta: " << modelColeta->lastError();
  }

  ui->tableColeta->setModel(modelColeta);
  ui->tableColeta->setItemDelegateForColumn(modelColeta->fieldIndex("status"), new ComboBoxDelegate(this));
  ui->tableColeta->hideColumn(modelColeta->fieldIndex("idPedido"));
  ui->tableColeta->hideColumn(modelColeta->fieldIndex("idLoja"));
  ui->tableColeta->hideColumn(modelColeta->fieldIndex("item"));
  ui->tableColeta->hideColumn(modelColeta->fieldIndex("idProduto"));
  ui->tableColeta->hideColumn(modelColeta->fieldIndex("prcUnitario"));
  ui->tableColeta->hideColumn(modelColeta->fieldIndex("parcial"));
  ui->tableColeta->hideColumn(modelColeta->fieldIndex("desconto"));
  ui->tableColeta->hideColumn(modelColeta->fieldIndex("parcialDesc"));
  ui->tableColeta->hideColumn(modelColeta->fieldIndex("descGlobal"));
  ui->tableColeta->hideColumn(modelColeta->fieldIndex("dataPrevCompra"));
  ui->tableColeta->hideColumn(modelColeta->fieldIndex("dataRealCompra"));
  ui->tableColeta->hideColumn(modelColeta->fieldIndex("dataPrevConf"));
  ui->tableColeta->hideColumn(modelColeta->fieldIndex("dataRealConf"));
  ui->tableColeta->hideColumn(modelColeta->fieldIndex("dataPrevFat"));
  ui->tableColeta->hideColumn(modelColeta->fieldIndex("dataRealColeta"));
  ui->tableColeta->hideColumn(modelColeta->fieldIndex("dataPrevEnt"));
  ui->tableColeta->hideColumn(modelColeta->fieldIndex("dataRealEnt"));
  ui->tableColeta->hideColumn(modelColeta->fieldIndex("dataPrevReceb"));
  ui->tableColeta->hideColumn(modelColeta->fieldIndex("dataRealReceb"));
  ui->tableColeta->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableColeta->horizontalHeader()->setResizeContentsPrecision(0);

  ui->tableColeta->setItemDelegateForColumn(modelColeta->fieldIndex("selecionado"), new CheckBoxDelegate(this));

  // Recebimentos fornecedor -------------------------------------------------------------------------------------------
  modelReceb = new SqlTableModel(this);
  modelReceb->setTable("pedido_fornecedor_has_produto");
  modelReceb->setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelReceb->setHeaderData(modelReceb->fieldIndex("selecionado"), Qt::Horizontal, "");
  modelReceb->setHeaderData(modelReceb->fieldIndex("fornecedor"), Qt::Horizontal, "Fornecedor");
  modelReceb->setHeaderData(modelReceb->fieldIndex("descricao"), Qt::Horizontal, "Descrição");
  modelReceb->setHeaderData(modelReceb->fieldIndex("colecao"), Qt::Horizontal, "Coleção");
  modelReceb->setHeaderData(modelReceb->fieldIndex("quant"), Qt::Horizontal, "Quant.");
  modelReceb->setHeaderData(modelReceb->fieldIndex("un"), Qt::Horizontal, "Un.");
  modelReceb->setHeaderData(modelReceb->fieldIndex("preco"), Qt::Horizontal, "Preço");
  modelReceb->setHeaderData(modelReceb->fieldIndex("formComercial"), Qt::Horizontal, "Form. Com.");
  modelReceb->setHeaderData(modelReceb->fieldIndex("codComercial"), Qt::Horizontal, "Cód. Com.");
  modelReceb->setHeaderData(modelReceb->fieldIndex("codBarras"), Qt::Horizontal, "Cód. Bar.");
  modelReceb->setHeaderData(modelReceb->fieldIndex("idCompra"), Qt::Horizontal, "Compra");
  modelReceb->setHeaderData(modelReceb->fieldIndex("dataRealColeta"), Qt::Horizontal, "Data Coleta");
  modelReceb->setHeaderData(modelReceb->fieldIndex("dataPrevReceb"), Qt::Horizontal, "Prev. Receb.");
  modelReceb->setHeaderData(modelReceb->fieldIndex("status"), Qt::Horizontal, "Status");

  modelReceb->setFilter("status = 'EM RECEBIMENTO'");

  if (not modelReceb->select()) {
    qDebug() << "Failed to populate pedido_fornecedor_has_produto: " << modelReceb->lastError();
  }

  ui->tableRecebimento->setModel(modelReceb);
  ui->tableRecebimento->setItemDelegateForColumn(modelReceb->fieldIndex("status"), new ComboBoxDelegate(this));
  ui->tableRecebimento->hideColumn(modelReceb->fieldIndex("idPedido"));
  ui->tableRecebimento->hideColumn(modelReceb->fieldIndex("idLoja"));
  ui->tableRecebimento->hideColumn(modelReceb->fieldIndex("item"));
  ui->tableRecebimento->hideColumn(modelReceb->fieldIndex("idProduto"));
  ui->tableRecebimento->hideColumn(modelReceb->fieldIndex("prcUnitario"));
  ui->tableRecebimento->hideColumn(modelReceb->fieldIndex("parcial"));
  ui->tableRecebimento->hideColumn(modelReceb->fieldIndex("desconto"));
  ui->tableRecebimento->hideColumn(modelReceb->fieldIndex("parcialDesc"));
  ui->tableRecebimento->hideColumn(modelReceb->fieldIndex("descGlobal"));
  ui->tableRecebimento->hideColumn(modelReceb->fieldIndex("dataPrevCompra"));
  ui->tableRecebimento->hideColumn(modelReceb->fieldIndex("dataRealCompra"));
  ui->tableRecebimento->hideColumn(modelReceb->fieldIndex("dataPrevConf"));
  ui->tableRecebimento->hideColumn(modelReceb->fieldIndex("dataRealConf"));
  ui->tableRecebimento->hideColumn(modelReceb->fieldIndex("dataPrevFat"));
  ui->tableRecebimento->hideColumn(modelReceb->fieldIndex("dataRealFat"));
  ui->tableRecebimento->hideColumn(modelReceb->fieldIndex("dataPrevEnt"));
  ui->tableRecebimento->hideColumn(modelReceb->fieldIndex("dataRealEnt"));
  ui->tableRecebimento->hideColumn(modelReceb->fieldIndex("dataPrevColeta"));
  ui->tableRecebimento->hideColumn(modelReceb->fieldIndex("dataRealReceb"));
  ui->tableRecebimento->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableRecebimento->horizontalHeader()->setResizeContentsPrecision(0);

  ui->tableRecebimento->setItemDelegateForColumn(modelReceb->fieldIndex("selecionado"), new CheckBoxDelegate(this));

  // Entregas cliente --------------------------------------------------------------------------------------------------
  modelEntregasCliente = new SqlTableModel(this);
  modelEntregasCliente->setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelEntregasCliente->setTable("view_venda");

  if (not modelEntregasCliente->select()) {
    qDebug() << "Failed to populate TableEntregasCliente: " << modelEntregasCliente->lastError();
    return;
  }

  ui->tableEntregasCliente->setModel(modelEntregasCliente);
  ui->tableEntregasCliente->setItemDelegate(doubledelegate);
  ui->tableEntregasCliente->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableEntregasCliente->horizontalHeader()->setResizeContentsPrecision(0);

  // NFe Entrada -------------------------------------------------------------------------------------------------------
  modelNfeEntrada = new SqlTableModel(this);
  modelNfeEntrada->setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelNfeEntrada->setTable("nfe");
  modelNfeEntrada->setFilter("tipo = 'ENTRADA'");

  if (not modelNfeEntrada->select()) {
    qDebug() << "Failed to populate TableNFe: " << modelNfeEntrada->lastError();
    return;
  }

  ui->tableNfeEntrada->setModel(modelNfeEntrada);
  ui->tableNfeEntrada->setColumnHidden(modelNfeEntrada->fieldIndex("NFe"), true);
  ui->tableNfeEntrada->setItemDelegate(doubledelegate);
  ui->tableNfeEntrada->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableNfeEntrada->horizontalHeader()->setResizeContentsPrecision(0);

  // NFe Saida ---------------------------------------------------------------------------------------------------------
  modelNfeSaida = new SqlTableModel(this);
  modelNfeSaida->setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelNfeSaida->setTable("nfe");
  modelNfeSaida->setFilter("tipo = 'SAIDA'");

  if (not modelNfeSaida->select()) {
    qDebug() << "Failed to populate TableNFe: " << modelNfeSaida->lastError();
    return;
  }

  ui->tableNfeSaida->setModel(modelNfeSaida);
  ui->tableNfeSaida->setColumnHidden(modelNfeSaida->fieldIndex("NFe"), true);
  ui->tableNfeSaida->setItemDelegate(doubledelegate);
  ui->tableNfeSaida->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableNfeSaida->horizontalHeader()->setResizeContentsPrecision(0);

  // Estoque -----------------------------------------------------------------------------------------------------------
  modelEstoque = new SqlTableModel(this);
  modelEstoque->setTable("view_estoque");
  modelEstoque->setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelEstoque->select()) {
    qDebug() << "Failed to populate TableEstoque: " << modelEstoque->lastError();
    return;
  }

  ui->tableEstoque->setModel(modelEstoque);
  ui->tableEstoque->setItemDelegate(doubledelegate);
  ui->tableEstoque->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableEstoque->horizontalHeader()->setResizeContentsPrecision(0);

  // Contas a pagar ----------------------------------------------------------------------------------------------------
  modelCAPagar = new SqlTableModel(this);
  modelCAPagar->setTable("conta_a_pagar");
  modelCAPagar->setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelCAPagar->select()) {
    qDebug() << "Failed to populate TableContasPagar: " << modelCAPagar->lastError();
    return;
  }

  ui->tableContasPagar->setModel(modelCAPagar);
  ui->tableContasPagar->setItemDelegate(doubledelegate);
  ui->tableContasPagar->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableContasPagar->horizontalHeader()->setResizeContentsPrecision(0);

  // Contas a receber --------------------------------------------------------------------------------------------------
  modelCAReceber = new SqlTableModel(this);
  modelCAReceber->setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelCAReceber->setTable("conta_a_receber");

  if (not modelCAReceber->select()) {
    qDebug() << "Failed to populate TableContasReceber: " << modelCAReceber->lastError();
    return;
  }

  ui->tableContasReceber->setModel(modelCAReceber);
  ui->tableContasReceber->setItemDelegate(doubledelegate);
  ui->tableContasReceber->verticalHeader()->setResizeContentsPrecision(0);
  ui->tableContasReceber->horizontalHeader()->setResizeContentsPrecision(0);
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
  QSqlQuery query;

  if (not query.exec("CALL update_venda_status()")) {
    qDebug() << "Erro atualizando status das vendas: " << query.lastError();
  }

  if (not modelCAPagar->select()) {
    qDebug() << "erro modelCAPagar: " << modelCAPagar->lastError();
    return;
  }

  if (not modelCAReceber->select()) {
    qDebug() << "erro modelCAReceber: " << modelCAReceber->lastError();
    return;
  }

  if (not modelOrcamento->select()) {
    qDebug() << "erro modelOrcamento: " << modelOrcamento->lastError();
    return;
  }

  if (not modelVendas->select()) {
    qDebug() << "erro modelVendas: " << modelVendas->lastError();
    return;
  }

  if (not modelEntregasCliente->select()) {
    qDebug() << "erro modelEntregasCliente: " << modelEntregasCliente->lastError();
    return;
  }

  if (not modelNfeSaida->select()) {
    qDebug() << "erro modelNFe: " << modelNfeSaida->lastError();
    return;
  }

  if (not modelEstoque->select()) {
    qDebug() << "erro modelEstoque: " << modelEstoque->lastError();
    return;
  }

  if (not modelColeta->select()) {
    qDebug() << "Erro modelColeta: " << modelColeta->lastError();
  }

  if (not modelReceb->select()) {
    qDebug() << "Erro modelReceb: " << modelReceb->lastError();
  }

  if (not modelItemPedidosPend->select()) {
    qDebug() << "Erro modelItemPedidosPend: " << modelItemPedidosPend->lastError();
  }

  modelItemPedidosComp->setQuery(modelItemPedidosComp->query().executedQuery());
  modelPedForn->setQuery(modelPedForn->query().executedQuery());
  modelPedForn2->setQuery(modelPedForn2->query().executedQuery());
  modelProdPend->setQuery(modelProdPend->query().executedQuery());
  modelFat->setQuery(modelFat->query().executedQuery());

  for (int i = 0; i < modelItemPedidosPend->rowCount(); ++i) {
    ui->tablePedidosPend->openPersistentEditor(
          modelItemPedidosPend->index(i, modelItemPedidosPend->fieldIndex("selecionado")));
  }

  for (int i = 0; i < modelColeta->rowCount(); ++i) {
    ui->tableColeta->openPersistentEditor(modelColeta->index(i, modelColeta->fieldIndex("selecionado")));
  }

  for (int i = 0; i < modelReceb->rowCount(); ++i) {
    ui->tableRecebimento->openPersistentEditor(modelReceb->index(i, modelReceb->fieldIndex("selecionado")));
  }

  ui->tableColeta->resizeColumnsToContents();
  ui->tableContasPagar->resizeColumnsToContents();
  ui->tableContasReceber->resizeColumnsToContents();
  ui->tableEntregasCliente->resizeColumnsToContents();
  ui->tableEstoque->resizeColumnsToContents();
  ui->tableFaturamento->resizeColumnsToContents();
  ui->tableNfeSaida->resizeColumnsToContents();
  ui->tableOrcamentos->resizeColumnsToContents();
  ui->tablePedidosComp->resizeColumnsToContents();
  ui->tableFornCompras->resizeColumnsToContents();
  ui->tablePedidosPend->resizeColumnsToContents();
  ui->tableProdutosPend->resizeColumnsToContents();
  ui->tableRecebimento->resizeColumnsToContents();
  ui->tableVendas->resizeColumnsToContents();
}

void MainWindow::on_radioButtonOrcValido_clicked() {
  modelOrcamento->setFilter("`Dias restantes` > 0 AND status != 'CANCELADO'");
  ui->tableOrcamentos->resizeColumnsToContents();
}

void MainWindow::on_radioButtonOrcExpirado_clicked() {
  modelOrcamento->setFilter("`Dias restantes` < 1");
  ui->tableOrcamentos->resizeColumnsToContents();
}

void MainWindow::on_radioButtonOrcLimpar_clicked() {
  modelOrcamento->setFilter("");
  ui->tableOrcamentos->resizeColumnsToContents();
}

void MainWindow::on_radioButtonVendAberto_clicked() {
  modelVendas->setFilter("status = 'aberto'");
  ui->tableVendas->resizeColumnsToContents();
}

void MainWindow::on_radioButtonVendFechado_clicked() {
  modelVendas->setFilter("status = 'fechado'");
  ui->tableVendas->resizeColumnsToContents();
}

void MainWindow::on_radioButtonVendLimpar_clicked() {
  modelVendas->setFilter("");
  ui->tableVendas->resizeColumnsToContents();
}

void MainWindow::on_radioButtonNFeAutorizado_clicked() {
  modelNfeSaida->setFilter("status = 'autorizado'");
  ui->tableNfeSaida->resizeColumnsToContents();
}

void MainWindow::on_radioButtonNFeEnviado_clicked() {
  modelNfeSaida->setFilter("status = 'enviado'");
  ui->tableNfeSaida->resizeColumnsToContents();
}

void MainWindow::on_radioButtonNFeLimpar_clicked() {
  modelNfeSaida->setFilter("");
  ui->tableNfeSaida->resizeColumnsToContents();
}

void MainWindow::on_radioButtonEntregaLimpar_clicked() {
  modelEntregasCliente->setFilter("tipo = 'cliente'");
  ui->tableEntregasCliente->resizeColumnsToContents();
}

void MainWindow::on_radioButtonEntregaEnviado_clicked() {
  modelEntregasCliente->setFilter("status = 'enviado' AND tipo = 'cliente'");
  ui->tableEntregasCliente->resizeColumnsToContents();
}

void MainWindow::on_radioButtonEntregaPendente_clicked() {
  modelEntregasCliente->setFilter("status = 'pendente' AND tipo = 'cliente'");
  ui->tableEntregasCliente->resizeColumnsToContents();
}

void MainWindow::on_radioButtonContaPagarLimpar_clicked() {
  modelCAPagar->setFilter("");
  ui->tableContasPagar->resizeColumnsToContents();
}

void MainWindow::on_radioButtonContaPagarPago_clicked() {
  modelCAPagar->setFilter("pago = 'sim'");
  ui->tableContasPagar->resizeColumnsToContents();
}

void MainWindow::on_radioButtonContaPagarPendente_clicked() {
  modelCAPagar->setFilter("pago = 'não'");
  ui->tableContasPagar->resizeColumnsToContents();
}

void MainWindow::on_radioButtonContaReceberLimpar_clicked() {
  modelCAReceber->setFilter("");
  ui->tableContasReceber->resizeColumnsToContents();
}

void MainWindow::on_radioButtonContaReceberRecebido_clicked() {
  modelCAReceber->setFilter("pago = 'sim'");
  ui->tableContasReceber->resizeColumnsToContents();
}

void MainWindow::on_radioButtonContaReceberPendente_clicked() {
  modelCAReceber->setFilter("pago = 'não'");
  ui->tableContasReceber->resizeColumnsToContents();
}

void MainWindow::on_radioButtonOrcProprios_clicked() {
  modelOrcamento->setFilter("idUsuario = " + QString::number(UserSession::getIdUsuario()));
  ui->tableOrcamentos->resizeColumnsToContents();
}

void MainWindow::on_pushButtonCriarOrc_clicked() { on_actionCriarOrcamento_triggered(); }

void MainWindow::on_lineEditBuscaOrcamentos_textChanged(const QString &text) {
  if (text.isEmpty()) {
    modelOrcamento->setFilter("");
  } else {
    modelOrcamento->setFilter("(Código LIKE '%" + text + "%')");
  }
}

void MainWindow::on_lineEditBuscaVendas_textChanged(const QString &text) {
  if (text.isEmpty()) {
    modelVendas->setFilter("");
  } else {
    modelVendas->setFilter("(idVenda LIKE '%" + text + "%') OR (cliente LIKE '%" + text + "%')");
  }
}

void MainWindow::on_lineEditBuscaContasPagar_textChanged(const QString &text) {
  if (text.isEmpty()) {
    modelCAPagar->setFilter("");
  } else {
    modelCAPagar->setFilter("(idVenda LIKE '%" + text + "%') OR (pago LIKE '%" + text + "%')");
  }
}

void MainWindow::on_lineEditBuscaContasReceber_textChanged(const QString &text) {
  if (text.isEmpty()) {
    modelCAReceber->setFilter("");
  } else {
    modelCAReceber->setFilter("(idVenda LIKE '%" + text + "%') OR (pago LIKE '%" + text + "%')");
  }
}

void MainWindow::on_lineEditBuscaEntregas_textChanged(const QString &text) {
  if (text.isEmpty()) {
    modelEntregasCliente->setFilter("");
  } else {
    modelEntregasCliente->setFilter("(idPedido LIKE '%" + text + "%') OR (status LIKE '%" + text + "%')");
  }
}

void MainWindow::on_lineEditBuscaProdutosPend_textChanged(const QString &text) { Q_UNUSED(text); }

void MainWindow::on_lineEditBuscaNFe_textChanged(const QString &text) {
  if (text.isEmpty()) {
    modelNfeSaida->setFilter("");
  } else {
    modelNfeSaida->setFilter("(idVenda LIKE '%" + text + "%') OR (status LIKE '%" + text + "%')");
  }
}

void MainWindow::on_actionCadastrarFornecedor_triggered() {
  CadastroFornecedor *cad = new CadastroFornecedor(this);
  cad->show();
}

void MainWindow::readSettings() {
  QSettings settings("ERP", "Staccato");
  settings.beginGroup("Login");

  if (not settings.contains("hostname")) {
    settings.setValue("hostname", QString("localhost"));
    settings.setValue("username", QString("test"));
    settings.setValue("password", QString("1234"));
    settings.setValue("port", QString("3306"));
  }

  hostname = settings.value("hostname").toString();
  username = settings.value("username").toString();
  password = settings.value("password").toString();
  port = settings.value("port").toString();
  homologacao = settings.value("homologacao").toBool();
  settings.endGroup();
}

void MainWindow::on_actionImportaProdutos_triggered() {
  ImportaProdutos *importa = new ImportaProdutos(this);
  importa->importar();
}

void MainWindow::on_tableVendas_activated(const QModelIndex &index) {
  Venda *vendas = new Venda(this);
  vendas->viewRegisterById(modelVendas->data(index.row(), "idVenda"));
}

void MainWindow::on_tableOrcamentos_activated(const QModelIndex &index) {
  Orcamento *orcamento = new Orcamento(this);
  orcamento->viewRegisterById(modelOrcamento->data(index.row(), "Código"));
  orcamento->show();
}

void MainWindow::on_tableContasPagar_activated(const QModelIndex &index) {
  ContasAPagar *contas = new ContasAPagar(this);
  contas->viewConta(modelCAPagar->data(index.row(), "idVenda").toString());
}

void MainWindow::on_tableContasReceber_activated(const QModelIndex &index) {
  ContasAReceber *contas = new ContasAReceber(this);
  contas->viewConta(modelCAReceber->data(index.row(), "idVenda").toString());
}

void MainWindow::on_tableEntregasCliente_activated(const QModelIndex &index) {
  EntregasCliente *entregas = new EntregasCliente(this);
  entregas->viewEntrega(modelEntregasCliente->data(index.row(), "idVenda").toString());
}

void MainWindow::on_tableFornCompras_activated(const QModelIndex &index) {
  int row = index.row();

  QString fornecedor =
      modelPedForn->data(modelPedForn->index(row, modelPedForn->record().indexOf("fornecedor"))).toString();

  modelItemPedidosPend->setFilter("fornecedor = '" + fornecedor + "' AND status = 'PENDENTE'");

  if (not modelItemPedidosPend->select()) {
    qDebug() << "Error: " << modelItemPedidosPend->lastError();
  }

  // TODO: filter compras e faturamentos?

  updateTables();

  ui->tableFornCompras->selectRow(row);
}

void MainWindow::on_tableNfeSaida_activated(const QModelIndex &index) {
  Venda *vendas = new Venda(this);
  vendas->viewRegisterById(modelNfeSaida->data(index.row(), "idVenda"));
}

bool MainWindow::event(QEvent *e) {
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

#ifdef TEST
bool MainWindow::TestInitDB() { return initDb(); }

bool MainWindow::TestCadastroClienteIncompleto() {
  CadastroCliente *cliente = new CadastroCliente(this);
  return cliente->TestClienteIncompleto();
}

bool MainWindow::TestCadastroClienteEndereco() {
  CadastroCliente *cliente = new CadastroCliente(this);
  return cliente->TestClienteEndereco();
}

bool MainWindow::TestCadastroClienteCompleto() {
  CadastroCliente *cliente = new CadastroCliente(this);
  return cliente->TestClienteCompleto();
}

void MainWindow::TestImportacao() {
  ImportaProdutos *importa = new ImportaProdutos(this);
  importa->TestImportacao();
}
#endif

void MainWindow::on_tableEstoque_activated(const QModelIndex &index) {
  Estoque *estoque = new Estoque(this);
  estoque->viewRegisterById(modelEstoque->data(index.row(), "Cód Com").toString());
}

void MainWindow::on_pushButtonEntradaEstoque_clicked() {
  XML xml;
  xml.importarXML();
  updateTables();
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

  darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
  darkPalette.setColor(QPalette::HighlightedText, Qt::black);

  qApp->setPalette(darkPalette);

  qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
}

void MainWindow::on_tableProdutosPend_activated(const QModelIndex &index) {
  ProdutosPendentes *produtos = new ProdutosPendentes(this);

  QString codComercial =
      modelProdPend->data(modelProdPend->index(index.row(), modelProdPend->record().indexOf("codComercial")))
      .toString();

  QString status =
      modelProdPend->data(modelProdPend->index(index.row(), modelProdPend->record().indexOf("status"))).toString();

  produtos->viewProduto(codComercial, status);
}

void MainWindow::on_pushButtonGerarCompra_clicked() {
  if (UserSession::getFromLoja("pastaCompra").isEmpty()) {
    QMessageBox::warning(
          this, "Aviso!",
          "Não há uma pasta definida para salvar os arquivos Excel, favor definir nas configurações da loja.");
    return;
  }

  if (UserSession::getFromLoja("emailCompra").isEmpty()) {
    QMessageBox::warning(this, "Aviso!",
                         "Não há um email de compras definido, favor cadastrar nas configurações da loja.");
    return;
  }

  if (modelPedForn->rowCount() > 1) {
    if (not modelItemPedidosPend->filter().contains("fornecedor")) {
      QMessageBox::warning(this, "Aviso!", "Selecione o fornecedor na tabela à esquerda.");
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

  if (inputDlg->exec() != InputDialog::Accepted) {
    return;
  }

  for (const auto row : lista) {
    QString produto = modelItemPedidosPend->data(row, "descricao").toString() + ", Quant: " +
                      modelItemPedidosPend->data(row, "quant").toString() + ", R$ " +
                      modelItemPedidosPend->data(row, "preco").toString().replace(".", ",");
    produtos.append(produto);
  }

  dataCompra = inputDlg->getDate();
  dataPrevista = inputDlg->getNextDate();

  //------------------------------
  QXlsx::Document xlsx("MODELO.xlsx");

  QSqlQuery queryVenda;
  queryVenda.prepare("SELECT * FROM venda_has_produto WHERE idProduto = :idProduto");
  queryVenda.bindValue(":idProduto", modelItemPedidosPend->data(lista.first(), "idProduto"));

  if (not queryVenda.exec() or not queryVenda.first()) {
    qDebug() << "Erro buscando dados da venda: " << queryVenda.lastError();
    return;
  }

  QString idVenda = queryVenda.value("idVenda").toString();

  QSqlQuery queryForn;
  queryForn.prepare("SELECT * FROM fornecedor WHERE razaoSocial = (SELECT fornecedor FROM venda_has_produto WHERE "
                    "idVenda = :idVenda LIMIT 1)");
  queryForn.bindValue(":idVenda", idVenda);

  if (not queryForn.exec() or not queryForn.first()) {
    qDebug() << "Erro buscando dados do fornecedor: " << queryForn.lastError();
    return;
  }

  xlsx.write("F4", idVenda); // TODO: temp, replace with correct id
  xlsx.write("E6", queryForn.value("razaoSocial"));
  xlsx.write("E8", queryForn.value("contatoNome"));
  xlsx.write("D10", "Data: " + QDate::currentDate().toString("dd/MM/yy"));

  for (auto row : lista) {
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

  if (xlsx.saveAs(UserSession::getFromLoja("pastaCompra") + "/" + idVenda + ".xlsx")) {
    QMessageBox::information(this, "Ok!", "Arquivo salvo como " + idVenda + ".xlsx");
  } else {
    QMessageBox::warning(this, "Aviso!", "Ocorreu algum erro ao salvar o arquivo.");
  }

  //------------------------------

  QString arquivo = UserSession::getFromLoja("pastaCompra") + "/" + idVenda + ".xlsx";

  QFile file(arquivo);

  if (not file.exists()) {
    qDebug() << "arquivo não encontrado";
  }

  SendMail *mail = new SendMail(this, produtos.join("\n"), arquivo);

  if (mail->exec() != SendMail::Accepted) {
    return;
  }

  for (const auto row : lista) {
    modelItemPedidosPend->setData(row, "selecionado", false);

    if (modelItemPedidosPend->data(row, "status").toString() != "PENDENTE") {
      modelItemPedidosPend->select();
      QMessageBox::warning(this, "Aviso!", "Produto não estava pendente!");
      return;
    }

    if (not modelItemPedidosPend->setData(row, "status", "EM COMPRA")) {
      qDebug() << "Erro marcando status EM COMPRA: " << modelItemPedidosPend->lastError();
    }

    // TODO: gerar e guardar idCompra
    QSqlQuery queryId;

    if (not queryId.exec("SELECT idCompra FROM pedido_fornecedor_has_produto ORDER BY idCompra DESC")) {
      qDebug() << "Erro buscando idCompra: " << queryId.lastError();
      return;
    }

    QString id = "1";

    if (queryId.first()) {
      id = QString::number(queryId.value(0).toInt() + 1);
    }

    if (not modelItemPedidosPend->setData(row, "idCompra", id)) {
      qDebug() << "Erro guardando idCompra: " << modelItemPedidosPend->lastError();
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
      qDebug() << "Erro atualizando status da venda: " << query.lastError();
    }
    //

    if (not modelItemPedidosPend->setData(row, "dataRealCompra", dataCompra.toString("yyyy-MM-dd"))) {
      qDebug() << "Erro guardando data da compra: " << modelItemPedidosPend->lastError();
    }

    if (not modelItemPedidosPend->setData(row, "dataPrevConf", dataPrevista.toString("yyyy-MM-dd"))) {
      qDebug() << "Erro guardando data prevista: " << modelItemPedidosPend->lastError();
    }
  }

  // TODO: guardar valores no contas a pagar
  QSqlQuery query;
  query.prepare("INSERT INTO conta_a_pagar (idVenda, dataEmissao, pago) VALUES (:idVenda, :dataEmissao, :pago)");
  query.bindValue(":idVenda", idVenda);
  query.bindValue(":dataEmissao", QDate::currentDate().toString("yyyy-MM-dd"));
  query.bindValue(":pago", "NÃO");

  if (not query.exec()) {
    qDebug() << "Erro guardando conta a pagar: " << query.lastError();
    return;
  }

  for (auto row : lista) {
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
      qDebug() << "Erro guardando conta a pagar pagamento: " << query.lastError();
      return;
    }
  }

  if (not modelItemPedidosPend->submitAll()) {
    qDebug() << "Erro salvando dados: " << modelItemPedidosPend->lastError();
  }

  updateTables();
}

void MainWindow::on_pushButtonConfirmarCompra_clicked() {
  QString idCompra;

  if (ui->tablePedidosComp->selectionModel()->selectedRows().size() == 0) {
    QMessageBox::warning(this, "Aviso!", "Nenhum item selecionado!");
    return;
  }

  int row = ui->tablePedidosComp->selectionModel()->selectedRows().first().row();
  idCompra =
      modelItemPedidosComp->data(modelItemPedidosComp->index(row, modelItemPedidosComp->record().indexOf("idCompra")))
      .toString();

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
    qDebug() << "Erro atualizando status da compra: " << query.lastError();
  }

  // salvar status na venda
  query.prepare("UPDATE venda_has_produto SET dataRealConf = :dataRealConf, dataPrevFat = :dataPrevFat, status = 'EM "
                "FATURAMENTO' WHERE idCompra = :idCompra");
  query.bindValue(":dataRealConf", dataConf);
  query.bindValue(":dataPrevFat", dataPrevista);
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    qDebug() << "Erro salvando status da venda: " << query.lastError();
  }
  //

  updateTables();

  QMessageBox::information(this, "Aviso!", "Confirmado compra.");
}

void MainWindow::on_radioButtonProdPendTodos_clicked() {
  modelProdPend->setQuery(
        "SELECT v.fornecedor, v.produto, v.formComercial, SUM(v.quant), v.un, v.codComercial, v.idCompra, "
        "v.status, (SELECT SUM(quant) FROM estoque WHERE codComercial = v.codComercial) AS quant FROM venda_has_produto "
        "AS v LEFT "
        "JOIN estoque AS e ON v.idProduto = e.idProduto "
        "GROUP BY v.status, v.codComercial");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("fornecedor"), Qt::Horizontal, "Fornecedor");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("produto"), Qt::Horizontal, "Descrição");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("formComercial"), Qt::Horizontal, "Form.");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("SUM(v.quant)"), Qt::Horizontal, "Quant.");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("un"), Qt::Horizontal, "Un.");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("codComercial"), Qt::Horizontal, "Cód. Com.");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("status"), Qt::Horizontal, "Status");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("quant"), Qt::Horizontal, "Estoque");

  ui->tableProdutosPend->resizeColumnsToContents();
}

void MainWindow::on_radioButtonProdPendPend_clicked() {
  modelProdPend->setQuery(
        "SELECT v.fornecedor, v.produto, v.formComercial, SUM(v.quant), v.un, v.codComercial, v.idCompra, "
        "v.status, (SELECT SUM(quant) FROM estoque WHERE codComercial = v.codComercial) AS quant FROM venda_has_produto "
        "AS v LEFT "
        "JOIN estoque AS e ON v.idProduto = e.idProduto "
        "WHERE v.status = 'PENDENTE' GROUP BY v.status, v.codComercial");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("fornecedor"), Qt::Horizontal, "Fornecedor");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("produto"), Qt::Horizontal, "Descrição");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("formComercial"), Qt::Horizontal, "Form.");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("SUM(v.quant)"), Qt::Horizontal, "Quant.");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("un"), Qt::Horizontal, "Un.");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("codComercial"), Qt::Horizontal, "Cód. Com.");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("status"), Qt::Horizontal, "Status");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("quant"), Qt::Horizontal, "Estoque");

  ui->tableProdutosPend->resizeColumnsToContents();
}

void MainWindow::on_radioButtonProdPendEmCompra_clicked() {
  modelProdPend->setQuery(
        "SELECT v.fornecedor, v.produto, v.formComercial, SUM(v.quant), v.un, v.codComercial, v.idCompra, "
        "v.status, p.quant FROM venda_has_produto AS v LEFT JOIN estoque AS p ON "
        "v.idProduto = p.idProduto WHERE v.status != 'PENDENTE' GROUP BY v.status, v.codComercial");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("fornecedor"), Qt::Horizontal, "Fornecedor");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("produto"), Qt::Horizontal, "Descrição");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("formComercial"), Qt::Horizontal, "Form.");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("SUM(v.quant)"), Qt::Horizontal, "Quant.");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("un"), Qt::Horizontal, "Un.");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("codComercial"), Qt::Horizontal, "Cód. Com.");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("status"), Qt::Horizontal, "Status");
  modelProdPend->setHeaderData(modelProdPend->record().indexOf("quant"), Qt::Horizontal, "Estoque");

  ui->tableProdutosPend->resizeColumnsToContents();
}

void MainWindow::on_pushButtonMarcarColetado_clicked() {
  QList<int> lista;

  for (const auto index :
       modelColeta->match(modelColeta->index(0, modelColeta->fieldIndex("selecionado")), Qt::DisplayRole, true, -1,
                          Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap))) {
    lista.append(index.row());
  }

  if (lista.size() == 0) {
    QMessageBox::warning(this, "Aviso!", "Nenhum item selecionado!");
    return;
  }

  InputDialog *inputDlg = new InputDialog(InputDialog::Coleta, this);
  QDate dataColeta, dataPrevista;

  if (inputDlg->exec() == InputDialog::Accepted) {
    dataColeta = inputDlg->getDate();
    dataPrevista = inputDlg->getNextDate();
  } else {
    return;
  }

  for (const auto row : lista) {
    modelColeta->setData(row, "selecionado", false);

    if (modelColeta->data(row, "status").toString() != "EM COLETA") {
      modelColeta->select();
      QMessageBox::warning(this, "Aviso!", "Produto não estava em coleta!");
      return;
    }

    if (not modelColeta->setData(row, "status", "EM RECEBIMENTO")) {
      qDebug() << "Erro marcando status EM RECEBIMENTO: " << modelColeta->lastError();
    }

    // salvar status na venda
    QSqlQuery query;

    query.prepare("UPDATE venda_has_produto SET dataRealColeta = :dataRealColeta, dataPrevReceb = :dataPrevReceb, "
                  "status = 'EM RECEBIMENTO' WHERE idCompra = :idCompra");
    query.bindValue(":dataRealColeta", dataColeta);
    query.bindValue(":dataPrevReceb", dataPrevista);
    query.bindValue(":idCompra", modelColeta->data(row, "idCompra"));

    if (not query.exec()) {
      qDebug() << "Erro atualizando status da venda: " << query.last();
    }
    //

    if (not modelColeta->setData(row, "dataRealColeta", dataColeta.toString("yyyy-MM-dd"))) {
      qDebug() << "Erro guardando data da coleta: " << modelColeta->lastError();
    }

    if (not modelColeta->setData(row, "dataPrevReceb", dataPrevista.toString("yyyy-MM-dd"))) {
      qDebug() << "Erro guardando data prevista: " << modelColeta->lastError();
    }
  }

  if (not modelColeta->submitAll()) {
    qDebug() << "Erro salvando dados: " << modelColeta->lastError();
  }

  updateTables();

  QMessageBox::information(this, "Aviso!", "Confirmado coleta.");
}

void MainWindow::on_pushButtonMarcarRecebido_clicked() {
  QList<int> lista;

  for (const auto index :
       modelReceb->match(modelReceb->index(0, modelReceb->fieldIndex("selecionado")), Qt::DisplayRole, true, -1,
                         Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap))) {
    lista.append(index.row());
  }

  if (lista.size() == 0) {
    QMessageBox::warning(this, "Aviso!", "Nenhum item selecionado!");
    return;
  }

  InputDialog *inputDlg = new InputDialog(InputDialog::Recebimento, this);
  QDate dataReceb;

  if (inputDlg->exec() == InputDialog::Accepted) {
    dataReceb = inputDlg->getDate();
  } else {
    return;
  }

  for (const auto row : lista) {
    modelReceb->setData(row, "selecionado", false);

    if (modelReceb->data(row, "status").toString() != "EM RECEBIMENTO") {
      modelReceb->select();
      QMessageBox::warning(this, "Aviso!", "Produto não estava em recebimento!");
      return;
    }

    if (not modelReceb->setData(row, "status", "FINALIZADO")) {
      qDebug() << "Erro marcando status ESTOQUE: " << modelReceb->lastError();
    }

    // salvar status na venda
    QSqlQuery query;

    query.prepare(
          "UPDATE venda_has_produto SET dataRealReceb = :dataRealReceb, status = 'ESTOQUE' WHERE idCompra = :idCompra");
    query.bindValue(":dataRealReceb", dataReceb);
    query.bindValue(":idCompra", modelReceb->data(row, "idCompra"));

    if (not query.exec()) {
      qDebug() << "Erro atualizando status da venda: " << query.last();
    }
    //

    if (not modelReceb->setData(row, "dataRealReceb", dataReceb.toString("yyyy-MM-dd"))) {
      qDebug() << "Erro guardando data de recebimento: " << modelReceb->lastError();
    }
  }

  // TODO: marcar flag no estoque de produto disponivel

  if (not modelReceb->submitAll()) {
    qDebug() << "Erro salvando dados: " << modelReceb->lastError();
  }

  updateTables();

  QMessageBox::information(this, "Aviso!", "Confirmado recebimento.");
}

void MainWindow::on_pushButtonMarcarFaturado_clicked() {
  QString idCompra;
  QList<int> rows;

  if (ui->tableFaturamento->selectionModel()->selectedRows().size() == 0) {
    QMessageBox::warning(this, "Aviso!", "Nenhum item selecionado.");
    return;
  }

  for (auto index : ui->tableFaturamento->selectionModel()->selectedRows()) {
    rows.append(modelFat->data(modelFat->index(index.row(), modelFat->record().indexOf("idCompra"))).toInt());
  }

  int row = ui->tableFaturamento->selectionModel()->selectedRows().first().row();
  idCompra = modelFat->data(modelFat->index(row, modelFat->record().indexOf("idCompra"))).toString();

  ImportarXML *import = new ImportarXML(rows, this);
  import->show();

  if (import->exec() != QDialog::Accepted) {
    return;
  }

  //----------------------------------------------------------//

  InputDialog *inputDlg = new InputDialog(InputDialog::ConfirmarCompra, this);
  inputDlg->setFilter(idCompra);
  QDate dataFat, dataPrevista;

  if (inputDlg->exec() != InputDialog::Accepted) {
    return;
  }

  dataFat = inputDlg->getDate();
  dataPrevista = inputDlg->getNextDate();

  QSqlQuery query;
  query.prepare("UPDATE pedido_fornecedor_has_produto SET dataRealFat = :dataRealFat, dataPrevColeta = "
                ":dataPrevColeta, status = 'EM COLETA' WHERE idCompra = :idCompra");
  query.bindValue(":dataRealFat", dataFat);
  query.bindValue(":dataPrevColeta", dataPrevista);
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    qDebug() << "Erro atualizando status da compra: " << query.lastError();
  }

  // salvar status na venda
  query.prepare("UPDATE venda_has_produto SET dataRealFat = :dataRealFat, dataPrevColeta = :dataPrevColeta, status = "
                "'EM COLETA' WHERE idCompra = :idCompra");
  query.bindValue(":dataRealFat", dataFat);
  query.bindValue(":dataPrevColeta", dataPrevista);
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    qDebug() << "Erro salvando status na venda: " << query.lastError();
  }
  //

  updateTables();

  QMessageBox::information(this, "Aviso!", "Confirmado faturamento.");
}

void MainWindow::on_pushButtonTesteFaturamento_clicked() {
  QList<int> temp;
  ImportarXML *import = new ImportarXML(temp, this);
  import->show();
}

void MainWindow::on_tableFornLogistica_activated(const QModelIndex &index) {
  QString fornecedor =
      modelPedForn->data(modelPedForn->index(index.row(), modelPedForn->record().indexOf("fornecedor"))).toString();

  modelColeta->setFilter("fornecedor = '" + fornecedor + "' AND status = 'EM COLETA'");

  if (not modelColeta->select()) {
    qDebug() << "Error: " << modelColeta->lastError();
  }

  modelReceb->setFilter("fornecedor = '" + fornecedor + "' AND status = 'EM RECEBIMENTO'");

  if (not modelReceb->select()) {
    qDebug() << "Error: " << modelReceb->lastError();
  }

  updateTables();
}

void MainWindow::on_pushButtonTesteEmail_clicked() {
  SendMail *mail = new SendMail(this);
  mail->show();
}

void MainWindow::on_pushButtonComprar_clicked() {}

void MainWindow::on_pushButtonTodosFornCompras_clicked() {
  modelItemPedidosPend->setFilter("status = 'PENDENTE'");

  updateTables();
}

void MainWindow::on_pushButtonExibirXML_clicked() {
  QString xml = QFileDialog::getOpenFileName(this, "Arquivo XML", QDir::currentPath(), "*.xml");

  if (xml.isEmpty()) {
    return;
  }

  QFile file(xml);

  if (not file.open(QFile::ReadOnly)) {
    qDebug() << "Erro abrindo arquivo: " << file.errorString();
    return;
  }

  xml = file.readAll();

  XML_Viewer *viewer = new XML_Viewer;
  viewer->exibirXML(xml);
  viewer->show();
}

void MainWindow::on_tableNfeEntrada_activated(const QModelIndex &index) {
  XML_Viewer *viewer = new XML_Viewer(this);

  viewer->exibirXML(modelNfeEntrada->data(index.row(), "xml").toString());
  viewer->show();
}

bool MainWindow::getHomologacao() const { return homologacao; }

void MainWindow::setHomologacao(bool value) { homologacao = value; }

// TODO: substituir qDebug's por QMessageBox's
// TODO: gerenciar lugares de estoque (cadastro/permissoes)
// TODO: a tabela de fornecedores em compra deve mostrar apenas os pedidos que estejam pendente/confirmar/faturar
// TODO: try making most functions const (hint is if there is no red text or no 'this' it problably should be const)
// TODO: verify if rows should be resized to contents too
