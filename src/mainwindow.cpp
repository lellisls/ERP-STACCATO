#include <QShortcut>
#include <QStyleFactory>
#include <QFileDialog>
#include <QSqlRecord>
#include <QDate>
#include <QTimer>
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
#include "checkboxdelegate.h"
#include "comboboxdelegate.h"
#include "contasapagar.h"
#include "contasareceber.h"
#include "doubledelegate.h"
#include "entregascliente.h"
#include "estoque.h"
#include "importaprodutos.h"
#include "importarxml.h"
#include "inputdialog.h"
#include "logindialog.h"
#include "orcamento.h"
#include "orcamentoproxymodel.h"
#include "porcentagemdelegate.h"
#include "produtospendentes.h"
#include "QSimpleUpdater"
#include "sendmail.h"
#include "usersession.h"
#include "venda.h"
#include "xlsxdocument.h"
#include "xml.h"
#include "xml_viewer.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  defaultStyle = this->style()->objectName();
  defautPalette = qApp->palette();

  qApp->setApplicationVersion("0.6");

  //  setSettings("Login/hostname", ""); //to test store selection

  if (settings("Login/hostname").toString().isEmpty()) {
    QStringList items;
    items << "Alphaville"
          << "Gabriel";

    QString loja = QInputDialog::getItem(this, "Escolha a loja", "Qual a sua loja?", items, 0, false);

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

  ui->splitter_5->setStretchFactor(0, 0);
  ui->splitter_5->setStretchFactor(1, 1);

  ui->splitter_6->setStretchFactor(0, 0);
  ui->splitter_6->setStretchFactor(1, 1);

  setWindowIcon(QIcon("Staccato.ico"));
  setWindowTitle("ERP Staccato");
  readSettings();

  QShortcut *shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this);
  connect(shortcut, &QShortcut::activated, this, &QWidget::close);

  setupTables();

  setWindowTitle(windowTitle() + " - " + UserSession::getNome() + " - " + UserSession::getTipoUsuario() + " - " +
                 hostname + (homologacao ? " - HOMOLOGACAO" : ""));

  ui->radioButtonOrcLimpar->click();
  ui->radioButtonVendLimpar->click();

  if (UserSession::getTipoUsuario() != "ADMINISTRADOR") {
    ui->actionGerenciar_Lojas->setDisabled(true);
    ui->actionGerenciar_Transportadoras->setDisabled(true);
    ui->actionImportaProdutos->setDisabled(true);
    ui->actionCadastrarUsuario->setDisabled(true);
    ui->actionCadastrarProfissional->setDisabled(true);
    ui->actionCadastrarFornecedor->setDisabled(true);
    ui->groupBoxLojas->hide();
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

    //    ui->radioButtonOrcValido->setChecked(true);
    //    on_radioButtonOrcValido_clicked();
    ui->radioButtonOrcProprios->click();
    ui->radioButtonVendProprios->click();
  }

  ui->radioButtonProdPendPend->click();

  connect(ui->radioButtonVendLimpar, &QAbstractButton::toggled, this, &MainWindow::montaFiltroVendas);
  connect(ui->radioButtonVendProprios, &QAbstractButton::toggled, this, &MainWindow::montaFiltroVendas);
  connect(ui->checkBoxVendaPendente, &QAbstractButton::toggled, this, &MainWindow::montaFiltroVendas);
  connect(ui->checkBoxVendaIniciado, &QAbstractButton::toggled, this, &MainWindow::montaFiltroVendas);
  connect(ui->checkBoxVendaEmCompra, &QAbstractButton::toggled, this, &MainWindow::montaFiltroVendas);
  connect(ui->checkBoxVendaEmFaturamento, &QAbstractButton::toggled, this, &MainWindow::montaFiltroVendas);
  connect(ui->checkBoxVendaEmColeta, &QAbstractButton::toggled, this, &MainWindow::montaFiltroVendas);
  connect(ui->checkBoxVendaEmRecebimento, &QAbstractButton::toggled, this, &MainWindow::montaFiltroVendas);
  connect(ui->checkBoxVendaEstoque, &QAbstractButton::toggled, this, &MainWindow::montaFiltroVendas);
  connect(ui->checkBoxVendaFinalizado, &QAbstractButton::toggled, this, &MainWindow::montaFiltroVendas);

  montaFiltroVendas();

  QSqlQuery query("SELECT * FROM loja WHERE descricao != 'Geral'");

  ui->comboBoxLojas->addItem("");

  while (query.next()) {
    ui->comboBoxLojas->addItem(query.value("sigla").toString(), query.value("idLoja"));
  }

  ui->comboBoxLojas->setCurrentValue(UserSession::getLoja());
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

  ui->tableOrcamentos->setModel(new OrcamentoProxyModel(modelOrcamento, "Dias restantes", this));
  ui->tableOrcamentos->setItemDelegate(doubledelegate);
  ui->tableOrcamentos->sortByColumn("Código");

  // Vendas ------------------------------------------------------------------------------------------------------------
  modelVendas = new SqlTableModel(this);
  modelVendas->setTable("view_venda");

  ui->tableVendas->setModel(new OrcamentoProxyModel(modelVendas, "Dias restantes", this));
  ui->tableVendas->setItemDelegateForColumn("Bruto", doubledelegate);
  ui->tableVendas->setItemDelegateForColumn("Líquido", doubledelegate);
  ui->tableVendas->setItemDelegateForColumn("Frete", doubledelegate);
  ui->tableVendas->setItemDelegateForColumn("Total R$", doubledelegate);
  ui->tableVendas->sortByColumn("Código");

  // Produtos Pendentes ------------------------------------------------------------------------------------------------
  modelProdPend = new SqlTableModel(this);
  modelProdPend->setTable("view_produtos_pendentes");

  modelProdPend->setHeaderData("Form", "Form.");
  modelProdPend->setHeaderData("Quant", "Quant.");
  modelProdPend->setHeaderData("Un", "Un.");
  modelProdPend->setHeaderData("Cód Com", "Cód. Com.");

  ui->tableProdutosPend->setModel(modelProdPend);

  // Fornecedores Compras ----------------------------------------------------------------------------------------------
  modelPedForn = new SqlTableModel(this);
  modelPedForn->setTable("view_fornecedor_compra");

  modelPedForn->setHeaderData("fornecedor", "Fornecedor");
  modelPedForn->setHeaderData("COUNT(fornecedor)", "Itens");

  ui->tableFornCompras->setModel(modelPedForn);

  // Fornecedores Logística --------------------------------------------------------------------------------------------
  modelPedForn2 = new SqlTableModel(this);
  modelPedForn2->setTable("view_fornecedor_logistica");

  modelPedForn2->setHeaderData("fornecedor", "Fornecedor");
  modelPedForn2->setHeaderData("COUNT(fornecedor)", "Itens");

  ui->tableFornLogistica->setModel(modelPedForn2);

  // Compras - Pendentes -----------------------------------------------------------------------------------------------
  modelItemPedidosPend = new SqlTableModel(this);
  modelItemPedidosPend->setTable("pedido_fornecedor_has_produto");
  modelItemPedidosPend->setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelItemPedidosPend->setHeaderData("selecionado", "");
  modelItemPedidosPend->setHeaderData("fornecedor", "Fornecedor");
  modelItemPedidosPend->setHeaderData("descricao", "Descrição");
  modelItemPedidosPend->setHeaderData("colecao", "Coleção");
  modelItemPedidosPend->setHeaderData("quant", "Quant.");
  modelItemPedidosPend->setHeaderData("un", "Un.");
  modelItemPedidosPend->setHeaderData("preco", "Preço");
  modelItemPedidosPend->setHeaderData("formComercial", "Form. Com.");
  modelItemPedidosPend->setHeaderData("codComercial", "Cód. Com.");
  modelItemPedidosPend->setHeaderData("codBarras", "Cód. Bar.");
  modelItemPedidosPend->setHeaderData("idCompra", "Compra");
  modelItemPedidosPend->setHeaderData("dataPrevCompra", "Prev. Compra");
  modelItemPedidosPend->setHeaderData("dataCompra", "Data Compra");
  modelItemPedidosPend->setHeaderData("status", "Status");

  modelItemPedidosPend->setFilter("status = 'PENDENTE'");

  ui->tablePedidosPend->setModel(modelItemPedidosPend);
  ui->tablePedidosPend->setItemDelegateForColumn("selecionado", new CheckBoxDelegate(this));
  ui->tablePedidosPend->hideColumn("quantUpd");
  ui->tablePedidosPend->hideColumn("idPedido");
  ui->tablePedidosPend->hideColumn("idLoja");
  ui->tablePedidosPend->hideColumn("item");
  ui->tablePedidosPend->hideColumn("idProduto");
  ui->tablePedidosPend->hideColumn("prcUnitario");
  ui->tablePedidosPend->hideColumn("parcial");
  ui->tablePedidosPend->hideColumn("desconto");
  ui->tablePedidosPend->hideColumn("parcialDesc");
  ui->tablePedidosPend->hideColumn("descGlobal");
  ui->tablePedidosPend->hideColumn("dataRealCompra");
  ui->tablePedidosPend->hideColumn("dataPrevConf");
  ui->tablePedidosPend->hideColumn("dataRealConf");
  ui->tablePedidosPend->hideColumn("dataPrevFat");
  ui->tablePedidosPend->hideColumn("dataRealFat");
  ui->tablePedidosPend->hideColumn("dataPrevColeta");
  ui->tablePedidosPend->hideColumn("dataRealColeta");
  ui->tablePedidosPend->hideColumn("dataPrevEnt");
  ui->tablePedidosPend->hideColumn("dataRealEnt");
  ui->tablePedidosPend->hideColumn("dataPrevReceb");
  ui->tablePedidosPend->hideColumn("dataRealReceb");

  // Compras - A Confirmar ---------------------------------------------------------------------------------------------
  modelItemPedidosComp = new SqlTableModel(this);
  modelItemPedidosComp->setTable("view_compras");

  modelItemPedidosComp->setHeaderData("fornecedor", "Fornecedor");
  modelItemPedidosComp->setHeaderData("idCompra", "Compra");
  modelItemPedidosComp->setHeaderData("COUNT(idProduto)", "Itens");
  modelItemPedidosComp->setHeaderData("SUM(preco)", "Preço");
  modelItemPedidosComp->setHeaderData("status", "Status");

  ui->tablePedidosComp->setModel(modelItemPedidosComp);

  // Faturamentos ------------------------------------------------------------------------------------------------------
  modelFat = new SqlTableModel(this);
  modelFat->setTable("view_faturamento");

  modelFat->setHeaderData("fornecedor", "Fornecedor");
  modelFat->setHeaderData("idCompra", "Compra");
  modelFat->setHeaderData("COUNT(idProduto)", "Itens");
  modelFat->setHeaderData("SUM(preco)", "Preço");
  modelFat->setHeaderData("status", "Status");

  ui->tableFaturamento->setModel(modelFat);

  // Coletas -----------------------------------------------------------------------------------------------------------
  modelColeta = new SqlTableModel(this);
  modelColeta->setTable("pedido_fornecedor_has_produto");
  modelColeta->setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelColeta->setHeaderData("selecionado", "");
  modelColeta->setHeaderData("fornecedor", "Fornecedor");
  modelColeta->setHeaderData("descricao", "Descrição");
  modelColeta->setHeaderData("colecao", "Coleção");
  modelColeta->setHeaderData("quant", "Quant.");
  modelColeta->setHeaderData("un", "Un.");
  modelColeta->setHeaderData("preco", "Preço");
  modelColeta->setHeaderData("formComercial", "Form. Com.");
  modelColeta->setHeaderData("codComercial", "Cód. Com.");
  modelColeta->setHeaderData("codBarras", "Cód. Bar.");
  modelColeta->setHeaderData("idCompra", "Compra");
  modelColeta->setHeaderData("dataRealFat", "Data Fat.");
  modelColeta->setHeaderData("dataPrevColeta", "Prev. Coleta");
  modelColeta->setHeaderData("status", "Status");

  modelColeta->setFilter("status = 'EM COLETA'");

  ui->tableColeta->setModel(modelColeta);
  ui->tableColeta->setItemDelegateForColumn("status", new ComboBoxDelegate(this));
  ui->tableColeta->setItemDelegateForColumn("selecionado", new CheckBoxDelegate(this));
  ui->tableColeta->hideColumn("idPedido");
  ui->tableColeta->hideColumn("idLoja");
  ui->tableColeta->hideColumn("item");
  ui->tableColeta->hideColumn("idProduto");
  ui->tableColeta->hideColumn("prcUnitario");
  ui->tableColeta->hideColumn("parcial");
  ui->tableColeta->hideColumn("desconto");
  ui->tableColeta->hideColumn("parcialDesc");
  ui->tableColeta->hideColumn("descGlobal");
  ui->tableColeta->hideColumn("dataPrevCompra");
  ui->tableColeta->hideColumn("dataRealCompra");
  ui->tableColeta->hideColumn("dataPrevConf");
  ui->tableColeta->hideColumn("dataRealConf");
  ui->tableColeta->hideColumn("dataPrevFat");
  ui->tableColeta->hideColumn("dataRealColeta");
  ui->tableColeta->hideColumn("dataPrevEnt");
  ui->tableColeta->hideColumn("dataRealEnt");
  ui->tableColeta->hideColumn("dataPrevReceb");
  ui->tableColeta->hideColumn("dataRealReceb");
  ui->tableColeta->hideColumn("quantUpd");

  // Recebimentos fornecedor -------------------------------------------------------------------------------------------
  modelReceb = new SqlTableModel(this);
  modelReceb->setTable("pedido_fornecedor_has_produto");
  modelReceb->setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelReceb->setHeaderData("selecionado", "");
  modelReceb->setHeaderData("fornecedor", "Fornecedor");
  modelReceb->setHeaderData("descricao", "Descrição");
  modelReceb->setHeaderData("colecao", "Coleção");
  modelReceb->setHeaderData("quant", "Quant.");
  modelReceb->setHeaderData("un", "Un.");
  modelReceb->setHeaderData("preco", "Preço");
  modelReceb->setHeaderData("formComercial", "Form. Com.");
  modelReceb->setHeaderData("codComercial", "Cód. Com.");
  modelReceb->setHeaderData("codBarras", "Cód. Bar.");
  modelReceb->setHeaderData("idCompra", "Compra");
  modelReceb->setHeaderData("dataRealColeta", "Data Coleta");
  modelReceb->setHeaderData("dataPrevReceb", "Prev. Receb.");
  modelReceb->setHeaderData("status", "Status");

  modelReceb->setFilter("status = 'EM RECEBIMENTO'");

  ui->tableRecebimento->setModel(modelReceb);
  ui->tableRecebimento->setItemDelegateForColumn("status", new ComboBoxDelegate(this));
  ui->tableRecebimento->setItemDelegateForColumn("selecionado", new CheckBoxDelegate(this));
  ui->tableRecebimento->hideColumn("idPedido");
  ui->tableRecebimento->hideColumn("idLoja");
  ui->tableRecebimento->hideColumn("item");
  ui->tableRecebimento->hideColumn("idProduto");
  ui->tableRecebimento->hideColumn("prcUnitario");
  ui->tableRecebimento->hideColumn("parcial");
  ui->tableRecebimento->hideColumn("desconto");
  ui->tableRecebimento->hideColumn("parcialDesc");
  ui->tableRecebimento->hideColumn("descGlobal");
  ui->tableRecebimento->hideColumn("dataPrevCompra");
  ui->tableRecebimento->hideColumn("dataRealCompra");
  ui->tableRecebimento->hideColumn("dataPrevConf");
  ui->tableRecebimento->hideColumn("dataRealConf");
  ui->tableRecebimento->hideColumn("dataPrevFat");
  ui->tableRecebimento->hideColumn("dataRealFat");
  ui->tableRecebimento->hideColumn("dataPrevEnt");
  ui->tableRecebimento->hideColumn("dataRealEnt");
  ui->tableRecebimento->hideColumn("dataPrevColeta");
  ui->tableRecebimento->hideColumn("dataRealReceb");
  ui->tableRecebimento->hideColumn("quantUpd");

  // Entregas cliente --------------------------------------------------------------------------------------------------
  modelEntregasCliente = new SqlTableModel(this);
  modelEntregasCliente->setTable("view_venda");

  ui->tableEntregasCliente->setModel(modelEntregasCliente);
  ui->tableEntregasCliente->setItemDelegate(doubledelegate);

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

  // Estoque -----------------------------------------------------------------------------------------------------------
  modelEstoque = new SqlTableModel(this);
  modelEstoque->setTable("view_estoque");
  modelEstoque->setEditStrategy(QSqlTableModel::OnManualSubmit);

  ui->tableEstoque->setModel(modelEstoque);
  ui->tableEstoque->setItemDelegate(doubledelegate);

  // Contas a pagar ----------------------------------------------------------------------------------------------------
  modelCAPagar = new SqlTableModel(this);
  modelCAPagar->setTable("conta_a_pagar");
  modelCAPagar->setEditStrategy(QSqlTableModel::OnManualSubmit);

  ui->tableContasPagar->setModel(modelCAPagar);
  ui->tableContasPagar->setItemDelegate(doubledelegate);

  // Contas a receber --------------------------------------------------------------------------------------------------
  modelCAReceber = new SqlTableModel(this);
  modelCAReceber->setTable("conta_a_receber");
  modelCAReceber->setEditStrategy(QSqlTableModel::OnManualSubmit);

  ui->tableContasReceber->setModel(modelCAReceber);
  ui->tableContasReceber->setItemDelegate(doubledelegate);
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
  if (ui->tabWidget->currentIndex() == 0) { // Orcamentos
    if (not modelOrcamento->select()) {
      QMessageBox::critical(this, "Erro!", "Erro lendo tabela orçamento: " + modelOrcamento->lastError().text());
      return;
    }

    ui->tableOrcamentos->resizeColumnsToContents();
  }

  if (ui->tabWidget->currentIndex() == 1) { // Vendas
    if (not modelVendas->select()) {
      QMessageBox::critical(this, "Erro!", "Erro lendo tabela vendas: " + modelVendas->lastError().text());
      return;
    }

    ui->tableVendas->resizeColumnsToContents();
  }

  if (ui->tabWidget->currentIndex() == 2) { // Compras
    if (not modelProdPend->select()) {
      QMessageBox::critical(this, "Erro!",
                            "Erro lendo tabela produtos pendentes: " + modelProdPend->lastError().text());
      return;
    }

    if (not modelPedForn->select()) {
      QMessageBox::critical(this, "Erro!",
                            "Erro lendo tabela fornecedores compra: " + modelPedForn->lastError().text());
      return;
    }

    ui->tableProdutosPend->resizeColumnsToContents();
    ui->tableFornCompras->resizeColumnsToContents();

    if (ui->tabWidget_2->currentIndex() == 0) { // Gerar Compra
      modelItemPedidosPend->setFilter("0");

      if (not modelItemPedidosPend->select()) {
        QMessageBox::critical(this, "Erro!", "Erro lendo tabela pedido_fornecedor_has_produto: " +
                              modelItemPedidosPend->lastError().text());
        return;
      }

      for (int i = 0; i < modelItemPedidosPend->rowCount(); ++i) {
        ui->tablePedidosPend->openPersistentEditor(
              modelItemPedidosPend->index(i, modelItemPedidosPend->fieldIndex("selecionado")));
      }

      ui->tablePedidosPend->resizeColumnsToContents();
    }

    if (ui->tabWidget_2->currentIndex() == 1) { // Confirmar Compra
      if (not modelItemPedidosComp->select()) {
        QMessageBox::critical(this, "Erro!", "Erro lendo tabela compras: " + modelItemPedidosComp->lastError().text());
        return;
      }

      ui->tablePedidosComp->resizeColumnsToContents();
    }

    if (ui->tabWidget_2->currentIndex() == 2) { // Faturamento
      if (not modelFat->select()) {
        QMessageBox::critical(this, "Erro!", "Erro lendo tabela faturamento: " + modelFat->lastError().text());
        return;
      }

      ui->tableFaturamento->resizeColumnsToContents();
    }
  }

  if (ui->tabWidget->currentIndex() == 3) { // Logistica
    if (not modelPedForn2->select()) {
      QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + modelPedForn2->lastError().text());
      return;
    }

    ui->tableFornLogistica->resizeColumnsToContents();

    if (ui->tabWidget_3->currentIndex() == 0) { // Coleta
      // TODO: set filter = 0? (for when no manufacturer is selected)
      if (not modelColeta->select()) {
        QMessageBox::critical(this, "Erro!",
                              "Erro lendo tabela pedido_fornecedor_has_produto: " + modelColeta->lastError().text());
        return;
      }

      for (int i = 0; i < modelColeta->rowCount(); ++i) {
        ui->tableColeta->openPersistentEditor(modelColeta->index(i, modelColeta->fieldIndex("selecionado")));
      }

      ui->tableColeta->resizeColumnsToContents();
    }

    if (ui->tabWidget_3->currentIndex() == 1) { // Recebimento
      if (not modelReceb->select()) {
        QMessageBox::critical(this, "Erro!",
                              "Erro lendo tabela pedido_fornecedor_has_produto: " + modelReceb->lastError().text());
        return;
      }

      for (int i = 0; i < modelReceb->rowCount(); ++i) {
        ui->tableRecebimento->openPersistentEditor(modelReceb->index(i, modelReceb->fieldIndex("selecionado")));
      }

      ui->tableRecebimento->resizeColumnsToContents();
    }

    if (ui->tabWidget_3->currentIndex() == 2) { // Entregas
      if (not modelEntregasCliente->select()) {
        QMessageBox::critical(this, "Erro!", "Erro lendo tabela vendas: " + modelEntregasCliente->lastError().text());
        return;
      }

      ui->tableEntregasCliente->resizeColumnsToContents();
    }
  }

  if (ui->tabWidget->currentIndex() == 4) {     // NFe
    if (ui->tabWidget_4->currentIndex() == 0) { // Entrada
      if (not modelNfeEntrada->select()) {
        QMessageBox::critical(this, "Erro!", "Erro lendo tabela NFe: " + modelNfeEntrada->lastError().text());
        return;
      }

      ui->tableNfeEntrada->resizeColumnsToContents();
    }

    if (ui->tabWidget_4->currentIndex() == 1) { // Saida
      if (not modelNfeSaida->select()) {
        QMessageBox::critical(this, "Erro!", "Erro lendo tabela NFe: " + modelNfeSaida->lastError().text());
        return;
      }

      ui->tableNfeSaida->resizeColumnsToContents();
    }
  }

  if (ui->tabWidget->currentIndex() == 5) { // Estoque
    if (not modelEstoque->select()) {
      QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + modelEstoque->lastError().text());
      return;
    }
  }

  if (ui->tabWidget->currentIndex() == 6) { // Contas
    if (not modelCAPagar->select()) {
      QMessageBox::critical(this, "Erro!", "Erro lendo tabela conta_a_pagar: " + modelCAPagar->lastError().text());
      return;
    }

    if (not modelCAReceber->select()) {
      QMessageBox::critical(this, "Erro!", "Erro lendo tabela conta_a_receber: " + modelCAReceber->lastError().text());
      return;
    }
  }
}

