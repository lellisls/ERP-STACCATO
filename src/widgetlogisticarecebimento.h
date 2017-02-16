#ifndef WIDGETLOGISTICARECEBIMENTO_H
#define WIDGETLOGISTICARECEBIMENTO_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetLogisticaRecebimento;
}

class WidgetLogisticaRecebimento : public QWidget {
  Q_OBJECT

public:
  explicit WidgetLogisticaRecebimento(QWidget *parent = 0);
  ~WidgetLogisticaRecebimento();
  bool updateTables();
  void tableFornLogistica_activated(const QString &fornecedor);

signals:
  void errorSignal(const QString &error);

private slots:
  void on_checkBoxMarcarTodos_clicked(const bool);
  void on_lineEditBusca_textChanged(const QString &text);
  void on_pushButtonMarcarRecebido_clicked();
  void on_pushButtonReagendar_clicked();
  void on_table_entered(const QModelIndex &);
  void on_pushButtonVenda_clicked();

private:
  // attributes
  QString fornecedor;
  QString error;
  SqlTableModel model;
  Ui::WidgetLogisticaRecebimento *ui;
  // methods
  bool processRows(const QModelIndexList &list, const QDateTime &dataReceb, const QString &recebidoPor);
  bool reagendar(const QModelIndexList &list, const QDateTime &dataPrevReceb);
  void setupTables();
};

#endif // WIDGETLOGISTICARECEBIMENTO_H
