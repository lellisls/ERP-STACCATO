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
  void on_pushButtonRemoverProduto_clicked();
  void on_tableProdutos_entered(const QModelIndex &);
  void on_tableTransp2_entered(const QModelIndex &);
  void on_tableVendas_clicked(const QModelIndex &index);
  void on_tableVendas_entered(const QModelIndex &);

private:
  // attributes
  QString error;
  SqlTableModel modelProdutos;
  SqlTableModel modelViewProdutos;
  SqlTableModel modelTransp;
  SqlTableModel modelTransp2;
  SqlTableModel modelVendas;
  Ui::WidgetLogisticaEntrega *ui;
  // methods
  bool adicionarProduto(const QModelIndexList &list);
  bool processRows();
  void calcularPeso();
  void montaFiltro();
  void setupTables();
  void calcularDisponivel();
  bool adicionarProdutoParcial(const int row);
  bool quebrarProduto(const int row, const int quantAgendar, const int quantTotal);
};

#endif // WIDGETLOGISTICAENTREGA_H
