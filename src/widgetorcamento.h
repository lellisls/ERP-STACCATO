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
    bool updateTables();

  private slots:
    void on_radioButtonOrcValido_clicked();
    void on_radioButtonOrcExpirado_clicked();
    void on_radioButtonOrcLimpar_clicked();
    void on_radioButtonOrcProprios_clicked();
    void on_lineEditBuscaOrcamentos_textChanged(const QString &text);
    void on_tableOrcamentos_activated(const QModelIndex &index);
    void on_pushButtonCriarOrc_clicked();

  private:
    Ui::WidgetOrcamento *ui;
    SqlTableModel modelOrcamento;
    void setupTables();
};

#endif // WIDGETORCAMENTO_H
