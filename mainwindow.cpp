#include <QCheckBox>
#include <QDataWidgetMapper>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlTableModel>
#include <QtSql>

#include "backgroundproxymodel.h"
#include "cadastrocliente.h"
#include "cadastroloja.h"
#include "cadastroproduto.h"
#include "cadastroprofissional.h"
#include "cadastrotransportadora.h"
#include "cadastrofornecedor.h"
#include "cadastrousuario.h"
#include "contasapagar.h"
#include "contasareceber.h"
#include "entregascliente.h"
#include "importabd.h"
#include "initdb.h"
#include "logindialog.h"
#include "mainwindow.h"
#include "orcamento.h"
#include "pedidoscompra.h"
#include "recebimentosfornecedor.h"
#include "sendmail.h"
#include "ui_mainwindow.h"
#include "usersession.h"
#include "venda.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent), ui(new Ui::MainWindow), modelOrcamento(nullptr), modelCAPagar(nullptr),
    modelCAReceber(nullptr), modelVendas(nullptr), modelPedCompra(nullptr) {
  ui->setupUi(this);
  setWindowTitle("ERP Staccato");

  readSettings();

#ifdef QT_DEBUG
  if (!dbConnect()) {
    exit(1);
  } else if (!UserSession::login(
               "admin", "1234")) { // Para desabilitar o login comente o bloco anterior e descomente este
    //                 bloco!
    QMessageBox::critical(this, "Atenção!", "Login inválido!", QMessageBox::Ok, QMessageBox::NoButton);
    exit(1);
  }
#else
  LoginDialog *dialog = new LoginDialog(this);
  if (dialog->exec() == QDialog::Rejected) {
    exit(1);
  }
#endif

  modelOrcamento = new QSqlTableModel(this);
  modelVendas = new QSqlRelationalTableModel(this);
  modelCAPagar = new QSqlTableModel(this);
  modelCAReceber = new QSqlTableModel(this);
  modelEntregasCliente = new QSqlTableModel(this);
  modelRecebimentosForn = new QSqlTableModel(this);
  modelPedCompra = new QSqlRelationalTableModel(this);
  modelNFe = new QSqlTableModel(this);

  initializeTables();

  ui->tabWidget->setTabEnabled(2, false);
  ui->tabWidget->setTabEnabled(3, false);
  ui->tabWidget->setTabEnabled(4, false);
  ui->tabWidget->setTabEnabled(5, false);

  setWindowTitle(windowTitle() + " - " + UserSession::getNome() + " - " + UserSession::getTipo());

  if (UserSession::getTipo() == "VENDEDOR") {
    ui->tableContasPagar->hide();
    ui->labelContasPagar->hide();
    ui->tableContasReceber->hide();
    ui->labelContasReceber->hide();
    ui->tableEntregasCliente->hide();
    ui->labelEntregasCliente->hide();
    ui->tableRecebimentosFornecedor->hide();
    ui->labelRecebimentosFornecedor->hide();
    ui->tableNFE->hide();
    ui->labelNFE->hide();
    ui->tabWidget->setTabEnabled(2,false);
    ui->tabWidget->setTabEnabled(3,false);
    ui->tabWidget->setTabEnabled(4,false);
    ui->tabWidget->setTabEnabled(5,false);
    ui->tablePedidosCompra->hide();
    ui->labelPedidosCompra->hide();
    ui->actionCadastrarUsuario->setVisible(false);
    //    ui->actionRestaurar_BD->setVisible(false);

    ui->radioButtonOrcValido->setChecked(true);
    on_radioButtonOrcValido_clicked();
  }
}

