#include "testmainwindow.h"
#include "mainwindow.h"

void TestMainWindow::testInitDB() {
  window = new MainWindow();
  QVERIFY2(window->TestInitDB(), "Inicializar BD falhou");
}

void TestMainWindow::testCadastroClienteIncompleto() {
  QVERIFY2(window->TestCadastroClienteIncompleto(), "Cadastro cliente falhou");
}

void TestMainWindow::testCadastroClienteEndereco() {
  QVERIFY2(window->TestCadastroClienteEndereco(), "Cadastro cliente falhou");
}

void TestMainWindow::testCadastroClienteCompleto() {
  QVERIFY2(window->TestCadastroClienteCompleto(), "Cadastro cliente falhou");
}

// test importacao
// test cadastrar orcamento
// test cadastrar venda
// test gerar nfe