void MainWindow::on_radioButtonOrcValido_clicked() {
  modelOrcamento->setFilter("(Código LIKE '%" + UserSession::getSiglaLoja() +
                            "%') AND `Dias restantes` > 0 AND status != 'CANCELADO'");
  ui->tableOrcamentos->resizeColumnsToContents();
}

void MainWindow::on_radioButtonOrcExpirado_clicked() {
  modelOrcamento->setFilter("(Código LIKE '%" + UserSession::getSiglaLoja() + "%') AND `Dias restantes` < 1");
  ui->tableOrcamentos->resizeColumnsToContents();
}

void MainWindow::on_radioButtonOrcLimpar_clicked() {
  modelOrcamento->setFilter("(Código LIKE '%" + UserSession::getSiglaLoja() + "%')");
  ui->tableOrcamentos->resizeColumnsToContents();
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
  modelOrcamento->setFilter("(Código LIKE '%" + UserSession::getSiglaLoja() + "%') AND Vendedor = '" +
                            UserSession::getNome() + "'");
  ui->tableOrcamentos->resizeColumnsToContents();
}

void MainWindow::on_pushButtonCriarOrc_clicked() { on_actionCriarOrcamento_triggered(); }

void MainWindow::on_lineEditBuscaOrcamentos_textChanged(const QString &text) {
  modelOrcamento->setFilter("(Código LIKE '%" + UserSession::getSiglaLoja() + "%')" +
                            (text.isEmpty() ? "" : " AND ((Código LIKE '%" + text + "%') OR (Vendedor LIKE '%" + text +
                                              "%') OR (Cliente LIKE '%" + text + "%'))"));

  ui->tableOrcamentos->resizeColumnsToContents();
}

