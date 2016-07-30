#ifndef VENDA_H
#define VENDA_H

#include "registerdialog.h"

namespace Ui {
class Venda;
}

class Venda : public RegisterDialog {
  Q_OBJECT

public:
  explicit Venda(QWidget *parent = 0);
  ~Venda();
  void prepararVenda(const QString &idOrcamento);

private slots:
  void montarFluxoCaixa();
  void on_checkBoxFreteManual_clicked(const bool &checked);
  void on_comboBoxPgt1_currentTextChanged(const QString &text);
  void on_comboBoxPgt2_currentTextChanged(const QString &text);
  void on_comboBoxPgt3_currentTextChanged(const QString &text);
  void on_dateTimeEdit_dateTimeChanged(const QDateTime &);
  void on_doubleSpinBoxDescontoGlobal_valueChanged(const double &);
  void on_doubleSpinBoxDescontoGlobalReais_valueChanged(const double &);
  void on_doubleSpinBoxFrete_valueChanged(const double &);
  void on_doubleSpinBoxPgt1_valueChanged(double);
  void on_doubleSpinBoxPgt2_valueChanged(double);
  void on_doubleSpinBoxPgt3_valueChanged(double);
  void on_doubleSpinBoxTotal_valueChanged(const double &);
  void on_pushButtonCadastrarPedido_clicked();
  void on_pushButtonCancelamento_clicked();
  void on_pushButtonDevolucao_clicked();
  void on_pushButtonFreteLoja_clicked();
  void on_pushButtonGerarExcel_clicked();
  void on_pushButtonImprimir_clicked();
  void on_pushButtonLimparPag_clicked();
  void on_pushButtonPgtLoja_clicked();
  void on_pushButtonVoltar_clicked();
  void on_tableFluxoCaixa_entered(const QModelIndex &);

signals:
  void finished();

private:
  // attributes
  Ui::Venda *ui;
  bool isBlockedGlobal = false;
  bool isBlockedReais = false;
  bool isBlockedTotal = false;
  double minimoFrete;
  double porcFrete;
  QString m_idOrcamento;
  SqlTableModel modelFluxoCaixa;
  SqlTableModel modelItem;
  // methods
  virtual bool save() override;
  virtual bool savingProcedures() override;
  virtual bool verifyFields() override;
  virtual bool viewRegister() override;
  virtual void clearFields() override;
  virtual void registerMode() override;
  virtual void setupMapper() override;
  virtual void successMessage() override;
  virtual void updateMode() override;
  void calcPrecoGlobalTotal();
  void calculoSpinBox1();
  void calculoSpinBox2();
  bool generateId();
  void resetarPagamentos();
  void setupTables();
  bool cancelamento();
  bool cadastrar();
  bool atualizarCredito();
};

#endif // VENDA_H
