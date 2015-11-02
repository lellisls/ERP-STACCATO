#ifndef PRODUTOSPENDENTES_H
#define PRODUTOSPENDENTES_H

#include <QDialog>

#include "sqlquerymodel.h"

namespace Ui {
  class ProdutosPendentes;
}

class ProdutosPendentes : public QDialog {
    Q_OBJECT

  public:
    explicit ProdutosPendentes(QWidget *parent = 0);
    ~ProdutosPendentes();
    void viewProduto(const QString codComercial, const QString status);

  private slots:
    void on_pushButtonComprar_clicked();
    void on_pushButtonConsumirEstoque_clicked();

  private:
    Ui::ProdutosPendentes *ui;
    SqlQueryModel modelProdutos;
    QString codComercial;
    // methods
    void setupTables();
};

#endif // PRODUTOSPENDENTES_H
