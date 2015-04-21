#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
//  qSetMessagePattern("%{function}:%{file}:%{line} - %{message}");

  QApplication app(argc, argv);
  app.setOrganizationName("ERP");
  app.setApplicationName("Staccato");

  QTranslator qtTranslator;
  if(!qtTranslator.load("qt_" + QLocale::system().name(),
                    QLibraryInfo::location(QLibraryInfo::TranslationsPath)) ){
  }
  app.installTranslator(&qtTranslator);
  QTranslator pt;
  if(!pt.load(":/qt_portuguese.qm")){
    qDebug() << "Error loading qt_portuguese.ts";
  }
  app.installTranslator(&pt);

  MainWindow window;
  window.showMaximized();
  return app.exec();
}
