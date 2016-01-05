#ifndef EXCEL_H
#define EXCEL_H

#include <QSqlQuery>

#include "sqltablemodel.h"

class Excel : public QObject {
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
    QSqlQuery queryOrc;
    QSqlQuery queryProduto;
    QSqlQuery queryProfissional;
    QSqlQuery queryVendedor;
    QString id;
    QWidget *parent;
    SqlTableModel model;
    SqlTableModel modelItem;
    // methods
    void setQuerys();
    void verificaTipo();
};

#endif // EXCEL_H
