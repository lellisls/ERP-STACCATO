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
  void TableFornLogistica_activated(const QString &fornecedor);

signals:
  void errorSignal(QString error);

private slots:
  void on_checkBoxMarcarTodos_clicked(const bool &);
  void on_pushButtonMarcarRecebido_clicked();
  void on_table_entered(const QModelIndex &);
  void on_lineEditBuscaRecebimento_textChanged(const QString &text);

private:
  // attributes
  Ui::WidgetLogisticaRecebimento *ui;
  SqlTableModel model;
  // methods
  void setupTables();
};

#endif // WIDGETLOGISTICARECEBIMENTO_H
