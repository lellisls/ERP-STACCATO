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
  void on_lineEditBusca_textChanged(const QString &text);
  void on_pushButtonCancelarNFe_clicked();
  void on_table_activated(const QModelIndex &index);
  void on_table_entered(const QModelIndex &);

private:
  // attributes
  SqlTableModel model;
  QString error;
  Ui::WidgetNfeEntrada *ui;
  // methods
  void setupTables();
  bool cancelar(const int row);
};

#endif // WIDGETNFEENTRADA_H
