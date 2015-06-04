#include <QtTest/QtTest>
#include "testmainwindow.h"

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

  TestMainWindow testMainWindow;

  return QTest::qExec(&testMainWindow, argc, argv);
}
