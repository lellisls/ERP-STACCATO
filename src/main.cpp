#include <QApplication>
#include <QInputDialog>

#include "QSimpleUpdater"
#include "logindialog.h"
#include "mainwindow.h"
#include "usersession.h"

QVariant settings(const QString &key) { return UserSession::settings(key); }

void setSettings(const QString &key, const QVariant &value) { UserSession::setSettings(key, value); }

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
  app.setApplicationVersion("0.2.8");
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
  updater->setReferenceUrl("http://" + settings("Login/hostname").toString() + "/versao.txt");
  updater->setDownloadUrl("http://" + settings("Login/hostname").toString() + "/Instalador.exe");
  updater->setSilent(true);
  updater->setShowNewestVersionMessage(true);
  updater->checkForUpdates();
}

void storeSelection() {
  if (settings("Login/hostname").toString().isEmpty()) {
    QStringList items;
    items << "Alphaville"
          << "Gabriel"
          << "Granja";

    QString loja = QInputDialog::getItem(0, "Escolha a loja", "Qual a sua loja?", items, 0, false);

    if (loja == "Alphaville") setSettings("Login/hostname", "192.168.2.144");
    if (loja == "Gabriel") setSettings("Login/hostname", "192.168.1.101");
    if (loja == "Granja") setSettings("Login/hostname", "192.168.0.10");
  }
}

// NOTE: verificar todos os QSqlQuery.exec
