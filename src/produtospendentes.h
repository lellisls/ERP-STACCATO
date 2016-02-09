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

  private:
    // attributes
    Ui::ProdutosPendentes *ui;
    QString codComercial;
    SqlTableModel modelEstoque;
    SqlTableModel modelProdutos;
    // methods
    void atualizaVenda(const QDate &dataPrevista);
    void insere(const QDate &dataPrevista);
    void setupTables();
};

#endif // PRODUTOSPENDENTES_H
