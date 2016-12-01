#ifndef XML_VIEWER_H
#define XML_VIEWER_H

#include <QDialog>
#include <QDomElement>
#include <QStandardItemModel>

namespace Ui {
class XML_Viewer;
}

class XML_Viewer : public QDialog {
  Q_OBJECT

public:
  explicit XML_Viewer(QWidget *parent = 0);
  ~XML_Viewer();
  void exibirXML(const QByteArray &fileContent);

private slots:
  void on_pushButtonDanfe_clicked();

private:
  // attributes
  QByteArray fileContent;
  QStandardItemModel model;
  QString fileName;
  Ui::XML_Viewer *ui;
  // methods
  bool imprimirDanfe();
};

#endif // XML_VIEWER_H