void MainWindow::on_lineEditBuscaVendas_textChanged(const QString &text) {
  if (text.isEmpty()) {
    montaFiltroVendas();
    //    modelVendas->setFilter("");
  } else {
    // TODO: append this to montaFiltroVendas?
    modelVendas->setFilter("(Código LIKE '%" + text + "%') OR (Vendedor LIKE '%" + text + "%') OR (Cliente LIKE '%" +
                           text + "%')");
  }

  // TODO: refactor to conditional operator

  ui->tableVendas->resizeColumnsToContents();
}

void MainWindow::on_lineEditBuscaContasPagar_textChanged(const QString &text) {
  modelCAPagar->setFilter(text.isEmpty() ? "" : "(idVenda LIKE '%" + text + "%') OR (pago LIKE '%" + text + "%')");

  ui->tableContasPagar->resizeColumnsToContents();
}

void MainWindow::on_lineEditBuscaContasReceber_textChanged(const QString &text) {
  modelCAReceber->setFilter(text.isEmpty() ? "" : "(idVenda LIKE '%" + text + "%') OR (pago LIKE '%" + text + "%')");

  ui->tableContasReceber->resizeColumnsToContents();
}

void MainWindow::on_lineEditBuscaEntregas_textChanged(const QString &text) {
  modelEntregasCliente->setFilter(text.isEmpty() ? "" : "(idPedido LIKE '%" + text + "%') OR (status LIKE '%" + text +
                                                   "%')");

  ui->tableEntregasCliente->resizeColumnsToContents();
}

