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
  explicit Devolucao(const QString &idVenda, QWidget *parent = 0);
  ~Devolucao();

private slots:
  void on_doubleSpinBoxCaixas_valueChanged(const double caixas);
  void on_doubleSpinBoxQuant_editingFinished();
  void on_doubleSpinBoxQuant_valueChanged(double);
  void on_doubleSpinBoxTotalItem_valueChanged(double value);
  void on_groupBoxCredito_toggled(bool);
  void on_pushButtonDevolverItem_clicked();
  void on_tableProdutos_clicked(const QModelIndex &index);

private:
  // attributes
  bool createNewId = false;
  const QString idVenda;
  QDataWidgetMapper mapperItem;
  QString error;
  QString idDevolucao;
  SqlTableModel modelCliente;
  SqlTableModel modelDevolvidos;
  SqlTableModel modelPagamentos;
  SqlTableModel modelProdutos;
  SqlTableModel modelVenda;
  Ui::Devolucao *ui;
  // methods
  bool atualizarDevolucao();
  bool criarContas();
  bool criarDevolucao();
  bool desvincularCompra();
  bool devolverItem(const QModelIndexList &list);
  bool inserirItens(const QModelIndexList &list);
  bool salvarCredito();
  void calcPrecoItemTotal();
  void determinarIdDevolucao();
  void setupTables();
  void limparCampos();
};

#endif // DEVOLUCAO_H
