#ifndef WIDGETLOGISTICAENTREGA_H
#define WIDGETLOGISTICAENTREGA_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetLogisticaEntrega;
}

class WidgetLogisticaEntrega : public QWidget {
  Q_OBJECT

public:
  explicit WidgetLogisticaEntrega(QWidget *parent = 0);
  ~WidgetLogisticaEntrega();
  bool updateTables();

signals:
  void errorSignal(const QString &error);

private slots:
  void on_dateTimeEdit_dateChanged(const QDate &date);
  void on_itemBoxVeiculo_textChanged(const QString &);
  void on_pushButtonAdicionarParcial_clicked();
  void on_pushButtonAdicionarProduto_clicked();
  void on_pushButtonAgendarCarga_clicked();
  void on_pushButtonReagendarPedido_clicked();
  void on_pushButtonRemoverProduto_clicked();
  void on_tableProdutos_entered(const QModelIndex &);
  void on_tableTransp2_entered(const QModelIndex &);
  void on_tableVendas_clicked(const QModelIndex &index);
  void on_tableVendas_entered(const QModelIndex &);

  void on_tableVendas_doubleClicked(const QModelIndex &index);

private:
  // attributes
  QString error;
  //  SqlTableModel modelConsumo;
  SqlTableModel modelProdutos;
  SqlTableModel modelTransp;
  SqlTableModel modelTransp2;
  SqlTableModel modelVendas;
  SqlTableModel modelViewProdutos;
  Ui::WidgetLogisticaEntrega *ui;
  // methods
  bool adicionarProduto(const QModelIndexList &list);
  bool adicionarProdutoParcial(const int row);
  bool processRows();
  bool quebrarProduto(const int row, const int quantAgendar, const int quantTotal);
  void calcularDisponivel();
  void calcularPeso();
  void montaFiltro();
  void setupTables();
  bool reagendar(const QModelIndexList &list, const QDate &dataPrev, const QString &observacao);
};

#endif // WIDGETLOGISTICAENTREGA_H
