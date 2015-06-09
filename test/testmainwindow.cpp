#include "testmainwindow.h"
#include "src/mainwindow.h"

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

void TestMainWindow::testImportacao(){
  window->TestImportacao();

  QSqlQuery *query = new QSqlQuery("SELECT COUNT(*) FROM Produto");
  query->first();

  // Testa a tabela da Bellinzoni
  QCOMPARE(query->value(0).toInt(), 22);
}

// test cadastrar orcamento
// test cadastrar venda
// test gerar nfe
