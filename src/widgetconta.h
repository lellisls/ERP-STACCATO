#ifndef WIDGETCONTA_H
#define WIDGETCONTA_H

#include "sqltablemodel.h"

#include <QWidget>

namespace Ui {
  class WidgetConta;
}

class WidgetConta : public QWidget {
    Q_OBJECT

  public:
    explicit WidgetConta(QWidget *parent = 0);
    ~WidgetConta();
    bool updateTables();

  private slots:
    void on_radioButtonContaPagarLimpar_clicked();
    void on_radioButtonContaPagarPago_clicked();
    void on_radioButtonContaPagarPendente_clicked();
    void on_radioButtonContaReceberLimpar_clicked();
    void on_radioButtonContaReceberRecebido_clicked();
    void on_radioButtonContaReceberPendente_clicked();
    void on_tableContasPagar_activated(const QModelIndex &index);
    void on_tableContasReceber_activated(const QModelIndex &index);
    void on_lineEditBuscaContasPagar_textChanged(const QString &text);
    void on_lineEditBuscaContasReceber_textChanged(const QString &text);

  private:
    Ui::WidgetConta *ui;
    SqlTableModel *modelCAPagar;
    SqlTableModel *modelCAReceber;
    void setupTables();
};

#endif // WIDGETCONTA_H
