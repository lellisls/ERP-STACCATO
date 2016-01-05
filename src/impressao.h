#ifndef IMPRESSAO_H
#define IMPRESSAO_H

#include <QSqlQuery>

#include "sqltablemodel.h"

class Impressao : public QObject {
    Q_OBJECT

  public:
    Impressao(QString id, QWidget *parent = 0);
    void print();

  private slots:
    void setValue(const int &recNo, const QString &paramName, QVariant &paramValue, const int &reportPage);

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
    QSqlQuery queryProduto;
    QSqlQuery queryProfissional;
    QSqlQuery queryVendedor;
    QWidget *parent;
    SqlTableModel model;
    SqlTableModel modelItem;
    // methods
    void setQuerys();
    void verificaTipo();
};

#endif // IMPRESSAO_H
