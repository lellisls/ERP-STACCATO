#ifndef WIDGETNFE_H
#define WIDGETNFE_H

#include <QWidget>

namespace Ui {
class WidgetNfe;
}

class WidgetNfe : public QWidget {
  Q_OBJECT

public:
  explicit WidgetNfe(QWidget *parent = 0);
  ~WidgetNfe();
  bool updateTables(QString &error);

private slots:
  void on_tabWidgetNfe_currentChanged(const int &);
  void on_pushButtonExibirXML_clicked();

private:
  Ui::WidgetNfe *ui;
};

#endif // WIDGETNFE_H
