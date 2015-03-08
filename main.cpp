#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setOrganizationName("ERP");
  app.setApplicationName("Staccato");

  QTranslator qtTranslator;
  qtTranslator.load("qt_" + QLocale::system().name(),
                    QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  app.installTranslator(&qtTranslator);
  MainWindow window;
  window.showMaximized();
  return app.exec();
}