void MainWindow::on_lineEditBuscaNFe_textChanged(const QString &text) {
  modelNfeSaida->setFilter(text.isEmpty() ? "" : "(idVenda LIKE '%" + text + "%') OR (status LIKE '%" + text + "%')");

  ui->tableProdutosPend->resizeColumnsToContents();
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

void MainWindow::on_tableVendas_activated(const QModelIndex &index) {
  Venda *vendas = new Venda(this);
  vendas->viewRegisterById(modelVendas->data(index.row(), "Código"));
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
  entregas->viewEntrega(modelEntregasCliente->data(index.row(), "Código").toString());
}

void MainWindow::on_tableFornCompras_activated(const QModelIndex &index) {
  int row = index.row();

  QString fornecedor = modelPedForn->data(row, "fornecedor").toString();

  modelItemPedidosPend->setFilter("fornecedor = '" + fornecedor + "' AND status = 'PENDENTE'");

  if (not modelItemPedidosPend->select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela pedido_fornecedor_has_produto: " +
                          modelItemPedidosPend->lastError().text());
    return;
  }

  for (int i = 0; i < modelItemPedidosPend->rowCount(); ++i) {
    ui->tablePedidosPend->openPersistentEditor(
          modelItemPedidosPend->index(i, modelItemPedidosPend->fieldIndex("selecionado")));
  }

  ui->tablePedidosPend->resizeColumnsToContents();

  // TODO: filter compras e faturamentos?

  //  updateTables();

  //  ui->tableFornCompras->selectRow(row);
}

