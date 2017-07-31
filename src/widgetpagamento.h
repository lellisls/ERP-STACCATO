#ifndef WIDGETPAGAMENTO_H
#define WIDGETPAGAMENTO_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetPagamento;
}

class WidgetPagamento : public QWidget {
  Q_OBJECT

public:
  enum class Tipo { Nulo, Receber, Pagar };
  explicit WidgetPagamento(QWidget *parent = 0);
  ~WidgetPagamento();
  bool updateTables();
  void setTipo(const Tipo &value);

signals:
  void errorSignal(const QString &error);

private slots:
  void on_dateEditDe_dateChanged(const QDate &date);
  void on_doubleSpinBoxDe_valueChanged(const double value);
  void on_groupBoxData_toggled(const bool enabled);
  void on_pushButtonAdiantarRecebimento_clicked();
  void on_pushButtonExcluirLancamento_clicked();
  void on_pushButtonInserirLancamento_clicked();
  void on_pushButtonInserirTransferencia_clicked();
  void on_table_activated(const QModelIndex &index);
  void on_table_entered(const QModelIndex &);
  void on_tableVencer_doubleClicked(const QModelIndex &index);
  void on_tableVencer_entered(const QModelIndex &);
  void on_tableVencidos_doubleClicked(const QModelIndex &index);
  void on_tableVencidos_entered(const QModelIndex &);

private:
  // attributes
  SqlTableModel model;
  QSqlQueryModel modelVencidos;
  QSqlQueryModel modelVencer;
  Tipo tipo = Tipo::Nulo;
  Ui::WidgetPagamento *ui;
  // methods
  void makeConnections();
  void montaFiltro();
  void setupTables();
};

#endif // WIDGETPAGAMENTO_H
