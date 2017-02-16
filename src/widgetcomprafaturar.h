#ifndef WIDGETCOMPRAFATURAR_H
#define WIDGETCOMPRAFATURAR_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetCompraFaturar;
}

class WidgetCompraFaturar : public QWidget {
  Q_OBJECT

public:
  explicit WidgetCompraFaturar(QWidget *parent = 0);
  ~WidgetCompraFaturar();
  bool updateTables();

signals:
  void errorSignal(const QString &error);

private slots:
  void on_checkBoxRepresentacao_toggled(bool checked);
  void on_pushButtonCancelarCompra_clicked();
  void on_pushButtonMarcarFaturado_clicked();
  void on_pushButtonReagendar_clicked();
  void on_table_entered(const QModelIndex &);

private:
  // attributes
  QString error;
  SqlTableModel model;
  Ui::WidgetCompraFaturar *ui;
  // methods
  bool cancelar(const QModelIndexList &list);
  bool faturarCompra(const QDateTime &dataReal, const QStringList &idsCompra);
  void setupTables();
};

#endif // WIDGETCOMPRAFATURAR_H
