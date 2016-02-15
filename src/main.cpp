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
  app.setApplicationVersion("0.2.22");
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

// NOTE: adaptar a funcao de importar tabela para perguntar se é normal, estoque ou promocao e setar a flag de acordo
// (produtos de estoque sao sempre 1 por caixa?)
// NOTE: evitar divisoes por zero
// NOTE: para a tabela de estoque: inserir coluna estoque_promocao para pintar, inserir estoque na tabela produto mas
// colocar coluna idRelacionado para indicar qual o produto (para usar o idProduto original para vender/etc)
// NOTE: pegar as querys com 'select *' e reduzir para apenas as colunas necessarias
// NOTE: verificar comparacoes double: substituir por qFuzzyCompare ou floats
// NOTE: verificar todos os QSqlQuery.exec
// NOTE: pesquisar setData e selects/submits sem verificacao
// NOTE: verificar se tem como buscar id's nao relacionados e apagar produtos descontinuados
// NOTE: criar enum para tipos de usuario de forma que possa fazer 'tipo >= GERENTE' (criar coluna no bd com numeros
// 0,1,2...)
// NOTE: obrigar atualizar para conectar no servidor?
// TODO: repassar por todos pedidos/orcamentos imprimindo para verificar quais campos cortam (reduzir fonte?)
// NOTE: fazer um delegate para 'R$'?
// NOTE: criar um delegate unidade para concatenar a unidade na coluna quant?
// TODO: aparentemente nao precisa fazer date.tostring(yyyy/dd/mm) para converter para o formato do bd
// TODO: diferenciar portinari loes para questao de comissao no excel
// TODO: colocar autoredimensionar no scroll dos tables
// NOTE: opcao de salvar excel/pdf por mes
