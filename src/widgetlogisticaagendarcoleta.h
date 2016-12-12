#ifndef WIDGETLOGISTICAAGENDARCOLETA_H
#define WIDGETLOGISTICAAGENDARCOLETA_H

#include <QStandardItemModel>
#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetLogisticaAgendarColeta;
}

class WidgetLogisticaAgendarColeta : public QWidget {
  Q_OBJECT

public:
  explicit WidgetLogisticaAgendarColeta(QWidget *parent = 0);
  ~WidgetLogisticaAgendarColeta();
  bool updateTables();
  void tableFornLogistica_activated(const QString &fornecedor);

signals:
  void errorSignal(const QString &error);

private slots:
  void calcularPeso();
  void on_itemBoxVeiculo_textChanged(const QString &);
  void on_lineEditBusca_textChanged(const QString &text);
  void on_pushButtonAdicionarProduto_clicked();
  void on_pushButtonAgendarColeta_clicked();
  void on_pushButtonCancelarCarga_clicked();
  void on_pushButtonDanfe_clicked();
  void on_pushButtonMontarCarga_clicked();
  void on_pushButtonRemoverProduto_clicked();
  void on_tableEstoque_doubleClicked(const QModelIndex &index);
  void on_tableEstoque_entered(const QModelIndex &);

private:
  // attributes
  SqlTableModel modelEstoque;
  SqlTableModel modelTransp;
  SqlTableModel modelTransp2;
  QString fornecedor;
  Ui::WidgetLogisticaAgendarColeta *ui;
  // methods
  bool adicionarProduto(const QModelIndexList &list);
  bool imprimirDanfe();
  bool processRows(const QModelIndexList &list, const bool montarCarga = false);
  void setupTables();
};

#endif // WIDGETLOGISTICAAGENDARCOLETA_H