void MainWindow::on_tableNfeSaida_activated(const QModelIndex &index) {
  Venda *vendas = new Venda(this);
  vendas->viewRegisterById(modelNfeSaida->data(index.row(), "idVenda"));
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

void MainWindow::on_tableProdutosPend_activated(const QModelIndex &index) {
  ProdutosPendentes *produtos = new ProdutosPendentes(this);

  QString codComercial = modelProdPend->data(index.row(), "codComercial").toString();
  QString status = modelProdPend->data(index.row(), "status").toString();

  produtos->viewProduto(codComercial, status);
}

void MainWindow::on_pushButtonGerarCompra_clicked() { // TODO: refactor this function into smaller functions
  // TODO: colocar uma transaction
  QFile modelo(QDir::currentPath() + "/modelo.xlsx");

  if (not modelo.exists()) {
    QMessageBox::critical(this, "Erro!", "Não encontrou o modelo do Excel!");
    return;
  }

  //  if(modelItem.rowCount() > 17){
  //    QMessageBox::critical(this, "Erro!", "Mais itens do que cabe no modelo!");
  //    return;
  //  }

  if (settings("User/userFolder").toString().isEmpty()) {
    QMessageBox::warning(this, "Aviso!", "Não há uma pasta definida para salvar PDF/Excel. Por favor escolha uma.");
    setSettings("User/userFolder", QFileDialog::getExistingDirectory(this, "Pasta PDF/Excel"));
    return;
  }

  if (UserSession::getFromLoja("emailCompra").isEmpty()) {
    // TODO: set value here or open window to set
    QMessageBox::warning(this, "Aviso!",
                         "Não há um email de compras definido, favor cadastrar nas configurações da loja.");
    return;
  }

  if (modelPedForn->rowCount() > 1) {
    if (not modelItemPedidosPend->filter().contains("fornecedor")) {
      QMessageBox::critical(this, "Erro!", "Selecione o fornecedor na tabela à esquerda.");
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

  QString filtro = modelItemPedidosPend->filter();

  if (inputDlg->exec() != InputDialog::Accepted) {
    return;
  }

  dataCompra = inputDlg->getDate();
  dataPrevista = inputDlg->getNextDate();

  modelItemPedidosPend->setFilter(filtro);
  modelItemPedidosPend->select();

  for (const auto row : lista) {
    QString produto = modelItemPedidosPend->data(row, "descricao").toString() + ", Quant: " +
                      modelItemPedidosPend->data(row, "quant").toString() + ", R$ " +
                      modelItemPedidosPend->data(row, "preco").toString().replace(".", ",");
    produtos.append(produto);
  }

  //------------------------------
  // TODO: refactor this from venda or orcamento
  QXlsx::Document xlsx("modelo.xlsx");

  QSqlQuery queryVenda;
  queryVenda.prepare("SELECT * FROM venda_has_produto WHERE idProduto = :idProduto");
  queryVenda.bindValue(":idProduto", modelItemPedidosPend->data(lista.first(), "idProduto"));

  if (not queryVenda.exec() or not queryVenda.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados da venda:" + queryVenda.lastError().text());
    return;
  }

  QString idVenda = queryVenda.value("idVenda").toString();

  QSqlQuery queryForn;
  queryForn.prepare("SELECT * FROM fornecedor WHERE razaoSocial = (SELECT fornecedor FROM venda_has_produto WHERE "
                    "idVenda = :idVenda LIMIT 1)");
  queryForn.bindValue(":idVenda", idVenda);

  if (not queryForn.exec() or not queryForn.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando dados do fornecedor: " + queryForn.lastError().text());
    return;
  }

  xlsx.write("F4", idVenda); // TODO: temp, replace with correct id
  xlsx.write("E6", queryForn.value("razaoSocial"));
  xlsx.write("E8", queryForn.value("contatoNome"));
  xlsx.write("D10", "Data: " + QDate::currentDate().toString("dd/MM/yy"));

  for (auto const &row : lista) {
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

  QString path = settings("User/userFolder").toString();

  QDir dir(path);

  if (not dir.exists()) {
    dir.mkdir(path);
  }

  if (not xlsx.saveAs(path + "/" + idVenda + ".xlsx")) {
    return;
  }

  QMessageBox::information(this, "Ok!", "Arquivo salvo como " + path + "/" + idVenda + ".xlsx");

  modelItemPedidosPend->setFilter(filtro);

  //------------------------------
  QString arquivo = path + "/" + idVenda + ".xlsx";

  QFile file(arquivo);

  if (not file.exists()) {
    QMessageBox::critical(this, "Erro!", "Arquivo não encontrado");
    return;
  }

  SendMail *mail = new SendMail(this, produtos.join("\n"), arquivo);

  if (mail->exec() != SendMail::Accepted) {
    return;
  }

  for (const auto row : lista) {
    modelItemPedidosPend->setData(row, "selecionado", false);

    // TODO: place this in the beginning
    if (modelItemPedidosPend->data(row, "status").toString() != "PENDENTE") {
      modelItemPedidosPend->select();
      QMessageBox::critical(this, "Erro!", "Produto não estava pendente!");
      return;
    }

    if (not modelItemPedidosPend->setData(row, "status", "EM COMPRA")) {
      QMessageBox::critical(this, "Erro!",
                            "Erro marcando status EM COMPRA: " + modelItemPedidosPend->lastError().text());
      return;
    }

    // TODO: gerar e guardar idCompra
    QSqlQuery queryId;

    if (not queryId.exec("SELECT idCompra FROM pedido_fornecedor_has_produto ORDER BY idCompra DESC")) {
      QMessageBox::critical(this, "Erro!", "Erro buscando idCompra: " + queryId.lastError().text());
      return;
    }

    QString id = "1";

    if (queryId.first()) {
      id = QString::number(queryId.value(0).toInt() + 1);
    }

    if (not modelItemPedidosPend->setData(row, "idCompra", id)) {
      QMessageBox::critical(this, "Erro!", "Erro guardando idCompra: " + modelItemPedidosPend->lastError().text());
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
      QMessageBox::critical(this, "Erro!", "Erro atualizando status da venda: " + query.lastError().text());
      return;
    }
    //

    if (not modelItemPedidosPend->setData(row, "dataRealCompra", dataCompra.toString("yyyy-MM-dd"))) {
      QMessageBox::critical(this, "Erro!",
                            "Erro guardando data da compra: " + modelItemPedidosPend->lastError().text());
      return;
    }

    if (not modelItemPedidosPend->setData(row, "dataPrevConf", dataPrevista.toString("yyyy-MM-dd"))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando data prevista: " + modelItemPedidosPend->lastError().text());
      return;
    }
  }

  QSqlQuery query;
  query.prepare("INSERT INTO conta_a_pagar (idVenda, dataEmissao, pago) VALUES (:idVenda, :dataEmissao, :pago)");
  query.bindValue(":idVenda", idVenda);
  query.bindValue(":dataEmissao", QDate::currentDate().toString("yyyy-MM-dd"));
  query.bindValue(":pago", "NÃO");

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro guardando conta a pagar: " + query.lastError().text());
    return;
  }

  for (auto const &row : lista) {
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
      QMessageBox::critical(this, "Erro!", "Erro guardando conta a pagar pagamento: " + query.lastError().text());
      return;
    }
  }

  if (not modelItemPedidosPend->submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados da tabela pedido_fornecedor_has_produto: " +
                          modelItemPedidosPend->lastError().text());
    return;
  }

  updateTables();
}

void MainWindow::on_pushButtonConfirmarCompra_clicked() {
  QString idCompra;

  if (ui->tablePedidosComp->selectionModel()->selectedRows().size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  int row = ui->tablePedidosComp->selectionModel()->selectedRows().first().row();
  idCompra = modelItemPedidosComp->data(row, "idCompra").toString();

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
    QMessageBox::critical(this, "Erro!", "Erro atualizando status da compra: " + query.lastError().text());
    return;
  }

  // salvar status na venda
  query.prepare("UPDATE venda_has_produto SET dataRealConf = :dataRealConf, dataPrevFat = :dataPrevFat, status = 'EM "
                "FATURAMENTO' WHERE idCompra = :idCompra");
  query.bindValue(":dataRealConf", dataConf);
  query.bindValue(":dataPrevFat", dataPrevista);
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando status da venda: " + query.lastError().text());
    return;
  }
  //

  updateTables();

  QMessageBox::information(this, "Aviso!", "Confirmado compra.");
}

