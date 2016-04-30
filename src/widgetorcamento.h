#ifndef WIDGETORCAMENTO_H
#define WIDGETORCAMENTO_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetOrcamento;
}

class WidgetOrcamento : public QWidget {
  Q_OBJECT

public:
  explicit WidgetOrcamento(QWidget *parent = 0);
  ~WidgetOrcamento();
  bool updateTables(QString &error);

private slots:
  void montaFiltro();
  void on_pushButtonCriarOrc_clicked();
  void on_table_activated(const QModelIndex &index);
  void on_table_entered(const QModelIndex &);
  void on_pushButtonFollowup_clicked();
  void on_groupBoxStatus_toggled(const bool &enabled);

private:
  // attributes
  Ui::WidgetOrcamento *ui;
  SqlTableModel model;
  // methods
  void setupTables();
};

#endif // WIDGETORCAMENTO_H
