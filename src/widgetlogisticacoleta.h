#ifndef WIDGETLOGISTICACOLETA_H
#define WIDGETLOGISTICACOLETA_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
  class WidgetLogisticaColeta;
}

class WidgetLogisticaColeta : public QWidget
{
    Q_OBJECT

  public:
    explicit WidgetLogisticaColeta(QWidget *parent = 0);
    ~WidgetLogisticaColeta();
    QString updateTables();
    void TableFornLogistica_activated(const QString &fornecedor);

  private slots:
    void on_pushButtonMarcarColetado_clicked();

    void on_checkBoxMarcarTodos_clicked(const bool &checked);

  private:
    Ui::WidgetLogisticaColeta *ui;
    SqlTableModel modelColeta;
    void setupTables();
};

#endif // WIDGETLOGISTICACOLETA_H
