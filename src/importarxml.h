#ifndef IMPORTARXML_H
#define IMPORTARXML_H

#include <QDataWidgetMapper>
#include <QFileDialog>

#include "sqltablemodel.h"

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
    void on_pushButtonCancelar_clicked();
    void on_pushButtonImportar_clicked();
    void on_pushButtonProcurar_clicked();
    void on_pushButtonReparear_clicked();

  private:
    // attributes
    Ui::ImportarXML *ui;
    QString m_idCompra;
    SqlTableModel modelCompra;
    SqlTableModel modelEstoque;
    int idNFe;

    enum FieldColors {
      White = 0,     // Não processado
      Green = 1,     // Ok
      Yellow = 2,    // Quant difere
      Red = 3,       // Não encontrado
      DarkGreen = 4, // Consumo
    };

    // methods
    int lerXML(QFile &file);
    void associarItens(QModelIndex &item, int row, int idNFe, double &estoqueConsumido);
    void closeEvent(QCloseEvent *event);
    void criarConsumo(QModelIndex &item, QString codComercial, QString codBarras, QString idCompra, int row);
    void limparAssociacoes();
    void setarIdCompraNFe(const int &idNFe);
    void setupTables(const QString &fornecedor);
};

#endif // IMPORTARXML_H
