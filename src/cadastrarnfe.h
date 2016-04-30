#ifndef CADASTRARNFE_H
#define CADASTRARNFE_H

#include <QDialog>
#include <QTextStream>

#include "sqltablemodel.h"

namespace Ui {
class CadastrarNFe;
}

class CadastrarNFe : public QDialog {
  Q_OBJECT

public:
  explicit CadastrarNFe(QString idVenda, QWidget *parent = 0);
  ~CadastrarNFe();
  void prepararNFe(const QList<int> &items);

private slots:
  void on_pushButtonEnviarNFE_clicked();
  void on_pushButtonGerarNFE_clicked();
  void on_tableItens_activated(const QModelIndex &);
  void on_tableItens_entered(const QModelIndex &);
  void on_tableItens_pressed(const QModelIndex &);
  void updateImpostos();

private:
  // attributes
  Ui::CadastrarNFe *ui;
  const QString idVenda;
  QString arquivo;
  QString chaveAcesso;
  QString chaveNum;
  SqlTableModel modelLoja;
  SqlTableModel modelProd;
  SqlTableModel modelVenda;
  // methods
  bool writeDestinatario(QTextStream &stream);
  bool writeEmitente(QTextStream &stream);
  bool writeProduto(QTextStream &stream, double &total, double &icmsTotal);
  bool writeTXT();
  QString calculaDigitoVerificador(const QString &chave);
  QString clearStr(const QString &str) const;
  QString criarChaveAcesso();
  QString removeDiacritics(const QString &str) const;
  void guardarNotaBD();
  void writeIdentificacao(QTextStream &stream) const;
  void writeTotal(QTextStream &stream, double &total, double &icmsTotal, double &frete) const;
  void setupTables(QString idVenda);
};

#endif // CADASTRARNFE_H
