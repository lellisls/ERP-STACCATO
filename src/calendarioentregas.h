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
  void errorSignal(const QString &error);

private slots:
  void on_pushButtonConfirmarEntrega_clicked();
  void on_pushButtonGerarNFeEntregar_clicked();
  void on_pushButtonImprimirDanfe_clicked();
  void on_pushButtonReagendar_clicked();
  void on_tableCalendario_clicked(const QModelIndex &index);
  void on_tableCarga_clicked(const QModelIndex &index);

private:
  // attributes
  SqlTableModel modelCalendario;
  SqlTableModel modelCarga;
  SqlTableModel modelProdutos;
  Ui::CalendarioEntregas *ui;
  // methods
  bool confirmarEntrega();
  bool imprimirDanfe();
  bool reagendar();
  void setupTables();
};

#endif // CALENDARIOENTREGAS_H
