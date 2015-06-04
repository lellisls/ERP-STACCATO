#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include "mainwindow.h"

int main(int argc, char *argv[]) {

#ifdef QT_DEBUG
  qSetMessagePattern("%{function}:%{file}:%{line} - %{message}");
#else
  qSetMessagePattern("%{message}");
#endif

  QApplication app(argc, argv);
  app.setOrganizationName("ERP");
  app.setApplicationName("Staccato");

  QTranslator qtTranslator;
  if (not qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
  }
  app.installTranslator(&qtTranslator);
  QTranslator pt;
  if (not pt.load(":/qt_portuguese.qm")) {
    qDebug() << "Error loading qt_portuguese.ts";
  }
  app.installTranslator(&pt);

  MainWindow window;
  window.showMaximized();
  return app.exec();
}
