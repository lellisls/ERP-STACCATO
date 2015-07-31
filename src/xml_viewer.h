#ifndef XML_VIEWER_H
#define XML_VIEWER_H

#include <QDialog>
#include <QStandardItemModel>
#include <QDomElement>
#include <QSqlTableModel>

namespace Ui {
  class XML_Viewer;
}

class XML_Viewer : public QDialog {
    Q_OBJECT

  public:
    explicit XML_Viewer(QWidget *parent = 0);
    ~XML_Viewer();
    void exibirXML(QString file);

  private:
    // attributes
    Ui::XML_Viewer *ui;
    QString fileName;
    QStandardItemModel model;
    QSqlTableModel modelProduto;
};

#endif // XML_VIEWER_H
