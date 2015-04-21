#include <QtTest/QtTest>
#include "testsuite1.h"
#include "testsuite2.h"

int main(int argc, char** argv){
  QApplication app(argc, argv);
  QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

  TestSuite1 testSuite1;
  TestSuite2 testSuite2;

  return QTest::qExec(&testSuite1, argc, argv) | QTest::qExec(&testSuite2, argc, argv);
}
