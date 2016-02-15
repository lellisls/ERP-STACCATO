#ifndef VENDASMES_H
#define VENDASMES_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
  class VendasMes;
}

class VendasMes : public QWidget {
    Q_OBJECT

  public:
    explicit VendasMes(const int usuario, QWidget *parent = 0);
    ~VendasMes();

  private slots:
    void on_table_entered(const QModelIndex &);

  private:
    // attributes
    Ui::VendasMes *ui;
    SqlTableModel model;
    // methods
    void setupTables(const int usuario);
};

#endif // VENDASMES_H
