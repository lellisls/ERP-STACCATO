#ifndef IMPORTARXML_H
#define IMPORTARXML_H

#include <QDataWidgetMapper>
#include <QDate>
#include <QFileDialog>

#include "sqltablemodel.h"
#include "xml.h"

namespace Ui {
class ImportarXML;
}

class ImportarXML : public QDialog {
  Q_OBJECT

public:
  explicit ImportarXML(const QStringList &idsCompra, const QDateTime &dataReal, QWidget *parent = 0);
  ~ImportarXML();

private slots:
  void on_pushButtonCancelar_clicked();
  void on_pushButtonImportar_clicked();
  void on_pushButtonProcurar_clicked();
  void on_tableCompra_entered(const QModelIndex &);
  void on_tableConsumo_entered(const QModelIndex &);
  void on_tableEstoque_entered(const QModelIndex &);

private:
  // attributes
  const QDateTime dataReal;
  const QStringList idsCompra;
  QString error;
  SqlTableModel modelCompra;
  SqlTableModel modelConsumo;
  SqlTableModel modelEstoque;
  SqlTableModel modelEstoque_nfe;
  SqlTableModel modelEstoque_compra;
  SqlTableModel modelNFe;
  Ui::ImportarXML *ui;

  enum FieldColors {
    White = 0,     // Não processado
    Green = 1,     // Ok
    Yellow = 2,    // Quant difere
    Red = 3,       // Não encontrado
    DarkGreen = 4, // Consumo
  };

  // methods
  bool associarItens(const int rowCompra, const int rowEstoque, double &estoqueConsumido);
  bool cadastrarNFe(XML &xml);
  bool criarConsumo();
  bool importar();
  bool inserirItemSql(XML &xml);
  bool inserirNoSqlModel(XML &xml, const QStandardItem *item);
  bool lerXML(QFile &file);
  bool limparAssociacoes();
  void WrapParear();
  bool perguntarLocal(XML &xml);
  bool verificaCNPJ(const XML &xml);
  bool verificaExiste(XML &xml);
  void setupTables(const QStringList &idsCompra);
  bool criarConsumo2(const int rowCompra, const int rowEstoque, const double quantAdicionar);
  void procurar();
  bool parear();
};

#endif // IMPORTARXML_H
