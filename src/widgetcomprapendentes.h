#ifndef WIDGETCOMPRAPENDENTES_H
#define WIDGETCOMPRAPENDENTES_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
  class WidgetCompraPendentes;
}

class WidgetCompraPendentes : public QWidget {
    Q_OBJECT

  public:
    explicit WidgetCompraPendentes(QWidget *parent = 0);
    ~WidgetCompraPendentes();
    QString updateTables();

  private slots:
    void on_tableProdutosPend_activated(const QModelIndex &index);
    void on_groupBoxStatus_toggled(const bool &enabled);
    void montaFiltro();

  private:
    // attributes
    Ui::WidgetCompraPendentes *ui;
    SqlTableModel modelProdPend;
    // methods
    void setupTables();
};

#endif // WIDGETCOMPRAPENDENTES_H
