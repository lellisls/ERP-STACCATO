#ifndef IMPRESSAO_H
#define IMPRESSAO_H

#include <QSqlQuery>

#include "lrreportengine.h"
#include "sqltablemodel.h"

class Impressao : public QObject {
  Q_OBJECT

public:
  explicit Impressao(const QString &id);
  Impressao(const Impressao &) = delete;
  void print();

private:
  // attributes
  enum Type { Orcamento, Venda } type;
  const QString id;
  QSqlQuery queryCliente;
  QSqlQuery queryEndEnt;
  QSqlQuery queryEndFat;
  QSqlQuery queryLoja;
  QSqlQuery queryLojaEnd;
  QSqlQuery query;
  QSqlQuery queryProfissional;
  QSqlQuery queryVendedor;
  SqlTableModel modelItem;
  LimeReport::ReportEngine *report;
  // methods
  bool setQuerys();
  void verificaTipo();
};

#endif // IMPRESSAO_H
