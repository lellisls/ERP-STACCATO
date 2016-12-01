#ifndef PRODUTOSPENDENTES_H
#define PRODUTOSPENDENTES_H

#include <QDialog>

#include "sqltablemodel.h"

namespace Ui {
class ProdutosPendentes;
}

class ProdutosPendentes : public QDialog {
  Q_OBJECT

public:
  explicit ProdutosPendentes(QWidget *parent = 0);
  ~ProdutosPendentes();
  void viewProduto(const QString &codComercial, const QString &idVenda);

private slots:
  void on_pushButtonComprar_clicked();
  void on_pushButtonConsumirCompra_clicked();
  void on_pushButtonConsumirEstoque_clicked();
  void on_tableEstoque_entered(const QModelIndex &);
  void on_tableProdutos_entered(const QModelIndex &);

private:
  // attributes
  QString codComercial;
  SqlTableModel modelEstoque;
  SqlTableModel modelProdutos;
  SqlTableModel modelCompra;
  Ui::ProdutosPendentes *ui;
  // methods
  bool consumirEstoque();
  bool insere(const QDateTime &dataPrevista);
  void setupTables();
  bool comprar();
  bool consumirCompra();
};

#endif // PRODUTOSPENDENTES_H