void MainWindow::on_radioButtonProdPendTodos_clicked() {
  modelProdPend->setFilter("");

  ui->tableProdutosPend->resizeColumnsToContents();
}

void MainWindow::on_radioButtonProdPendPend_clicked() {
  modelProdPend->setFilter("status = 'PENDENTE'");

  ui->tableProdutosPend->resizeColumnsToContents();
}

void MainWindow::on_radioButtonProdPendEmCompra_clicked() {
  // TODO: mostrar caixas e un2
  modelProdPend->setFilter("status != 'PENDENTE'");

  ui->tableProdutosPend->resizeColumnsToContents();
}

void MainWindow::on_pushButtonMarcarColetado_clicked() {
  QList<int> lista;

  for (int i = 0; i < modelColeta->rowCount(); ++i) {
    qDebug() << "selecionado: " << modelColeta->data(i, "selecionado");
  }

  // TODO: see why the hell this wont work with true while others tables do
  for (const auto index :
       modelColeta->match(modelColeta->index(0, modelColeta->fieldIndex("selecionado")), Qt::DisplayRole, 1, -1,
                          Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap))) {
    lista.append(index.row());
  }

  if (lista.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
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
      QMessageBox::critical(this, "Erro!", "Erro marcando status EM RECEBIMENTO: " + modelColeta->lastError().text());
      return;
    }

    // salvar status na venda
    QSqlQuery query;

    query.prepare("UPDATE venda_has_produto SET dataRealColeta = :dataRealColeta, dataPrevReceb = :dataPrevReceb, "
                  "status = 'EM RECEBIMENTO' WHERE idCompra = :idCompra");
    query.bindValue(":dataRealColeta", dataColeta);
    query.bindValue(":dataPrevReceb", dataPrevista);
    query.bindValue(":idCompra", modelColeta->data(row, "idCompra"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status da venda: " + query.lastError().text());
      return;
    }
    //

    if (not modelColeta->setData(row, "dataRealColeta", dataColeta.toString("yyyy-MM-dd"))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando data da coleta: " + modelColeta->lastError().text());
      return;
    }

    if (not modelColeta->setData(row, "dataPrevReceb", dataPrevista.toString("yyyy-MM-dd"))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando data prevista: " + modelColeta->lastError().text());
      return;
    }
  }

  if (not modelColeta->submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados da tabela pedido_fornecedor_has_produto: " +
                          modelColeta->lastError().text());
    return;
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
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  InputDialog *inputDlg = new InputDialog(InputDialog::Recebimento, this);

  if (inputDlg->exec() != InputDialog::Accepted) {
    return;
  }

  QDate dataReceb = inputDlg->getDate();

  for (const auto row : lista) {
    modelReceb->setData(row, "selecionado", false);

    if (modelReceb->data(row, "status").toString() != "EM RECEBIMENTO") {
      modelReceb->select();
      QMessageBox::critical(this, "Erro!", "Produto não estava em recebimento!");
      return;
    }

    if (not modelReceb->setData(row, "status", "FINALIZADO")) {
      QMessageBox::critical(this, "Erro!", "Erro marcando status ESTOQUE: " + modelReceb->lastError().text());
      return;
    }

    // salvar status na venda
    QSqlQuery query;

    query.prepare(
          "UPDATE venda_has_produto SET dataRealReceb = :dataRealReceb, status = 'ESTOQUE' WHERE idCompra = :idCompra");
    query.bindValue(":dataRealReceb", dataReceb);
    query.bindValue(":idCompra", modelReceb->data(row, "idCompra"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status da venda: " + query.lastError().text());
      return;
    }
    //

    if (not modelReceb->setData(row, "dataRealReceb", dataReceb.toString("yyyy-MM-dd"))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando data de recebimento: " + modelReceb->lastError().text());
      return;
    }
  }

  // TODO: marcar flag no estoque de produto disponivel

  if (not modelReceb->submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados da tabela pedido_fornecedor_has_produto: " +
                          modelReceb->lastError().text());
    return;
  }

  updateTables();

  QMessageBox::information(this, "Aviso!", "Confirmado recebimento.");
}

