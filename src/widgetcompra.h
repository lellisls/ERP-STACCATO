#ifndef WIDGETCOMPRA_H
#define WIDGETCOMPRA_H

#include <QWidget>

namespace Ui {
class WidgetCompra;
}

class WidgetCompra : public QWidget {
  Q_OBJECT

public:
  explicit WidgetCompra(QWidget *parent = 0);
  ~WidgetCompra();
  bool updateTables();

signals:
  void errorSignal(const QString &error);

private slots:
  void on_tabWidget_currentChanged(const int &);

private:
  Ui::WidgetCompra *ui;
};

#endif // WIDGETCOMPRA_H
