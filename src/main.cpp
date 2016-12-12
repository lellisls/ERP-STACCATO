#include <QApplication>

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
  app.setApplicationVersion("0.4.39");
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
// NOTE: colocar formato real e comercial do produto (Por exemplo Downtown Hg gr 877,0x877,0x2,0 (90x90 RET) loes)
// NOTE: divide views into categories like: view_compra_..., view_logistica_..., view_financeiro_..., etc
// NOTE: update_venda_status verificar se 'devolvido' e 'cancelado' devem ficar do jeito que estao (se nao estao saindo
// do estado cancelado etc)
// NOTE: search for places where I copy columns and skip 'created' and 'lastUpdated'
