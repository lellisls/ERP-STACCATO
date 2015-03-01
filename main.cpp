#include <QApplication>
#include <QTranslator>

#include "mainwindow.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setOrganizationName("ERP");
  app.setApplicationName("Staccato");

  QTranslator qtTranslator;
  qtTranslator.load("qt_" + QLocale::system().name());
  app.installTranslator(&qtTranslator);
  MainWindow window;
  window.showMaximized();
  return app.exec();
}
