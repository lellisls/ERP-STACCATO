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

  public slots:
    QString updateTables();

  private slots:
    void montaFiltro();
    void on_pushButtonCriarOrc_clicked();
    void on_tableOrcamentos_activated(const QModelIndex &index);

  private:
    // attributes
    Ui::WidgetOrcamento *ui;
    SqlTableModel modelOrcamento;
    // methods
    void setupTables();
};

#endif // WIDGETORCAMENTO_H
