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
  void on_pushButtonConsumirEstoque_clicked();
  void on_tableProdutos_entered(const QModelIndex &);

private:
  // attributes
  QString codComercial;
  QString error;
  SqlTableModel modelProdutos;
  SqlTableModel modelViewProdutos;
  SqlTableModel modelEstoque;
  Ui::ProdutosPendentes *ui;
  // methods
  bool insere(const QDateTime &dataPrevista);
  void setupTables();
  bool comprar(const QModelIndexList &list, const QDateTime &dataPrevista);
  bool consumirEstoque(const int rowProduto, const int rowEstoque);
  bool enviarExcedenteParaCompra(const int row, const QDateTime &dataPrevista);
  bool enviarProdutoParaCompra(const int row, const QDateTime &dataPrevista);
  bool atualizarVendaCompra(const int row, const QDateTime &dataPrevista);
  bool quebrarVendaConsumo(const double quantConsumir, const double quantTotalVenda, const int rowProduto);
  bool quebrarVenda(const int row, const QDateTime &dataPrevista);
  void recarregarTabelas();
};

#endif // PRODUTOSPENDENTES_H