bool MainWindow::dbConnect() {
  if (!QSqlDatabase::drivers().contains("QMYSQL")) {
    QMessageBox::critical(this, "Não foi possível carregar o banco de dados",
                          "Este aplicativo requer o driver QMYSQL");
    exit(1);
  }

  qDebug() << "Connecting to database";

  QSqlDatabase::removeDatabase("QMYSQL");
  QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
  //  qDebug() << "hostname: " << hostname;
  //  qDebug() << "username: " << username;
  //  qDebug() << "password: " << password;
  db.setHostName(hostname);
  db.setUserName(username);
  db.setPassword(password);
  db.setDatabaseName("mysql");
  if (db.open()) {
    QSqlQuery qry = db.exec("SHOW SCHEMAS");
    bool hasMydb = false;
    while (qry.next()) {
      if (qry.value(0).toString() == "mydb")
        hasMydb = true;
    }
    if (!hasMydb) {
      //      QMessageBox::critical(this, "Atenção", "Banco de dados não encontrado!", QMessageBox::Ok,
      //                            QMessageBox::NoButton);
      //      close();
      qDebug() << "mydb schema not found!";
      if(!initDb()){
        qDebug() << "initDb Error";
        return false;
      }
      //      QSqlError err = initDb(); // Para desativar criação do bd comentar aqui
      //      if (err.type() != QSqlError::NoError) {
      //        showError(err);
      //        return false;
      //      }
    }
    db.close();
    db.setDatabaseName("mydb");
    if (db.open()) {
      qDebug() << "mydb schema found!";
      return true;
    } else {
      showError(db.lastError());
      return false;
    }
  } else {
    switch (db.lastError().number()) {
    case 1045:
      QMessageBox::critical(this, "ERRO: Banco de dados inacessível!", "Verifique se o usuário e senha do banco de dados estão corretos.");
      break;
    case 2002:
      QMessageBox::critical(this, "ERRO: Banco de dados inacessível!", "Verifique se o servidor está ligado, e acessível pela rede.");
      break;
    case 2003:
      QMessageBox::critical(this, "ERRO: Banco de dados inacessível!", "Verifique se o servidor está ligado, e acessível pela rede.");
      break;
    case 2005:
      QMessageBox::critical(this, "ERRO: Banco de dados inacessível!", "Verifique se o IP do servidor foi escrito corretamente.");
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

void MainWindow::setupTable(QTableView *table) {

  table->resizeColumnsToContents();

  table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

void MainWindow::showError(const QSqlError &err) {
  qDebug() << err;
  QMessageBox::critical(this, "Erro", "Erro inicializando o banco de dados: " + err.text());
}

void MainWindow::on_actionCriarOrcamento_triggered() {
  Orcamento *orcamento = new Orcamento(this);
  connect(orcamento, &Orcamento::finished, this, &MainWindow::updateTables);
}

QString MainWindow::getPort() const {
  return port;
}

void MainWindow::setPort(const QString &value) {
  port = value;
}

QString MainWindow::getPassword() const {
  return password;
}

void MainWindow::setPassword(const QString &value) {
  password = value;
}

QString MainWindow::getUsername() const {
  return username;
}

void MainWindow::setUsername(const QString &value) {
  username = value;
}

QString MainWindow::getHostname() const {
  return hostname;
}

void MainWindow::setHostname(const QString &value) {
  hostname = value;
}

void MainWindow::on_actionCadastrarProdutos_triggered() {
  CadastroProduto *cad = new CadastroProduto(this);
  cad->show();
  cad->adjustSize();
}

void MainWindow::on_actionCadastrarCliente_triggered() {
  CadastroCliente *cad = new CadastroCliente(this);
  cad->show();
}

void MainWindow::initializeTables() {
  modelOrcamento->setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelVendas->setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelCAPagar->setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelCAReceber->setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelEntregasCliente->setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelPedCompra->setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelNFe->setEditStrategy(QSqlTableModel::OnManualSubmit);

  // Orçamento -------------------------------------
  modelOrcamento->setTable("ViewOrcamento");
  modelOrcamento->setSort(modelOrcamento->fieldIndex("Dias Restantes"), Qt::DescendingOrder);
  if (!modelOrcamento->select()) {
    qDebug() << "Failed to populate TableOrcamento! " << modelOrcamento->lastError();
  }

  //  ui->tableOrcamentos->setModel(modelOrcamento);
  BackgroundProxyModel *proxyModel = new BackgroundProxyModel(modelOrcamento->fieldIndex("Dias Restantes"));
  proxyModel->setSourceModel(modelOrcamento);
  ui->tableOrcamentos->setModel(proxyModel);
  ui->tableOrcamentos->setColumnHidden(modelOrcamento->fieldIndex("idUsuario"), true);
  setupTable(ui->tableOrcamentos);

  // Vendas -------------------------------------
  //  modelVendas->setTable("ViewVendas");
  modelVendas->setTable("Venda");
  modelVendas->setRelation(modelVendas->fieldIndex("idLoja"), QSqlRelation("Loja", "idLoja", "descricao"));
  modelVendas->setRelation(modelVendas->fieldIndex("idUsuario"),
                           QSqlRelation("Usuario", "idUsuario", "nome"));
  modelVendas->setRelation(modelVendas->fieldIndex("idCadastroCliente"),
                           QSqlRelation("Cliente", "idCliente", "nome_razao"));
  modelVendas->setRelation(modelVendas->fieldIndex("idEnderecoEntrega"),
                           QSqlRelation("Endereco", "idEndereco", "logradouro"));
  modelVendas->setRelation(modelVendas->fieldIndex("idProfissional"),
                           QSqlRelation("Profissional", "idProfissional", "nome"));
  modelVendas->setHeaderData(modelVendas->fieldIndex("idLoja"), Qt::Horizontal, "Loja");
  modelVendas->setHeaderData(modelVendas->fieldIndex("idUsuario"), Qt::Horizontal, "Vendedor");
  modelVendas->setHeaderData(modelVendas->fieldIndex("idCadastroCliente"), Qt::Horizontal, "Cliente");
  modelVendas->setHeaderData(modelVendas->fieldIndex("idEnderecoEntrega"), Qt::Horizontal, "Endereço");
  modelVendas->setHeaderData(modelVendas->fieldIndex("idProfissional"), Qt::Horizontal, "Profissional");
  modelVendas->setHeaderData(modelVendas->fieldIndex("data"), Qt::Horizontal, "Data");
  modelVendas->setHeaderData(modelVendas->fieldIndex("total"), Qt::Horizontal, "Total");
  modelVendas->setHeaderData(modelVendas->fieldIndex("desconto"), Qt::Horizontal, "Desc.");
  modelVendas->setHeaderData(modelVendas->fieldIndex("frete"), Qt::Horizontal, "Frete");
  modelVendas->setHeaderData(modelVendas->fieldIndex("validade"), Qt::Horizontal, "Validade");
  modelVendas->setHeaderData(modelVendas->fieldIndex("status"), Qt::Horizontal, "Status");
  if (!modelVendas->select()) {
    qDebug() << "Failed to populate TableVendas! " << modelVendas->lastError();
  }

  ui->tableVendas->setModel(modelVendas);
  ui->tableVendas->setItemDelegate(new QSqlRelationalDelegate(ui->tableVendas));
  setupTable(ui->tableVendas);

  // Contas a pagar -------------------------------------
  modelCAPagar->setTable("ContaAPagar");
  if (!modelCAPagar->select()) {
    qDebug() << "Failed to populate TableContasPagar! " << modelCAPagar->lastError();
  }

  ui->tableContasPagar->setModel(modelCAPagar);
  setupTable(ui->tableContasPagar);

  // Contas a receber -------------------------------------
  modelCAReceber->setTable("ContaAReceber");
  if (!modelCAReceber->select()) {
    qDebug() << "Failed to populate TableContasReceber! " << modelCAReceber->lastError();
  }

  ui->tableContasReceber->setModel(modelCAReceber);
  setupTable(ui->tableContasReceber);

  // Entregas cliente
  modelEntregasCliente->setTable("PedidoTransportadora");
  if (!modelEntregasCliente->select()) {
    qDebug() << "Failed to populate TableEntregasCliente";
  }
  modelEntregasCliente->setFilter("tipo = 'CLIENTE'"); // filter to clientes only

  ui->tableEntregasCliente->setModel(modelEntregasCliente);
  setupTable(ui->tableEntregasCliente);

  // Recebimentos fornecedor
  modelRecebimentosForn->setTable("PedidoTransportadora");
  if (!modelRecebimentosForn->select()) {
    qDebug() << "Failed to populate TableRecebimentosFornecedor";
  }
  modelRecebimentosForn->setFilter("tipo = 'FORNECEDOR'"); // filter to fornecedor only

  ui->tableRecebimentosFornecedor->setModel(modelRecebimentosForn);
  setupTable(ui->tableRecebimentosFornecedor);

  // Pedidos de compra
  modelPedCompra->setTable("PedidoFornecedor");
  modelPedCompra->setRelation(modelPedCompra->fieldIndex("idLoja"),
                              QSqlRelation("Loja", "idLoja", "descricao"));
  modelPedCompra->setHeaderData(modelPedCompra->fieldIndex("idLoja"), Qt::Horizontal, "Loja");
  modelPedCompra->setRelation(modelPedCompra->fieldIndex("idUsuario"),
                              QSqlRelation("Usuario", "idUsuario", "nome"));
  modelPedCompra->setHeaderData(modelPedCompra->fieldIndex("idUsuario"), Qt::Horizontal, "Vendedor");
  modelPedCompra->setRelation(modelPedCompra->fieldIndex("idCadastroCliente"),
                              QSqlRelation("Cliente", "idCliente", "nome_razao"));
  modelPedCompra->setHeaderData(modelPedCompra->fieldIndex("idCadastroCliente"), Qt::Horizontal, "Cliente");
  modelPedCompra->setRelation(modelPedCompra->fieldIndex("idEnderecoEntrega"),
                              QSqlRelation("Endereco", "idEndereco", "logradouro"));
  modelPedCompra->setHeaderData(modelPedCompra->fieldIndex("idEnderecoEntrega"), Qt::Horizontal, "Endereço");
  modelPedCompra->setRelation(modelPedCompra->fieldIndex("idProfissional"),
                              QSqlRelation("Profissional", "idProfissional", "nome"));
  modelPedCompra->setHeaderData(modelPedCompra->fieldIndex("idProfissional"), Qt::Horizontal, "Profissional");
  if (!modelPedCompra->select()) {
    qDebug() << "Failed to populate TablePedidosCompra!" << modelPedCompra->lastError();
  }

  ui->tablePedidosCompra->setModel(modelPedCompra);
  setupTable(ui->tablePedidosCompra);

  // NFe
  modelNFe->setTable("NFe");
  if (!modelNFe->select()) {
    qDebug() << "Failed to populate TableNFe" << modelNFe->lastError();
  }

  ui->tableNFE->setModel(modelNFe);
  ui->tableNFE->hideColumn(1); // NFe
  setupTable(ui->tableNFE);

  //  paintRows();
}

void MainWindow::on_actionCadastrarUsuario_triggered() {
  CadastroUsuario *cad = new CadastroUsuario(this);
  cad->show();
  cad->adjustSize();
}

void MainWindow::on_actionCadastrarProfissional_triggered() {
  CadastroProfissional *cad = new CadastroProfissional(this);
  cad->show();
  cad->adjustSize();
}

// void MainWindow::on_actionRestaurar_BD_triggered() {
//  QSqlError err = initDb();
//  if (err.type() != QSqlError::NoError) {
//    showError(err);
//  }
//  initializeTables();
//  QMessageBox::information(this, "Atenção", "Banco de dados redefinido com sucesso!", QMessageBox::Ok,
//                           QMessageBox::NoButton);
//}

void MainWindow::on_actionGerenciar_Transportadoras_triggered() {
  CadastroTransportadora *cad = new CadastroTransportadora(this);
  cad->show();
  cad->adjustSize();
}

void MainWindow::on_actionGerenciar_Lojas_triggered() {
  CadastroLoja *cad = new CadastroLoja(this);
  cad->show();
  cad->adjustSize();
}

void MainWindow::updateTables() {
  modelCAPagar->select();
  modelCAReceber->select();
  modelOrcamento->select();
  modelVendas->select();
  modelPedCompra->select();
  modelEntregasCliente->select();
  modelRecebimentosForn->select();
  modelNFe->select();

  ui->tableContasPagar->resizeColumnsToContents();
  ui->tableContasReceber->resizeColumnsToContents();
  ui->tableRecebimentosFornecedor->resizeColumnsToContents();
  ui->tableEntregasCliente->resizeColumnsToContents();
  ui->tableOrcamentos->resizeColumnsToContents();
  ui->tablePedidosCompra->resizeColumnsToContents();
  ui->tableVendas->resizeColumnsToContents();
  ui->tableNFE->resizeColumnsToContents();
}

void MainWindow::on_actionImportarTabelaFornecedor_triggered() {
  new ImportaBD(this);
}

void MainWindow::on_tableOrcamentos_doubleClicked(const QModelIndex &index) {
  Orcamento *orc = new Orcamento(this);
  connect(orc, &Orcamento::finished, this, &MainWindow::updateTables);
  //    qDebug() << "index: " << modelOrcamento->fieldIndex("Código");
  orc->viewRegisterById(
        modelOrcamento->data(modelOrcamento->index(index.row(), modelOrcamento->fieldIndex("Código"))));
}

void MainWindow::on_tableContasPagar_doubleClicked(const QModelIndex &index) {
  ContasAPagar *contas = new ContasAPagar(this);
  contas->viewConta(
        modelCAPagar->data(modelCAPagar->index(index.row(), modelCAPagar->fieldIndex("idVenda"))).toString());
}

void MainWindow::on_tableContasReceber_doubleClicked(const QModelIndex &index) {
  ContasAReceber *contas = new ContasAReceber(this);
  contas->viewConta(modelCAReceber->data(modelCAReceber->index(
                                           index.row(), modelCAReceber->fieldIndex("idVenda"))).toString());
}

void MainWindow::on_tablePedidosCompra_doubleClicked(const QModelIndex &index) {
  PedidosCompra *pedidos = new PedidosCompra(this);
  pedidos->viewPedido(modelPedCompra->data(modelPedCompra->index(index.row(), 0)).toString());
}

void MainWindow::on_actionAtualizar_tabelas_triggered() {
  updateTables();
}

void MainWindow::on_tableVendas_doubleClicked(const QModelIndex &index) {
  Venda *vendas = new Venda(this);
  connect(vendas, &Venda::finished, this, &MainWindow::updateTables);
  vendas->viewRegisterById(
        modelVendas->data(modelVendas->index(index.row(), modelVendas->fieldIndex("idVenda"))));
}

void MainWindow::on_radioButtonOrcValido_clicked() {
  if (UserSession::getTipo() == "VENDEDOR") {
    modelOrcamento->setFilter("`Dias restantes` > 0 AND status != 'CANCELADO' AND idUsuario = " +
                              QString::number(UserSession::getId()) + "");
  } else {
    modelOrcamento->setFilter("`Dias restantes` > 0 AND status != 'CANCELADO'");
  }
}

void MainWindow::on_radioButtonOrcExpirado_clicked() {
  if (UserSession::getTipo() == "VENDEDOR") {
    modelOrcamento->setFilter("`Dias restantes` < 1 AND idUsuario = " +
                              QString::number(UserSession::getId()) + "");
  } else {
    modelOrcamento->setFilter("`Dias restantes` < 1");
  }
}

void MainWindow::on_radioButtonOrcLimpar_clicked() {
  if (UserSession::getTipo() == "VENDEDOR") {
    modelOrcamento->setFilter("idUsuario = " + QString::number(UserSession::getId()) + "");
  } else {
    modelOrcamento->setFilter("");
  }
}

void MainWindow::on_radioButtonVendAberto_clicked() {
  modelVendas->setFilter("status = 'aberto'");
}

void MainWindow::on_radioButtonVendFechado_clicked() {
  modelVendas->setFilter("status = 'fechado'");
}

void MainWindow::on_radioButtonVendLimpar_clicked() {
  modelVendas->setFilter("");
}

void MainWindow::on_radioButtonNFeAutorizado_clicked() {
  modelNFe->setFilter("status = 'autorizado'");
}

void MainWindow::on_radioButtonNFeEnviado_clicked() {
  modelNFe->setFilter("status = 'enviado'");
}

void MainWindow::on_radioButtonNFeLimpar_clicked() {
  modelNFe->setFilter("");
}

void MainWindow::on_radioButtonFornLimpar_clicked() {
  modelPedCompra->setFilter("");
}

void MainWindow::on_radioButtonFornAberto_clicked() {
  modelPedCompra->setFilter("status = 'aberto'");
}

void MainWindow::on_radioButtonFornFechado_clicked() {
  modelPedCompra->setFilter("status = 'fechado'");
}

void MainWindow::on_radioButtonRecebimentoLimpar_clicked() {
  modelRecebimentosForn->setFilter("tipo = 'fornecedor'");
}

void MainWindow::on_radioButtonRecebimentoRecebido_clicked() {
  modelRecebimentosForn->setFilter("status = 'recebido' AND tipo = 'fornecedor'");
}

void MainWindow::on_radioButtonRecebimentoPendente_clicked() {
  modelRecebimentosForn->setFilter("status = 'pendente' AND tipo = 'fornecedor'");
}

void MainWindow::on_radioButtonEntregaLimpar_clicked() {
  modelEntregasCliente->setFilter("tipo = 'cliente'");
}

void MainWindow::on_radioButtonEntregaEnviado_clicked() {
  modelEntregasCliente->setFilter("status = 'enviado' AND tipo = 'cliente'");
}

void MainWindow::on_radioButtonEntregaPendente_clicked() {
  modelEntregasCliente->setFilter("status = 'pendente' AND tipo = 'cliente'");
}

void MainWindow::on_radioButtonContaPagarLimpar_clicked() {
  modelCAPagar->setFilter("");
}

void MainWindow::on_radioButtonContaPagarPago_clicked() {
  modelCAPagar->setFilter("pago = 'sim'");
}

void MainWindow::on_radioButtonContaPagarPendente_clicked() {
  modelCAPagar->setFilter("pago = 'não'");
}

void MainWindow::on_radioButtonContaReceberLimpar_clicked() {
  modelCAReceber->setFilter("");
}

void MainWindow::on_radioButtonContaReceberRecebido_clicked() {
  modelCAReceber->setFilter("pago = 'sim'");
}

void MainWindow::on_radioButtonContaReceberPendente_clicked() {
  modelCAReceber->setFilter("pago = 'não'");
}

void MainWindow::on_tableRecebimentosFornecedor_doubleClicked(const QModelIndex &index) {
  RecebimentosFornecedor *recebimentos = new RecebimentosFornecedor(this);
  recebimentos->viewRecebimento(
        modelRecebimentosForn->data(modelRecebimentosForn->index(index.row(), modelRecebimentosForn->fieldIndex(
                                                                   "idPedido"))).toString());
}

void MainWindow::on_tableEntregasCliente_doubleClicked(const QModelIndex &index) {
  EntregasCliente *entregas = new EntregasCliente(this);
  entregas->viewEntrega(
        modelEntregasCliente->data(modelEntregasCliente->index(
                                     index.row(), modelEntregasCliente->fieldIndex("idPedido"))).toString());
}

void MainWindow::on_tableNFE_doubleClicked(const QModelIndex &index) {
  Q_UNUSED(index);
}

void MainWindow::on_pushButtonCriarOrc_clicked() {
  on_actionCriarOrcamento_triggered();
}

void MainWindow::on_lineEditBuscaOrcamentos_textChanged(const QString &text) {
  if(text.isEmpty()) {
    modelOrcamento->setFilter("");
  } else {
    modelOrcamento->setFilter("(Código LIKE '%" + text + "%')");
  }
}

void MainWindow::on_lineEditBuscaVendas_textChanged(const QString &text) {
  if(text.isEmpty()) {
    modelVendas->setFilter("");
  } else {
    modelVendas->setFilter("(idVenda LIKE '%" + text + "%') OR (Cliente LIKE '%" + text + "%')");
  }
}

void MainWindow::on_lineEditBuscaContasPagar_textChanged(const QString &text) {
  if(text.isEmpty()) {
    modelCAPagar->setFilter("");
  } else {
    modelCAPagar->setFilter("(idVenda LIKE '%" + text + "%') OR (pago LIKE '%" + text + "%')");
  }
}

void MainWindow::on_lineEditBuscaContasReceber_textChanged(const QString &text) {
  if(text.isEmpty()) {
    modelCAReceber->setFilter("");
  } else {
    modelCAReceber->setFilter("(idVenda LIKE '%" + text + "%') OR (pago LIKE '%" + text + "%')");
  }
}

void MainWindow::on_lineEditBuscaEntregas_textChanged(const QString &text) {
  if(text.isEmpty()) {
    modelEntregasCliente->setFilter("");
  } else {
    modelEntregasCliente->setFilter("(idPedido LIKE '%" + text + "%') OR (status LIKE '%" + text + "%')");
  }
}

void MainWindow::on_lineEditBuscaProdutosPend_textChanged(const QString &text) {
  if(text.isEmpty()) {
    modelPedCompra->setFilter("");
  } else {
    modelPedCompra->setFilter("(Cliente LIKE '%" + text + "%') OR (status LIKE '%" + text + "%')");
  }
}

void MainWindow::on_lineEditBuscaRecebimentos_textChanged(const QString &text) {
  if(text.isEmpty()) {
    modelRecebimentosForn->setFilter("");
  } else {
    modelRecebimentosForn->setFilter("(idPedido LIKE '%" + text + "%') OR (status LIKE '%" + text + "%')");
  }
}

void MainWindow::on_lineEditBuscaNFe_textChanged(const QString &text) {
  if(text.isEmpty()) {
    modelNFe->setFilter("");
  } else {
    modelNFe->setFilter("(idVenda LIKE '%" + text + "%') OR (status LIKE '%" + text + "%')");
  }
}

void MainWindow::on_actionCadastrarFornecedor_triggered() {
  CadastroFornecedor *cad = new CadastroFornecedor(this);
  cad->setWindowTitle("Cadastrar fornecedor");
  cad->show();
  cad->adjustSize();
}

void MainWindow::readSettings() {
  QSettings settings("ERP", "Staccato");
  settings.beginGroup("Login");
  if(!settings.contains("hostname")) {
    settings.setValue("hostname", QString("localhost"));
    settings.setValue("username", QString("test"));
    settings.setValue("password", QString("1234"));
    settings.setValue("port", QString("3306"));
  }
  hostname = settings.value("hostname").toString();
  username = settings.value("username").toString();
  password = settings.value("password").toString();
  port = settings.value("port").toString();
  settings.endGroup();
}

void MainWindow::on_actionRestaurar_BD_triggered()
{
  if(!initDb()){
    qDebug() << "initDb Error";
  } else{
    QMessageBox::information(this, "Aviso!", "Banco de dados inicializado com sucesso.");
    updateTables();
  }
}
