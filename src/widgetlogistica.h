#ifndef WIDGETLOGISTICA_H
#define WIDGETLOGISTICA_H

#include <QWidget>

#include "src/sqltablemodel.h"

namespace Ui {
  class WidgetLogistica;
}

class WidgetLogistica : public QWidget {
    Q_OBJECT

  public:
    explicit WidgetLogistica(QWidget *parent = 0);
    ~WidgetLogistica();
    bool updateTables();

  private slots:
    void on_radioButtonEntregaLimpar_clicked();
    void on_radioButtonEntregaEnviado_clicked();
    void on_radioButtonEntregaPendente_clicked();
    void on_lineEditBuscaEntregas_textChanged(const QString &text);
    void on_tableEntregasCliente_activated(const QModelIndex &index);
    void on_pushButtonMarcarColetado_clicked();
    void on_pushButtonMarcarRecebido_clicked();
    void on_tableFornLogistica_activated(const QModelIndex &index);
    void on_tabWidgetLogistica_currentChanged(const int&);

  private:
    Ui::WidgetLogistica *ui;
    SqlTableModel *modelPedForn2;
    SqlTableModel *modelColeta;
    SqlTableModel *modelReceb;
    SqlTableModel *modelEntregasCliente;
    void setupTables();
};

#endif // WIDGETLOGISTICA_H
