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
  bool updateTables();

signals:
  void errorSignal(QString error);

private slots:
  void montaFiltro();
  void on_comboBoxLojas_currentIndexChanged(int);
  void on_groupBoxStatus_toggled(const bool &enabled);
  void on_pushButtonCriarOrc_clicked();
  void on_pushButtonFollowup_clicked();
  void on_table_activated(const QModelIndex &index);
  void on_table_entered(const QModelIndex &);

private:
  // attributes
  Ui::WidgetOrcamento *ui;
  SqlTableModel model;
  // methods
  void setupTables();
  void setPermissions();
  void makeConnections();
};

#endif // WIDGETORCAMENTO_H
