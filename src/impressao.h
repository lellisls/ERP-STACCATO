#ifndef IMPRESSAO_H
#define IMPRESSAO_H

#include <QSqlQuery>

#include "lrreportengine.h"
#include "sqltablemodel.h"

class Impressao {

public:
  Impressao(QString id, QWidget *parent = 0);
  Impressao(const Impressao &) = delete;
  void print();

private:
  // attributes
  enum Type { Orcamento, Venda } type;
  QString id;
  QSqlQuery queryCliente;
  QSqlQuery queryEndEnt;
  QSqlQuery queryEndFat;
  QSqlQuery queryLoja;
  QSqlQuery queryLojaEnd;
  QSqlQuery query;
  QSqlQuery queryProfissional;
  QSqlQuery queryVendedor;
  QWidget *parent;
  SqlTableModel modelItem;
  LimeReport::ReportEngine *report;
  // methods
  bool setQuerys();
  void verificaTipo();
};

#endif // IMPRESSAO_H