void MainWindow::on_pushButtonMarcarFaturado_clicked() {
  QString idCompra;
  QList<int> rows;

  if (ui->tableFaturamento->selectionModel()->selectedRows().size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado.");
    return;
  }

  for (auto const &index : ui->tableFaturamento->selectionModel()->selectedRows()) {
    rows.append(modelFat->data(index.row(), "idCompra").toInt());
  }

  int row = ui->tableFaturamento->selectionModel()->selectedRows().first().row();
  idCompra = modelFat->data(row, "idCompra").toString();

  ImportarXML *import = new ImportarXML(rows, this);
  import->show();

  if (import->exec() != QDialog::Accepted) {
    return;
  }

  //----------------------------------------------------------//

  InputDialog *inputDlg = new InputDialog(InputDialog::ConfirmarCompra, this);
  inputDlg->setFilter(idCompra);

  if (inputDlg->exec() != InputDialog::Accepted) {
    return;
  }

  QDate dataFat = inputDlg->getDate();
  QDate dataPrevista = inputDlg->getNextDate();

  QSqlQuery query;
  query.prepare("UPDATE pedido_fornecedor_has_produto SET dataRealFat = :dataRealFat, dataPrevColeta = "
                ":dataPrevColeta, status = 'EM COLETA' WHERE idCompra = :idCompra");
  query.bindValue(":dataRealFat", dataFat);
  query.bindValue(":dataPrevColeta", dataPrevista);
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status da compra: " + query.lastError().text());
    return;
  }

  // salvar status na venda
  query.prepare("UPDATE venda_has_produto SET dataRealFat = :dataRealFat, dataPrevColeta = :dataPrevColeta, status = "
                "'EM COLETA' WHERE idCompra = :idCompra");
  query.bindValue(":dataRealFat", dataFat);
  query.bindValue(":dataPrevColeta", dataPrevista);
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando status da venda: " + query.lastError().text());
    return;
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
  QString fornecedor = modelPedForn->data(index.row(), "fornecedor").toString();

  modelColeta->setFilter("fornecedor = '" + fornecedor + "' AND status = 'EM COLETA'");

  if (not modelColeta->select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + modelColeta->lastError().text());
    return;
  }

  modelReceb->setFilter("fornecedor = '" + fornecedor + "' AND status = 'EM RECEBIMENTO'");

  if (not modelReceb->select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + modelReceb->lastError().text());
    return;
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
    QMessageBox::critical(this, "Erro!", "Erro abrindo arquivo: " + file.errorString());
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

