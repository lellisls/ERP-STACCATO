#ifndef EXCEL_H
#define EXCEL_H

#include <QSqlQuery>
#include <QWidget>

class Excel {

public:
  Excel(QString id, QWidget *parent = 0);
  bool gerarExcel(const int oc = 0, const bool isRepresentacao = false, const QString representacao = QString());
  QString getFileName() const;

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
  QString fileName;
  QString id;
  QWidget *parent;
  // methods
  bool setQuerys();
  void verificaTipo();
};

#endif // EXCEL_H
