#ifndef DEVOLUCAO_H
#define DEVOLUCAO_H

#include <QDataWidgetMapper>
#include <QDialog>

#include "sqltablemodel.h"

namespace Ui {
class Devolucao;
}

class Devolucao : public QDialog {
  Q_OBJECT

public:
  explicit Devolucao(QString idVenda, QWidget *parent = 0);
  ~Devolucao();

private slots:
  void on_doubleSpinBoxQuant_editingFinished();
  void on_doubleSpinBoxQuant_valueChanged(double);
  void on_spinBoxCaixas_valueChanged(const int &caixas);
  void on_tableProdutos_clicked(const QModelIndex &index);
  void on_pushButtonDevolverItem_clicked();
  void on_doubleSpinBoxTotalItem_valueChanged(double value);

  void on_groupBoxCredito_toggled(bool);

private:
  // attributes
  Ui::Devolucao *ui;
  SqlTableModel modelProdutos;
  SqlTableModel modelDevolvidos;
  SqlTableModel modelPagamentos;
  SqlTableModel modelVenda;
  SqlTableModel modelCliente;
  QDataWidgetMapper mapperItem;
  QString idVenda;
  // methods
  void setupTables(QString idVenda);
  void calcPrecoItemTotal();
  void criarDevolucao(QModelIndexList list);
  void inserirItens(QModelIndexList list);
  void criarContas();
  void atualizarDevolucao();
  void salvarCredito();
};

#endif // DEVOLUCAO_H
