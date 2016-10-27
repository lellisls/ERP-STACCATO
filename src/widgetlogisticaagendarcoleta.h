#ifndef WIDGETLOGISTICAAGENDARCOLETA_H
#define WIDGETLOGISTICAAGENDARCOLETA_H

#include <QStandardItemModel>
#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetLogisticaAgendarColeta;
}

class WidgetLogisticaAgendarColeta : public QWidget {
  Q_OBJECT

public:
  explicit WidgetLogisticaAgendarColeta(QWidget *parent = 0);
  ~WidgetLogisticaAgendarColeta();
  bool updateTables();
  void TableFornLogistica_activated(const QString &fornecedor);

signals:
  void errorSignal(QString error);

private slots:
  void calcularPeso();
  void on_pushButtonAgendarColeta_clicked();
  void on_pushButtonMontarCarga_clicked();
  void on_pushButtonSendLeft_clicked();
  void on_pushButtonSendRight_clicked();
  void on_tableEstoque_entered(const QModelIndex &);

private:
  // attributes
  QStandardItemModel *treeModel;
  SqlTableModel modelEstoque;
  SqlTableModel modelTransp;
  Ui::WidgetLogisticaAgendarColeta *ui;
  // methods
  bool processRows(const QModelIndexList &list);
  void setupTables();
};

#endif // WIDGETLOGISTICAAGENDARCOLETA_H
