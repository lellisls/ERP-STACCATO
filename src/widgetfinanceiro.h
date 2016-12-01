#ifndef WIDGETFINANCEIRO_H
#define WIDGETFINANCEIRO_H

#include <QWidget>

namespace Ui {
class WidgetFinanceiro;
}

class WidgetFinanceiro : public QWidget {
  Q_OBJECT

public:
  explicit WidgetFinanceiro(QWidget *parent = 0);
  ~WidgetFinanceiro();
  bool updateTables();

signals:
  void errorSignal(const QString &error);

private:
  Ui::WidgetFinanceiro *ui;
};

#endif // WIDGETFINANCEIRO_H
