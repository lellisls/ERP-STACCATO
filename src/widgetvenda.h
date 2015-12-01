#ifndef WIDGETVENDA_H
#define WIDGETVENDA_H

#include "sqltablemodel.h"

#include <QWidget>

namespace Ui {
  class WidgetVenda;
}

class WidgetVenda : public QWidget {
    Q_OBJECT

  public:
    explicit WidgetVenda(QWidget *parent = 0);
    ~WidgetVenda();
    QString updateTables();

  private slots:
    void montaFiltro();
    void on_groupBoxStatusVenda_toggled(const bool &enabled);
    void on_comboBoxLojas_currentTextChanged(const QString &);
    void on_lineEditBuscaVendas_textChanged(const QString &text);
    void on_tableVendas_activated(const QModelIndex &index);

  private:
    Ui::WidgetVenda *ui;
    SqlTableModel modelVendas;
    void setupTables();
};

#endif // WIDGETVENDA_H
