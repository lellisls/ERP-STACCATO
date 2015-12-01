#ifndef WIDGETNFE_H
#define WIDGETNFE_H

#include "sqltablemodel.h"

#include <QWidget>

namespace Ui {
  class WidgetNfe;
}

class WidgetNfe : public QWidget {
    Q_OBJECT

  public:
    explicit WidgetNfe(QWidget *parent = 0);
    ~WidgetNfe();
    QString updateTables();

  private slots:
    void on_radioButtonNFeAutorizado_clicked();
    void on_radioButtonNFeEnviado_clicked();
    void on_radioButtonNFeLimpar_clicked();
    void on_lineEditBuscaNFe_textChanged(const QString &text);
    void on_tableNfeSaida_activated(const QModelIndex &index);
    void on_tableNfeEntrada_activated(const QModelIndex &index);
    void on_tabWidgetNfe_currentChanged(const int &);
    void on_pushButtonExibirXML_clicked();

  private:
    Ui::WidgetNfe *ui;
    SqlTableModel modelNfeEntrada;
    SqlTableModel modelNfeSaida;
    void setupTables();
};

#endif // WIDGETNFE_H
