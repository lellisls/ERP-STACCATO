#ifndef XML_VIEWER_H
#define XML_VIEWER_H

#include <QDialog>
#include <QStandardItemModel>
#include <QDomElement>

namespace Ui {
  class XML_Viewer;
}

class XML_Viewer : public QDialog {
    Q_OBJECT

  public:
    explicit XML_Viewer(QWidget *parent = 0);
    ~XML_Viewer();
    void exibirXML(const QString &fileContent);

  private:
    // attributes
    Ui::XML_Viewer *ui;
    QString fileName;
    QStandardItemModel model;
    // methods
};

#endif // XML_VIEWER_H