void MainWindow::setHomologacao(bool value) { homologacao = value; }

void MainWindow::on_tabWidget_currentChanged(int) { updateTables(); }

void MainWindow::on_tabWidget_2_currentChanged(int) { updateTables(); }

void MainWindow::on_tabWidget_3_currentChanged(int) { updateTables(); }

void MainWindow::on_tabWidget_4_currentChanged(int) { updateTables(); }

void MainWindow::on_groupBoxStatusVenda_toggled(bool enabled) {
  for (auto const &child : ui->groupBoxStatusVenda->findChildren<QCheckBox *>()) {
    child->setEnabled(true);
  }

  for (auto const &child : ui->groupBoxStatusVenda->findChildren<QCheckBox *>()) {
    child->setChecked(enabled);
  }
}

void MainWindow::montaFiltroVendas() {
  QString loja = ui->groupBoxLojas->isVisible() ? ui->comboBoxLojas->currentText() : UserSession::getSiglaLoja();

  int counter = 0;

  for (auto const &child : ui->groupBoxStatusVenda->findChildren<QCheckBox *>()) {
    if (not child->isChecked()) {
      counter++;
    }
  }

  if (counter == ui->groupBoxStatusVenda->findChildren<QCheckBox *>().size()) {
    if (ui->radioButtonVendLimpar->isChecked()) {
      modelVendas->setFilter("(Código LIKE '%" + loja + "%')");
    }

    if (ui->radioButtonVendProprios->isChecked()) {
      modelVendas->setFilter("(Código LIKE '%" + loja + "%') AND Vendedor = '" + UserSession::getNome() + "'");
    }

    ui->tableVendas->resizeColumnsToContents();

    return;
  }

  QString filtro;

  if (ui->radioButtonVendLimpar->isChecked()) {
    filtro = "(Código LIKE '%" + loja + "%')";
  }

  if (ui->radioButtonVendProprios->isChecked()) {
    filtro = "(Código LIKE '%" + loja + "%') AND Vendedor = '" + UserSession::getNome() + "'";
  }

  QString filtro2;

  for (auto const &child : ui->groupBoxStatusVenda->findChildren<QCheckBox *>()) {
    if (child->isChecked()) {
      filtro2 += filtro2.isEmpty() ? "status = '" + child->text().toUpper() + "'"
                                   : " OR status = '" + child->text().toUpper() + "'";
    }
  }

  if (not filtro2.isEmpty()) {
    //    qDebug() << "filtro = " << filtro + " AND (" + filtro2 + ")";
    modelVendas->setFilter(filtro + " AND (" + filtro2 + ")");
  }

  ui->tableVendas->resizeColumnsToContents();
}

void MainWindow::on_comboBoxLojas_currentTextChanged(const QString &) { montaFiltroVendas(); }

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
