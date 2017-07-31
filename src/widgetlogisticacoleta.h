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
  void on_table_entered(const QModelIndex &);
  void on_pushButtonVenda_clicked();
  void on_pushButtonCancelar_clicked();

private:
  // attributes
  QString fornecedor;
  QString error;
  SqlTableModel model;
  Ui::WidgetLogisticaColeta *ui;
  // methods
  bool cadastrar(const QModelIndexList &list, const QDate &dataColeta, const QDate &dataPrevReceb);
  bool reagendar(const QModelIndexList &list, const QDate &dataPrevColeta);
  void setupTables();
  bool cancelar(const QModelIndexList &list);
};

#endif // WIDGETLOGISTICACOLETA_H
