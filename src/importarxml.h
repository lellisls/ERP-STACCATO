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
    explicit ImportarXML(const QString &fornecedor, QWidget *parent = 0);
    ~ImportarXML();
    QString getIdCompra();

  private slots:
    void on_pushButtonImportar_clicked();
    void on_pushButtonProcurar_clicked();
    void on_tableEstoque_clicked(const QModelIndex &index);

  private:
    // attributes
    Ui::ImportarXML *ui;
    QDataWidgetMapper mapper;
    QString idCompra;
    SqlTableModel modelCompra;
    SqlTableModel modelEstoque;
    // methods
    void closeEvent(QCloseEvent *event);
    void setupTables();

    enum FieldColors {
      Green = 1,  // Ok
      Yellow = 2, // Quant difere
      Red = 3,    // Não encontrado
    };
};

#endif // IMPORTARXML_H
