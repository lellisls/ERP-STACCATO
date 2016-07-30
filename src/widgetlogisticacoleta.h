#ifndef WIDGETLOGISTICACOLETA_H
#define WIDGETLOGISTICACOLETA_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetLogisticaColeta;
}

class WidgetLogisticaColeta : public QWidget {
  Q_OBJECT

public:
  explicit WidgetLogisticaColeta(QWidget *parent = 0);
  ~WidgetLogisticaColeta();
  bool updateTables();
  void TableFornLogistica_activated(const QString &fornecedor);

signals:
  void errorSignal(QString error);

private slots:
  void on_checkBoxMarcarTodos_clicked(const bool &);
  void on_pushButtonMarcarColetado_clicked();
  void on_table_entered(const QModelIndex &);
  void on_lineEditBuscaColeta_textChanged(const QString &);

private:
  // attributes
  Ui::WidgetLogisticaColeta *ui;
  SqlTableModel model;
  // methods
  void setupTables();
};

#endif // WIDGETLOGISTICACOLETA_H
