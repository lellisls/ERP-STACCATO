#ifndef WIDGETNFESAIDA_H
#define WIDGETNFESAIDA_H

#include "sqltablemodel.h"

#include <QWidget>

namespace Ui {
  class WidgetNfeSaida;
}

class WidgetNfeSaida : public QWidget {
    Q_OBJECT

  public:
    explicit WidgetNfeSaida(QWidget *parent = 0);
    ~WidgetNfeSaida();
    QString updateTables();

  private slots:
    void on_table_activated(const QModelIndex &index);
    void on_radioButtonAutorizado_clicked();
    void on_radioButtonEnviado_clicked();
    void on_radioButtonTodos_clicked();
    void on_lineEditBusca_textChanged(const QString &text);

  private:
    Ui::WidgetNfeSaida *ui;
    SqlTableModel model;
    void setupTables();
};

#endif // WIDGETNFESAIDA_H
