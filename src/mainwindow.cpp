#include <QCheckBox>
#include <QDataWidgetMapper>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QShortcut>
#include <QSqlTableModel>
#include <QtSql>

#include "porcentagemdelegate.h"
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
#include "importaprodutos.h"
#include "doubledelegate.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  setWindowTitle("ERP Staccato");

  readSettings();

#ifdef QT_DEBUG
  if (not dbConnect()) {
    exit(1);
  } else if (not UserSession::login("admin", "1234")) {
    QMessageBox::critical(this, "Atenção!", "Login inválido!", QMessageBox::Ok, QMessageBox::NoButton);
    exit(1);
  }
#else
  LoginDialog *dialog = new LoginDialog(this);
  if (dialog->exec() == QDialog::Rejected) {
    exit(1);
  }
#endif

  QShortcut *shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this);
  connect(shortcut, &QShortcut::activated, this, &QWidget::close);

  initializeTables();

  setWindowTitle(windowTitle() + " - " + UserSession::getNome() + " - " + UserSession::getTipoUsuario());

  if (UserSession::getTipoUsuario() == "VENDEDOR") {
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
    ui->tabWidget->setTabEnabled(2, false);
    ui->tabWidget->setTabEnabled(3, false);
    ui->tabWidget->setTabEnabled(4, false);
    ui->tabWidget->setTabEnabled(5, false);
    ui->tablePedidosCompra->hide();
    ui->labelPedidosCompra->hide();
    ui->actionCadastrarUsuario->setVisible(false);

    ui->radioButtonOrcValido->setChecked(true);
    on_radioButtonOrcValido_clicked();
  }
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
//      qDebug() << "mydb schema not found.";
      if (not initDb()) {
        qDebug() << "initDb Error";
        return false;
      }
    }

    db.close();
    db.setDatabaseName("mydb");

    if (db.open()) {
      QSqlQuery queryProcedure;

      if (not queryProcedure.exec("CALL InvalidateExpired()")) {
        qDebug() << "Erro executando procedure InvalidateExpired: " << queryProcedure.lastError();
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

void MainWindow::setupTable(QTableView *table) {
  table->verticalHeader()->setResizeContentsPrecision(0);
  table->horizontalHeader()->setResizeContentsPrecision(0);
  table->resizeColumnsToContents();
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

void MainWindow::initializeTables() {
  DoubleDelegate *doubledelegate = new DoubleDelegate(this);

  // Orçamento -------------------------------------
  modelOrcamento = new QSqlTableModel(this);
  modelOrcamento->setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelOrcamento->setTable("ViewOrcamento");
  modelOrcamento->setSort(modelOrcamento->fieldIndex("Dias Restantes"), Qt::DescendingOrder);

  if (not modelOrcamento->select()) {
    qDebug() << "Failed to populate TableOrcamento: " << modelOrcamento->lastError();
    return;
  }

  BackgroundProxyModel *proxyModel = new BackgroundProxyModel(modelOrcamento->fieldIndex("Dias Restantes"));
  proxyModel->setSourceModel(modelOrcamento);
  ui->tableOrcamentos->setModel(proxyModel);
  ui->tableOrcamentos->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableOrcamentos->setColumnHidden(modelOrcamento->fieldIndex("idUsuario"), true);
  ui->tableOrcamentos->setItemDelegate(doubledelegate);

  // Vendas -------------------------------------
  modelVendas = new QSqlRelationalTableModel(this);
  modelVendas->setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelVendas->setTable("Venda");
  modelVendas->setRelation(modelVendas->fieldIndex("idLoja"), QSqlRelation("Loja", "idLoja", "descricao"));
  modelVendas->setRelation(modelVendas->fieldIndex("idUsuario"), QSqlRelation("Usuario", "idUsuario", "nome"));
  modelVendas->setRelation(modelVendas->fieldIndex("idCliente"), QSqlRelation("Cliente", "idCliente", "nome_razao"));
  modelVendas->setRelation(modelVendas->fieldIndex("idEnderecoEntrega"),
                           QSqlRelation("Cliente_has_Endereco", "idEndereco", "logradouro"));
  modelVendas->setRelation(modelVendas->fieldIndex("idProfissional"),
                           QSqlRelation("Profissional", "idProfissional", "nome_razao"));
  modelVendas->setHeaderData(modelVendas->fieldIndex("idLoja"), Qt::Horizontal, "Loja");
  modelVendas->setHeaderData(modelVendas->fieldIndex("idUsuario"), Qt::Horizontal, "Vendedor");
  modelVendas->setHeaderData(modelVendas->fieldIndex("idCliente"), Qt::Horizontal, "Cliente");
  modelVendas->setHeaderData(modelVendas->fieldIndex("idEnderecoEntrega"), Qt::Horizontal, "Endereço");
  modelVendas->setHeaderData(modelVendas->fieldIndex("idProfissional"), Qt::Horizontal, "Profissional");
  modelVendas->setHeaderData(modelVendas->fieldIndex("data"), Qt::Horizontal, "Data");
  modelVendas->setHeaderData(modelVendas->fieldIndex("total"), Qt::Horizontal, "Total");
  modelVendas->setHeaderData(modelVendas->fieldIndex("desconto"), Qt::Horizontal, "Desc.");
  modelVendas->setHeaderData(modelVendas->fieldIndex("frete"), Qt::Horizontal, "Frete");
  modelVendas->setHeaderData(modelVendas->fieldIndex("validade"), Qt::Horizontal, "Validade");
  modelVendas->setHeaderData(modelVendas->fieldIndex("status"), Qt::Horizontal, "Status");

  modelVendas->setHeaderData(modelVendas->fieldIndex("subTotalBru"), Qt::Horizontal, "Bruto");
  modelVendas->setHeaderData(modelVendas->fieldIndex("subTotalLiq"), Qt::Horizontal, "Líquido");
  modelVendas->setHeaderData(modelVendas->fieldIndex("descontoPorc"), Qt::Horizontal, "Desc.");
  modelVendas->setHeaderData(modelVendas->fieldIndex("descontoReais"), Qt::Horizontal, "Desc.");

  if (not modelVendas->select()) {
    qDebug() << "Failed to populate TableVendas: " << modelVendas->lastError();
    return;
  }

  ui->tableVendas->setModel(modelVendas);
  ui->tableVendas->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableVendas->setColumnHidden(modelVendas->fieldIndex("idEnderecoFaturamento"), true);
  ui->tableVendas->setItemDelegateForColumn(modelVendas->fieldIndex("subTotalBru"), doubledelegate);
  ui->tableVendas->setItemDelegateForColumn(modelVendas->fieldIndex("subTotalLiq"), doubledelegate);
  ui->tableVendas->setItemDelegateForColumn(modelVendas->fieldIndex("frete"), doubledelegate);
  ui->tableVendas->setItemDelegateForColumn(modelVendas->fieldIndex("descontoReais"), doubledelegate);
  ui->tableVendas->setItemDelegateForColumn(modelVendas->fieldIndex("total"), doubledelegate);
  ui->tableVendas->setItemDelegateForColumn(modelVendas->fieldIndex("descontoPorc"), new PorcentagemDelegate);

  // Contas a pagar -------------------------------------
  modelCAPagar = new QSqlTableModel(this);
  modelCAPagar->setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelCAPagar->setTable("ContaAPagar");

  if (not modelCAPagar->select()) {
    qDebug() << "Failed to populate TableContasPagar: " << modelCAPagar->lastError();
    return;
  }

  ui->tableContasPagar->setModel(modelCAPagar);
  ui->tableContasPagar->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableContasPagar->setItemDelegate(doubledelegate);

  // Contas a receber -------------------------------------
  modelCAReceber = new QSqlTableModel(this);
  modelCAReceber->setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelCAReceber->setTable("ContaAReceber");

  if (not modelCAReceber->select()) {
    qDebug() << "Failed to populate TableContasReceber: " << modelCAReceber->lastError();
    return;
  }

  ui->tableContasReceber->setModel(modelCAReceber);
  ui->tableContasReceber->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableContasReceber->setItemDelegate(doubledelegate);

  // Entregas cliente
  modelEntregasCliente = new QSqlTableModel(this);
  modelEntregasCliente->setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelEntregasCliente->setTable("PedidoTransportadora");

  if (not modelEntregasCliente->select()) {
    qDebug() << "Failed to populate TableEntregasCliente: " << modelEntregasCliente->lastError();
    return;
  }

  modelEntregasCliente->setFilter("tipo = 'CLIENTE'");

  ui->tableEntregasCliente->setModel(modelEntregasCliente);
  ui->tableEntregasCliente->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableEntregasCliente->setItemDelegate(doubledelegate);

  // Recebimentos fornecedor
  modelRecebimentosForn = new QSqlTableModel(this);
  modelRecebimentosForn->setTable("PedidoTransportadora");

  if (not modelRecebimentosForn->select()) {
    qDebug() << "Failed to populate TableRecebimentosFornecedor: " << modelRecebimentosForn->lastError();
    return;
  }

  modelRecebimentosForn->setFilter("tipo = 'FORNECEDOR'");

  ui->tableRecebimentosFornecedor->setModel(modelRecebimentosForn);
  ui->tableRecebimentosFornecedor->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableRecebimentosFornecedor->setItemDelegate(doubledelegate);

  // Pedidos de compra
  modelPedCompra = new QSqlRelationalTableModel(this);
  modelPedCompra->setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelPedCompra->setTable("PedidoFornecedor");
  modelPedCompra->setRelation(modelPedCompra->fieldIndex("idLoja"), QSqlRelation("Loja", "idLoja", "descricao"));
  modelPedCompra->setRelation(modelPedCompra->fieldIndex("idUsuario"), QSqlRelation("Usuario", "idUsuario", "nome"));
  modelPedCompra->setRelation(modelPedCompra->fieldIndex("idCliente"),
                              QSqlRelation("Cliente", "idCliente", "nome_razao"));
  modelPedCompra->setRelation(modelPedCompra->fieldIndex("idEnderecoEntrega"),
                              QSqlRelation("Cliente_has_Endereco", "idEndereco", "logradouro"));
  modelPedCompra->setRelation(modelPedCompra->fieldIndex("idProfissional"),
                              QSqlRelation("Profissional", "idProfissional", "nome_razao"));
  modelPedCompra->setHeaderData(modelPedCompra->fieldIndex("idLoja"), Qt::Horizontal, "Loja");
  modelPedCompra->setHeaderData(modelPedCompra->fieldIndex("idUsuario"), Qt::Horizontal, "Vendedor");
  modelPedCompra->setHeaderData(modelPedCompra->fieldIndex("idCliente"), Qt::Horizontal, "Cliente");
  modelPedCompra->setHeaderData(modelPedCompra->fieldIndex("idEnderecoEntrega"), Qt::Horizontal, "Endereço");
  modelPedCompra->setHeaderData(modelPedCompra->fieldIndex("idProfissional"), Qt::Horizontal, "Profissional");

  if (not modelPedCompra->select()) {
    qDebug() << "Failed to populate TablePedidosCompra:" << modelPedCompra->lastError();
    return;
  }

  ui->tablePedidosCompra->setModel(modelPedCompra);
  ui->tablePedidosCompra->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tablePedidosCompra->setItemDelegate(doubledelegate);

  // NFe
  modelNFe = new QSqlTableModel(this);
  modelNFe->setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelNFe->setTable("NFe");

  if (not modelNFe->select()) {
    qDebug() << "Failed to populate TableNFe: " << modelNFe->lastError();
    return;
  }

  ui->tableNFE->setModel(modelNFe);
  ui->tableNFE->setColumnHidden(modelNFe->fieldIndex("NFe"), true);
  ui->tableNFE->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableNFE->setItemDelegate(doubledelegate);
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

  if (not modelPedCompra->select()) {
    qDebug() << "erro modelPedCompra: " << modelPedCompra->lastError();
    return;
  }

  if (not modelEntregasCliente->select()) {
    qDebug() << "erro modelEntregasCliente: " << modelEntregasCliente->lastError();
    return;
  }

  if (not modelRecebimentosForn->select()) {
    qDebug() << "erro modelRecebimentosForn: " << modelRecebimentosForn->lastError();
    return;
  }

  if (not modelNFe->select()) {
    qDebug() << "erro modelNFe: " << modelNFe->lastError();
    return;
  }

  ui->tableContasPagar->resizeColumnsToContents();
  ui->tableContasReceber->resizeColumnsToContents();
  ui->tableRecebimentosFornecedor->resizeColumnsToContents();
  ui->tableEntregasCliente->resizeColumnsToContents();
  ui->tableOrcamentos->resizeColumnsToContents();
  ui->tablePedidosCompra->resizeColumnsToContents();
  ui->tableVendas->resizeColumnsToContents();
  ui->tableNFE->resizeColumnsToContents();
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
  modelNFe->setFilter("status = 'autorizado'");
  ui->tableNFE->resizeColumnsToContents();
}

void MainWindow::on_radioButtonNFeEnviado_clicked() {
  modelNFe->setFilter("status = 'enviado'");
  ui->tableNFE->resizeColumnsToContents();
}

void MainWindow::on_radioButtonNFeLimpar_clicked() {
  modelNFe->setFilter("");
  ui->tableNFE->resizeColumnsToContents();
}

void MainWindow::on_radioButtonFornLimpar_clicked() {
  modelPedCompra->setFilter("");
  ui->tablePedidosCompra->resizeColumnsToContents();
}

void MainWindow::on_radioButtonFornAberto_clicked() {
  modelPedCompra->setFilter("status = 'aberto'");
  ui->tablePedidosCompra->resizeColumnsToContents();
}

void MainWindow::on_radioButtonFornFechado_clicked() {
  modelPedCompra->setFilter("status = 'fechado'");
  ui->tablePedidosCompra->resizeColumnsToContents();
}

void MainWindow::on_radioButtonRecebimentoLimpar_clicked() {
  modelRecebimentosForn->setFilter("tipo = 'fornecedor'");
  ui->tableRecebimentosFornecedor->resizeColumnsToContents();
}

void MainWindow::on_radioButtonRecebimentoRecebido_clicked() {
  modelRecebimentosForn->setFilter("status = 'recebido' AND tipo = 'fornecedor'");
  ui->tableRecebimentosFornecedor->resizeColumnsToContents();
}

void MainWindow::on_radioButtonRecebimentoPendente_clicked() {
  modelRecebimentosForn->setFilter("status = 'pendente' AND tipo = 'fornecedor'");
  ui->tableRecebimentosFornecedor->resizeColumnsToContents();
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
  modelOrcamento->setFilter("idUsuario = " + QString::number(UserSession::getId()));
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
    modelVendas->setFilter("(idVenda LIKE '%" + text + "%') OR (Cliente LIKE '%" + text + "%')");
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

void MainWindow::on_lineEditBuscaProdutosPend_textChanged(const QString &text) {
  if (text.isEmpty()) {
    modelPedCompra->setFilter("");
  } else {
    modelPedCompra->setFilter("(Cliente LIKE '%" + text + "%') OR (status LIKE '%" + text + "%')");
  }
}

void MainWindow::on_lineEditBuscaRecebimentos_textChanged(const QString &text) {
  if (text.isEmpty()) {
    modelRecebimentosForn->setFilter("");
  } else {
    modelRecebimentosForn->setFilter("(idPedido LIKE '%" + text + "%') OR (status LIKE '%" + text + "%')");
  }
}

void MainWindow::on_lineEditBuscaNFe_textChanged(const QString &text) {
  if (text.isEmpty()) {
    modelNFe->setFilter("");
  } else {
    modelNFe->setFilter("(idVenda LIKE '%" + text + "%') OR (status LIKE '%" + text + "%')");
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
  settings.endGroup();
}

void MainWindow::showMaximized() {
  QMainWindow::showMaximized();

  // TODO: substituir por findChildren
  setupTable(ui->tableOrcamentos);
  setupTable(ui->tableVendas);
  setupTable(ui->tableContasPagar);
  setupTable(ui->tableContasReceber);
  setupTable(ui->tableEntregasCliente);
  setupTable(ui->tableNFE);
  setupTable(ui->tablePedidosCompra);
  setupTable(ui->tableRecebimentosFornecedor);
}

void MainWindow::on_actionImportaTeste_triggered() {
  ImportaProdutos *importa = new ImportaProdutos(this);
  importa->importar();
}

void MainWindow::on_tableVendas_activated(const QModelIndex &index) {
  Venda *vendas = new Venda(this);
  vendas->viewRegisterById(modelVendas->data(modelVendas->index(index.row(), modelVendas->fieldIndex("idVenda"))));
}

void MainWindow::on_tableOrcamentos_activated(const QModelIndex &index) {
  Orcamento *orcamento = new Orcamento(this);
  orcamento->viewRegisterById(
        modelOrcamento->data(modelOrcamento->index(index.row(), modelOrcamento->fieldIndex("Código"))));
  orcamento->show();
}

void MainWindow::on_tableContasPagar_activated(const QModelIndex &index) {
  ContasAPagar *contas = new ContasAPagar(this);
  contas->viewConta(
        modelCAPagar->data(modelCAPagar->index(index.row(), modelCAPagar->fieldIndex("idVenda"))).toString());
}

void MainWindow::on_tableContasReceber_activated(const QModelIndex &index) {
  ContasAReceber *contas = new ContasAReceber(this);
  contas->viewConta(
        modelCAReceber->data(modelCAReceber->index(index.row(), modelCAReceber->fieldIndex("idVenda"))).toString());
}

void MainWindow::on_tableEntregasCliente_activated(const QModelIndex &index) {
  EntregasCliente *entregas = new EntregasCliente(this);
  entregas->viewEntrega(
        modelEntregasCliente->data(modelEntregasCliente->index(index.row(), modelEntregasCliente->fieldIndex("idPedido")))
        .toString());
}

void MainWindow::on_tablePedidosCompra_activated(const QModelIndex &index) {
  PedidosCompra *pedidos = new PedidosCompra(this);
  pedidos->viewPedido(modelPedCompra->data(modelPedCompra->index(index.row(), 0)).toString());
}

void MainWindow::on_tableRecebimentosFornecedor_activated(const QModelIndex &index) {
  RecebimentosFornecedor *recebimentos = new RecebimentosFornecedor(this);
  recebimentos->viewRecebimento(
        modelRecebimentosForn->data(modelRecebimentosForn->index(
                                      index.row(), modelRecebimentosForn->fieldIndex("idPedido"))).toString());
}

void MainWindow::on_tableNFE_activated(const QModelIndex &index) {
  Venda *vendas = new Venda(this);
  vendas->viewRegisterById(modelNFe->data(modelNFe->index(index.row(), modelNFe->fieldIndex("idVenda"))));
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
