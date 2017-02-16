#ifndef WIDGETCOMPRAOC_H
#define WIDGETCOMPRAOC_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetCompraOC;
}

class WidgetCompraOC : public QWidget {
  Q_OBJECT

public:
  explicit WidgetCompraOC(QWidget *parent = 0);
  ~WidgetCompraOC();
  bool updateTables();

signals:
  void errorSignal(const QString &error);

private slots:
  void on_lineEditBusca_textChanged(const QString &text);
  void on_pushButtonDanfe_clicked();
  void on_pushButtonDesfazerConsumo_clicked();
  void on_tableNFe_entered(const QModelIndex &);
  void on_tablePedido_clicked(const QModelIndex &index);
  void on_tablePedido_entered(const QModelIndex &);
  void on_tableProduto_entered(const QModelIndex &);

private:
  // attributes
  SqlTableModel modelNFe;
  SqlTableModel modelPedido;
  SqlTableModel modelProduto;
  QString error;
  Ui::WidgetCompraOC *ui;
  // methods
  void setupTables();
  bool desfazerConsumo(const QModelIndexList &list);
};

#endif // WIDGETCOMPRAOC_H
