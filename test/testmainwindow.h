#ifndef TESTMAINWINDOW_H
#define TESTMAINWINDOW_H

#include <QtTest>
#include <src/mainwindow.h>

class TestMainWindow : public QObject {
    Q_OBJECT

  private slots:
    void testInitDB();
    void testCadastroClienteIncompleto();
    void testCadastroClienteEndereco();
    void testCadastroClienteCompleto();  
    void testImportacao();

  private:
    MainWindow *window;
};

#endif // TESTMAINWINDOW_H
