#ifndef IMPORTARXML_H
#define IMPORTARXML_H

#include <QDialog>
#include <QStandardItemModel>
#include <QDataWidgetMapper>

#include "xml.h"

namespace Ui {
  class ImportarXML;
}

class ImportarXML : public QDialog {
    Q_OBJECT

  public:
    explicit ImportarXML(QList<int> rows, QWidget *parent = 0);
    ~ImportarXML();

  public slots:
    void show();

  private slots:
    void on_pushButtonCancelar_clicked();
    void on_pushButtonImportar_clicked();
    void on_pushButtonProcurar_clicked();
    void on_tableEstoque_clicked(const QModelIndex &index);

  private:
    // attributes
    Ui::ImportarXML *ui;
    SqlTableModel modelEstoque, modelCompra;
    QDataWidgetMapper mapper;
    XML *xml = nullptr;
    // methods
};

#endif // IMPORTARXML_H
