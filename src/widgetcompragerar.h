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
  void errorSignal(QString error);

private slots:
  void calcularPreco();
  void on_checkBoxMarcarTodos_clicked(const bool &checked);
  void on_pushButtonGerarCompra_clicked();
  void on_tableForn_activated(const QModelIndex &index);
  void on_tableProdutos_entered(const QModelIndex &);

private:
  // attributes
  Ui::WidgetCompraGerar *ui;
  SqlTableModel modelProdutos;
  SqlTableModel modelForn;
  int oc;
  // methods
  void setupTables();
  bool gerarExcel(QList<int> &lista, QString &anexo);
  bool gerarCompra();
};

#endif // WIDGETCOMPRAGERAR_H
