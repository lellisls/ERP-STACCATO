#ifndef EXCEL_H
#define EXCEL_H

#include <QSqlQuery>
#include <QWidget>

class Excel {

public:
  Excel(const QString &id, QWidget *parent = 0);
  bool gerarExcel(const int oc = 0, const bool isRepresentacao = false, const QString &representacao = QString());
  QString getFileName() const;

private:
  // attributes
  enum class Tipo { Orcamento, Venda } tipo;
  const QString id;
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
  QWidget *parent;
  // methods
  bool setQuerys();
  void verificaTipo();
};

#endif // EXCEL_H
