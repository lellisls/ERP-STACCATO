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
  void tableFornLogistica_activated(const QString &fornecedor);

signals:
  void errorSignal(const QString &error);

private slots:
  void on_checkBoxMarcarTodos_clicked(const bool);
  void on_lineEditBusca_textChanged(const QString &);
  void on_pushButtonMarcarColetado_clicked();
  void on_pushButtonReagendar_clicked();
  void on_table_doubleClicked(const QModelIndex &index);
  void on_table_entered(const QModelIndex &);

private:
  // attributes
  SqlTableModel model;
  QString fornecedor;
  Ui::WidgetLogisticaColeta *ui;
  // methods
  bool cadastrar();
  bool reagendar();
  void setupTables();
};

#endif // WIDGETLOGISTICACOLETA_H
