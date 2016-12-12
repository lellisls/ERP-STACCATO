#ifndef WIDGETRELATORIO_H
#define WIDGETRELATORIO_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetRelatorio;
}

class WidgetRelatorio : public QWidget {
  Q_OBJECT

public:
  explicit WidgetRelatorio(QWidget *parent = 0);
  ~WidgetRelatorio();
  bool updateTables();

signals:
  void errorSignal(const QString &error);

private slots:
  void on_dateEditMes_dateChanged(const QDate &);
  void on_pushButtonExcel_clicked();
  void on_tableRelatorio_entered(const QModelIndex &);
  void on_tableTotalLoja_entered(const QModelIndex &);
  void on_tableTotalVendedor_entered(const QModelIndex &);

private:
  // attributes
  SqlTableModel modelOrcamento;
  SqlTableModel modelRelatorio;
  SqlTableModel modelTotalLoja;
  SqlTableModel modelTotalVendedor;
  Ui::WidgetRelatorio *ui;
  // methods
  bool setupTables();
  void setFilterTotaisVendedor();
  void setFilterTotaisLoja();
  void calcularTotalGeral();
  void setFilterRelatorio();
};

#endif // WIDGETRELATORIO_H
