#ifndef CADASTRARNFE_H
#define CADASTRARNFE_H

#include <QDataWidgetMapper>
#include <QDialog>
#include <QSqlQuery>
#include <QTextStream>

#include "sqltablemodel.h"

namespace Ui {
class CadastrarNFe;
}

class CadastrarNFe : public QDialog {
  Q_OBJECT

public:
  explicit CadastrarNFe(const QString &idVenda, QWidget *parent = 0);
  ~CadastrarNFe();
  void prepararNFe(const QList<int> &items);

private slots:
  void on_comboBoxCfop_currentTextChanged(const QString &text);
  void on_comboBoxCOFINScst_currentTextChanged(const QString &text);
  void on_comboBoxICMSModBc_currentIndexChanged(int index);
  void on_comboBoxICMSModBcSt_currentIndexChanged(int index);
  void on_comboBoxICMSOrig_currentIndexChanged(int index);
  void on_comboBoxIPIcst_currentTextChanged(const QString &text);
  void on_comboBoxPIScst_currentTextChanged(const QString &text);
  void on_comboBoxRegime_2_currentTextChanged(const QString &text);
  void on_comboBoxRegime_currentTextChanged(const QString &text);
  void on_comboBoxSituacaoTributaria_2_currentTextChanged(const QString &text);
  void on_comboBoxSituacaoTributaria_currentTextChanged(const QString &text);
  void on_doubleSpinBoxCOFINSpcofins_valueChanged(double);
  void on_doubleSpinBoxCOFINSvbc_valueChanged(double);
  void on_doubleSpinBoxCOFINSvcofins_valueChanged(double value);
  void on_doubleSpinBoxICMSpicms_valueChanged(double);
  void on_doubleSpinBoxICMSpicmsst_valueChanged(double);
  void on_doubleSpinBoxICMSvbc_valueChanged(double);
  void on_doubleSpinBoxICMSvbcst_valueChanged(double);
  void on_doubleSpinBoxICMSvicms_valueChanged(double value);
  void on_doubleSpinBoxICMSvicmsst_valueChanged(double value);
  void on_doubleSpinBoxPISppis_valueChanged(double);
  void on_doubleSpinBoxPISvbc_valueChanged(double);
  void on_doubleSpinBoxPISvpis_valueChanged(double value);
  void on_doubleSpinBoxValorFrete_valueChanged(double value);
  void on_itemBoxCliente_textChanged(const QString &);
  void on_itemBoxEnderecoEntrega_textChanged(const QString &);
  void on_itemBoxEnderecoFaturamento_textChanged(const QString &);
  void on_itemBoxVeiculo_textChanged(const QString &);
  void on_pushButtonConsultarCadastro_clicked();
  void on_pushButtonEnviarNFE_clicked();
  void on_tableItens_clicked(const QModelIndex &index);
  void on_tableItens_entered(const QModelIndex &);
  void on_tabWidget_currentChanged(int index);
  void updateImpostos();
  void alterarCertificado(const QString &text);

private:
  // attributes
  //int cnpjCertificado;
  const QString idVenda;
  QDataWidgetMapper mapper;
  QSqlQuery queryCliente;
  QSqlQuery queryEndereco;
  QSqlQuery queryIBGE;
  QSqlQuery queryLojaEnd;
  QString arquivo;
  QString chaveNum;
  QString error;
  QString xml;
  SqlTableModel modelLoja;
  SqlTableModel modelProd;
  SqlTableModel modelVenda;
  Ui::CadastrarNFe *ui;
  // methods
  bool cadastrar(const QVariant &id);
  bool calculaDigitoVerificador(QString &chave);
  bool criarChaveAcesso();
  bool preencherNumeroNFe();
  bool validar();
  QString clearStr(const QString &str) const;
  void setupTables();
  void writeDestinatario(QTextStream &stream) const;
  void writeEmitente(QTextStream &stream) const;
  void writeIdentificacao(QTextStream &stream) const;
  void writeProduto(QTextStream &stream) const;
  void writeTotal(QTextStream &stream) const;
  void writeTransportadora(QTextStream &stream) const;
  void writeVolume(QTextStream &stream) const;
};

#endif // CADASTRARNFE_H
