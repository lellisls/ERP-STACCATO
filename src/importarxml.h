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
  explicit ImportarXML(const QStringList &idsCompra, const QString &dataReal, QWidget *parent = 0);
  ~ImportarXML();

private slots:
  void on_pushButtonCancelar_clicked();
  void on_pushButtonImportar_clicked();
  void on_pushButtonProcurar_clicked();
  void on_pushButtonRemover_clicked();
  void on_pushButtonReparear_clicked();
  void on_tableCompra_entered(const QModelIndex &);
  void on_tableConsumo_entered(const QModelIndex &);
  void on_tableEstoque_entered(const QModelIndex &);

private:
  // attributes
  Ui::ImportarXML *ui;
  int idNFe;
  SqlTableModel modelCompra;
  SqlTableModel modelEstoque;
  SqlTableModel modelConsumo;
  QString dataReal;
  QStringList idsCompra;

  enum FieldColors {
    White = 0,     // Não processado
    Green = 1,     // Ok
    Yellow = 2,    // Quant difere
    Red = 3,       // Não encontrado
    DarkGreen = 4, // Consumo
  };

  // methods
  bool lerXML(QFile &file);
  void associarItens(int rowCompra, int rowEstoque, double &estoqueConsumido);
  void criarConsumo();
  void limparAssociacoes();
  void parear();
  void setupTables(const QStringList &idsCompra);
};

#endif // IMPORTARXML_H
