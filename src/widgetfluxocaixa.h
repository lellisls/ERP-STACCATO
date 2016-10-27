#ifndef WIDGETFLUXOCAIXA_H
#define WIDGETFLUXOCAIXA_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetFluxoCaixa;
}

class WidgetFluxoCaixa : public QWidget {
  Q_OBJECT

public:
  explicit WidgetFluxoCaixa(QWidget *parent = 0);
  ~WidgetFluxoCaixa();
  bool updateTables();

signals:
  void errorSignal(QString error);

private slots:
  void on_tableResumo_entered(const QModelIndex &);
  void on_tableCaixa_entered(const QModelIndex &);

private:
  // attributes
  SqlTableModel model;
  SqlTableModel model2;
  Ui::WidgetFluxoCaixa *ui;
  // methods
  void montaFiltro();
  void setupTables();
};

#endif // WIDGETFLUXOCAIXA_H
