#ifndef CALENDARIOENTREGAS_H
#define CALENDARIOENTREGAS_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class CalendarioEntregas;
}

class CalendarioEntregas : public QWidget {
  Q_OBJECT

public:
  explicit CalendarioEntregas(QWidget *parent = 0);
  ~CalendarioEntregas();
  bool updateTables();

signals:
  void errorSignal(QString error);

private slots:
  void on_table_activated(const QModelIndex &index);

private:
  // attributes
  Ui::CalendarioEntregas *ui;
  SqlTableModel model;
  // methods
  void setupTables();
};

#endif // CALENDARIOENTREGAS_H
