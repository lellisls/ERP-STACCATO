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
    bool updateTables(QString &error);

  private slots:
    void montaFiltro();
    void on_doubleSpinBoxQuantAvulso_valueChanged(const double &value);
    void on_doubleSpinBoxQuantAvulsoCaixas_valueChanged(const double &value);
    void on_groupBoxStatus_toggled(const bool &enabled);
    void on_pushButtonComprarAvulso_clicked();
    void on_table_activated(const QModelIndex &index);
    void on_table_entered(const QModelIndex &);
    void setarDadosAvulso();

  private:
    // attributes
    Ui::WidgetCompraPendentes *ui;
    SqlTableModel model;
    // methods
    bool atualiza(const QSqlQuery &query);
    bool insere(const QDate &dataPrevista);
    void setupTables();
};

#endif // WIDGETCOMPRAPENDENTES_H
