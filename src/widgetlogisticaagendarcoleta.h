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
  void on_pushButton_clicked();
  void calcularPeso();
  void on_pushButtonSendLeft_clicked();
  void on_pushButtonSendRight_clicked();

private:
  Ui::WidgetLogisticaAgendarColeta *ui;
  SqlTableModel modelEstoque;
  SqlTableModel modelTransp;
  QStandardItemModel *treeModel;
  void setupTables();
};

#endif // WIDGETLOGISTICAAGENDARCOLETA_H
