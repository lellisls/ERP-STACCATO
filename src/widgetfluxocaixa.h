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
  void errorSignal(const QString &error);

private slots:
  void on_tableResumo_entered(const QModelIndex &);
  void on_tableCaixa_entered(const QModelIndex &);

private:
  // attributes
  SqlTableModel modelCaixa;
  SqlTableModel modelResumo;
  Ui::WidgetFluxoCaixa *ui;
  // methods
  bool montaFiltro();
  void setupTables();
};

#endif // WIDGETFLUXOCAIXA_H
