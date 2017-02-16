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
  void on_lineEditBuscar_textChanged(const QString &text);
  void on_pushButtonCancelarEntrega_clicked();
  void on_pushButtonConfirmarEntrega_clicked();
  void on_pushButtonGerarNFeEntregar_clicked();
  void on_pushButtonImprimirDanfe_clicked();
  void on_pushButtonReagendar_clicked();
  void on_tableCalendario_clicked(const QModelIndex &index);
  void on_tableCarga_clicked(const QModelIndex &index);

private:
  // attributes
  QString error;
  SqlTableModel modelCalendario;
  SqlTableModel modelCarga;
  SqlTableModel modelProdutos;
  Ui::CalendarioEntregas *ui;
  // methods
  bool cancelarEntrega(const QModelIndexList &list);
  bool confirmarEntrega(const QDateTime &dataRealEnt, const QString &entregou, const QString &recebeu);
  bool reagendar(const QModelIndexList &list, const QDateTime &dataPrevEnt);
  void setupTables();
};

#endif // CALENDARIOENTREGAS_H
