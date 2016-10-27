#ifndef WIDGETLOGISTICAREPRESENTACAO_H
#define WIDGETLOGISTICAREPRESENTACAO_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetLogisticaRepresentacao;
}

class WidgetLogisticaRepresentacao : public QWidget {
  Q_OBJECT

public:
  explicit WidgetLogisticaRepresentacao(QWidget *parent = 0);
  ~WidgetLogisticaRepresentacao();
  bool updateTables();
  void TableFornLogistica_activated(const QString &fornecedor);

signals:
  void errorSignal(QString error);

private slots:
  void on_lineEditBusca_textChanged(const QString &text);
  void on_pushButtonMarcarEntregue_clicked();
  void on_table_entered(const QModelIndex &);

private:
  // attributes
  Ui::WidgetLogisticaRepresentacao *ui;
  SqlTableModel model;
  // methods
  void setupTables();
};

#endif // WIDGETLOGISTICAREPRESENTACAO_H
