#ifndef CADASTRARNFE_H
#define CADASTRARNFE_H

#include <QDataWidgetMapper>
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
  void on_doubleSpinBoxCOFINSpcofins_valueChanged(double);
  void on_doubleSpinBoxCOFINSvbc_valueChanged(double);
  void on_doubleSpinBoxICMSpicms_valueChanged(double);
  void on_doubleSpinBoxICMSvbc_valueChanged(double);
  void on_doubleSpinBoxPISppis_valueChanged(double);
  void on_doubleSpinBoxPISvbc_valueChanged(double);
  void on_pushButtonEnviarNFE_clicked();
  void on_pushButtonGerarNFE_clicked();
  void on_tableItens_clicked(const QModelIndex &index);
  void on_tableItens_entered(const QModelIndex &);
  void on_tabWidget_currentChanged(int index);
  void updateImpostos();

private:
  // attributes
  Ui::CadastrarNFe *ui;
  const QString idVenda;
  QString arquivo;
  QString chaveNum;
  SqlTableModel modelLoja;
  SqlTableModel modelProd;
  SqlTableModel modelVenda;
  QDataWidgetMapper mapper;
  // methods
  bool writeDestinatario(QTextStream &stream);
  bool writeEmitente(QTextStream &stream);
  bool writeProduto(QTextStream &stream);
  bool writeTXT(bool test = false);
  QString calculaDigitoVerificador(const QString &chave);
  QString clearStr(const QString &str) const;
  QString criarChaveAcesso();
  QString removeDiacritics(const QString &str) const;
  void guardarNotaBD();
  void setupTables(QString idVenda);
  void writeIdentificacao(QTextStream &stream) const;
  void writeTotal(QTextStream &stream) const;
};

#endif // CADASTRARNFE_H
