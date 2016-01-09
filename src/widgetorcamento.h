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
    QString updateTables();

  private slots:
    void montaFiltro();
    void on_pushButtonCriarOrc_clicked();
    void on_table_activated(const QModelIndex &index);

  private:
    // attributes
    Ui::WidgetOrcamento *ui;
    SqlTableModel model;
    // methods
    void setupTables();
};

#endif // WIDGETORCAMENTO_H
