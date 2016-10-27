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
  app.setApplicationVersion("0.4.10");
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
// NOTE: criar enum para tipos de usuario de forma que possa  'tipo >= GERENTE' (criar coluna no bd com numeros
// 0,1,2...)
// NOTE: repassar por todos pedidos/orcamentos imprimindo para verificar quais campos cortam (reduzir fonte?)
// NOTE: criar um delegate unidade para concatenar a unidade na coluna quant?
// NOTE: opcao de salvar excel/pdf por mes
// NOTE: colocar formato real e comercial do produto (Por exemplo Downtown Hg gr 877,0x877,0x2,0 (90x90 RET) loes)
// NOTE: escanear papel de venda
// TODO: ao gerar pedido de venda exportar relatorio para arquivo e salvar no owncloud
// TODO: produtos representacao castellato saindo com preco descontado (verificar se preco custo = preco venda)
// TODO: research viability of using pragma once
// TODO: divide views into categories like: view_compra_..., view_logistica_..., view_financeiro_..., etc
// TODO: make estoque_has_consumo idCompra column int (is varchar now)
// TODO: get processRows functions and change parameter to ref
// TODO: criar FK em venda_has_produto ligando idNFesaida com nfe
