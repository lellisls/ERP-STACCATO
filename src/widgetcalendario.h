#ifndef WIDGETCALENDARIO_H
#define WIDGETCALENDARIO_H

#include <QWidget>

namespace Ui {
class WidgetCalendario;
}

class WidgetCalendario : public QWidget {
  Q_OBJECT

public:
  explicit WidgetCalendario(QWidget *parent = 0);
  ~WidgetCalendario();
  bool updateTables();

private slots:
  void on_checkBoxMostrarFiltros_toggled(bool checked);
  void on_pushButtonProximo_clicked();
  void on_pushButtonAnterior_clicked();
  void on_calendarWidget_selectionChanged();

private:
  // attributes
  bool setup = false;
  Ui::WidgetCalendario *ui;
  // methods
  bool updateCalendar(const QDate &startDate);
  void updateFilter();
};

#endif // WIDGETCALENDARIO_H
