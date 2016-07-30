#ifndef EXCEL_H
#define EXCEL_H

#include <QSqlQuery>
#include <QWidget>

class Excel {

public:
  Excel(QString id, QWidget *parent = 0);
  void gerarExcel();

private:
  // attributes
  enum Type { Orcamento, Venda } type;
  QSqlQuery query;
  QSqlQuery queryCliente;
  QSqlQuery queryEndEnt;
  QSqlQuery queryEndFat;
  QSqlQuery queryLoja;
  QSqlQuery queryLojaEnd;
  QSqlQuery queryProduto;
  QSqlQuery queryProfissional;
  QSqlQuery queryVendedor;
  QString id;
  QWidget *parent;
  // methods
  bool setQuerys();
  void verificaTipo();
};

#endif // EXCEL_H
