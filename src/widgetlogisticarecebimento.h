#ifndef WIDGETLOGISTICARECEBIMENTO_H
#define WIDGETLOGISTICARECEBIMENTO_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
  class WidgetLogisticaRecebimento;
}

class WidgetLogisticaRecebimento : public QWidget
{
    Q_OBJECT

  public:
    explicit WidgetLogisticaRecebimento(QWidget *parent = 0);
    ~WidgetLogisticaRecebimento();
    QString updateTables();
    void TableFornLogistica_activated(const QString &fornecedor);

  private slots:
    void on_pushButtonMarcarRecebido_clicked();

    void on_checkBoxMarcarTodos_clicked(const bool &checked);

  private:
    Ui::WidgetLogisticaRecebimento *ui;
    SqlTableModel modelReceb;
    void setupTables();
};

#endif // WIDGETLOGISTICARECEBIMENTO_H
