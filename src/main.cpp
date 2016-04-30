#include <QApplication>
#include <QInputDialog>

#include "QSimpleUpdater"
#include "logindialog.h"
#include "mainwindow.h"
#include "usersession.h"

void update();

void storeSelection();

int main(int argc, char *argv[]) {

#ifdef QT_DEBUG
  qSetMessagePattern("%{function}:%{file}:%{line} - %{message}");
#else
  qSetMessagePattern("%{message}");
#endif

  QApplication app(argc, argv);
  app.setOrganizationName("Staccato");
  app.setApplicationName("ERP");
  app.setWindowIcon(QIcon("Staccato.ico"));
  app.setApplicationVersion("0.2.38");
  app.setStyle("Fusion");

  storeSelection();

  update();

  LoginDialog *dialog = new LoginDialog();

  if (dialog->exec() == QDialog::Rejected) exit(1);

  MainWindow window;
  window.showMaximized();

  return app.exec();
}

void update() {
  QSimpleUpdater *updater = new QSimpleUpdater();
  updater->setApplicationVersion(qApp->applicationVersion());
  updater->setReferenceUrl("http://" + UserSession::settings("Login/hostname").toString() + "/versao.txt");
  updater->setDownloadUrl("http://" + UserSession::settings("Login/hostname").toString() + "/Instalador.exe");
  updater->setSilent(true);
  updater->setShowNewestVersionMessage(true);
  updater->checkForUpdates();
}

void storeSelection() {
  if (UserSession::settings("Login/hostname").toString().isEmpty()) {
    QStringList items;
    items << "Alphaville"
          << "Gabriel"
          << "Granja";

    QString loja = QInputDialog::getItem(0, "Escolha a loja", "Qual a sua loja?", items, 0, false);

    if (loja == "Alphaville") UserSession::setSettings("Login/hostname", "192.168.2.144");
    if (loja == "Gabriel") UserSession::setSettings("Login/hostname", "192.168.1.101");
    if (loja == "Granja") UserSession::setSettings("Login/hostname", "192.168.0.10");
  }
}

// NOTE: evitar divisoes por zero
// NOTE: verificar comparacoes double: substituir por qFuzzyCompare ou floats
// NOTE: verificar todos os QSqlQuery.exec
// NOTE: pesquisar setData e selects/submits sem verificacao
// NOTE: verificar se tem como buscar id's nao relacionados e apagar produtos descontinuados
// NOTE: criar enum para tipos de usuario de forma que possa  'tipo >= GERENTE' (criar coluna no bd com numeros
// 0,1,2...)
// NOTE: repassar por todos pedidos/orcamentos imprimindo para verificar quais campos cortam (reduzir fonte?)
// NOTE: criar um delegate unidade para concatenar a unidade na coluna quant?
// NOTE: opcao de salvar excel/pdf por mes
// NOTE: colocar formato real e comercial do produto (Por exemplo Downtown Hg gr 877,0x877,0x2,0 (90x90 RET) loes)
// NOTE: escanear papel de venda
// TODO: arrumar no sql na tabela de pedido_fornecedor_has_produto a coluna preco (calcular quant * prcUnitario, verificar calculo na tela de compra)
