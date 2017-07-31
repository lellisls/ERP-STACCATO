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
  void on_groupBoxCaixa1_toggled(const bool checked);
  void on_groupBoxCaixa2_toggled(const bool checked);
  void on_tableCaixa_activated(const QModelIndex &index);
  void on_tableCaixa_entered(const QModelIndex &);
  void on_tableCaixa2_activated(const QModelIndex &index);
  void on_tableCaixa2_entered(const QModelIndex &);

private:
  // attributes
  bool isReady = false;
  QSqlQueryModel modelCaixa;
  QSqlQueryModel modelCaixa2;
  QSqlQueryModel modelFuturo;
  Ui::WidgetFluxoCaixa *ui;
  // methods
  void montaFiltro();
};

#endif // WIDGETFLUXOCAIXA_H
