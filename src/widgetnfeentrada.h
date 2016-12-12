#ifndef WIDGETNFEENTRADA_H
#define WIDGETNFEENTRADA_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetNfeEntrada;
}

class WidgetNfeEntrada : public QWidget {
  Q_OBJECT

public:
  explicit WidgetNfeEntrada(QWidget *parent = 0);
  ~WidgetNfeEntrada();
  bool updateTables();

signals:
  void errorSignal(const QString &error);

private slots:
  void on_table_activated(const QModelIndex &index);
  void on_table_entered(const QModelIndex &);
  void on_lineEditBusca_textChanged(const QString &text);

private:
  // attributes
  SqlTableModel model;
  Ui::WidgetNfeEntrada *ui;
  // methods
  void setupTables();
};

#endif // WIDGETNFEENTRADA_H
