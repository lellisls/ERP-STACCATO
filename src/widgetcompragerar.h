#ifndef WIDGETCOMPRAGERAR_H
#define WIDGETCOMPRAGERAR_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetCompraGerar;
}

class WidgetCompraGerar : public QWidget {
  Q_OBJECT

public:
  explicit WidgetCompraGerar(QWidget *parent = 0);
  ~WidgetCompraGerar();
  bool updateTables();

signals:
  void errorSignal(const QString &error);

private slots:
  void calcularPreco();
  void on_checkBoxMarcarTodos_clicked(const bool checked);
  void on_pushButtonCancelarCompra_clicked();
  void on_pushButtonGerarCompra_clicked();
  void on_tableForn_activated(const QModelIndex &index);
  void on_tableProdutos_entered(const QModelIndex &);

private:
  // attributes
  int oc;
  QString error;
  SqlTableModel modelForn;
  SqlTableModel modelProdutos;
  Ui::WidgetCompraGerar *ui;
  // methods
  bool cancelar(const QModelIndexList &list);
  bool gerarCompra(const QList<int> &lista, const QDateTime &dataCompra, const QDateTime &dataPrevista);
  bool gerarExcel(const QList<int> &lista, QString &anexo, const bool isRepresentacao);
  void setupTables();
};

#endif // WIDGETCOMPRAGERAR_H
