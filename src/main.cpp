#include <QApplication>
#include <QDebug>

#include "logindialog.h"
#include "mainwindow.h"

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
  app.setApplicationVersion("0.5.11");
  app.setStyle("Fusion");

  LoginDialog dialog;

  if (dialog.exec() == QDialog::Rejected) exit(1);

  MainWindow window;
  window.showMaximized();

  return app.exec();
}

// NOTE: evitar divisoes por zero
// NOTE: verificar comparacoes double: substituir por qFuzzyCompare ou floats
// NOTE: verificar todos os QSqlQuery.exec
// NOTE: pesquisar setData e selects/submits sem verificacao
// NOTE: criar um delegate unidade para concatenar a unidade na coluna quant?
// NOTE: divide views into categories like: view_compra_..., view_logistica_..., view_financeiro_..., etc
// NOTE: use initializer lists?
