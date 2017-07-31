#ifndef WIDGETCOMPRAPENDENTES_H
#define WIDGETCOMPRAPENDENTES_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetCompraPendentes;
}

class WidgetCompraPendentes : public QWidget {
  Q_OBJECT

public:
  explicit WidgetCompraPendentes(QWidget *parent = 0);
  ~WidgetCompraPendentes();
  bool updateTables();

signals:
  void errorSignal(const QString &error);

private slots:
  void montaFiltro();
  void on_doubleSpinBoxQuantAvulso_valueChanged(const double value);
  void on_doubleSpinBoxQuantAvulsoCaixas_valueChanged(const double value);
  void on_groupBoxStatus_toggled(bool enabled);
  void on_pushButtonComprarAvulso_clicked();
  void on_pushButtonExcel_clicked();
  void on_pushButtonPDF_clicked();
  void on_table_activated(const QModelIndex &index);
  void on_table_entered(const QModelIndex &);
  void setarDadosAvulso();

private:
  // attributes
  SqlTableModel model;
  Ui::WidgetCompraPendentes *ui;
  // methods
  bool insere(const QDate &dataPrevista);
  void makeConnections();
  void setupTables();
};

#endif // WIDGETCOMPRAPENDENTES_H
