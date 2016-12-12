#ifndef ORCAMENTO_H
#define ORCAMENTO_H

#include "registerdialog.h"
#include "searchdialogproxy.h"

namespace Ui {
class Orcamento;
}

class Orcamento : public RegisterDialog {
  Q_OBJECT

public:
  explicit Orcamento(QWidget *parent = 0);
  ~Orcamento();
  void show();

private slots:
  void on_checkBoxFreteManual_clicked(const bool checked);
  void on_checkBoxRepresentacao_toggled(const bool checked);
  void on_comboBoxLoja_currentTextChanged(const QString &);
  void on_doubleSpinBoxCaixas_valueChanged(const double caixas);
  void on_doubleSpinBoxDesconto_valueChanged(const double);
  void on_doubleSpinBoxDescontoGlobal_valueChanged(const double);
  void on_doubleSpinBoxDescontoGlobalReais_valueChanged(const double);
  void on_doubleSpinBoxFrete_valueChanged(const double);
  void on_doubleSpinBoxQuant_editingFinished();
  void on_doubleSpinBoxQuant_valueChanged(const double);
  void on_doubleSpinBoxSubTotalBruto_valueChanged(const double);
  void on_doubleSpinBoxSubTotalLiq_valueChanged(const double);
  void on_doubleSpinBoxTotal_valueChanged(const double);
  void on_doubleSpinBoxTotalItem_valueChanged(const double);
  void on_itemBoxCliente_textChanged(const QString &);
  void on_itemBoxProduto_textChanged(const QString &);
  void on_itemBoxVendedor_textChanged(const QString &);
  void on_pushButtonAdicionarItem_clicked();
  void on_pushButtonApagarOrc_clicked();
  void on_pushButtonAtualizarItem_clicked();
  void on_pushButtonAtualizarOrcamento_clicked();
  void on_pushButtonCadastrarOrcamento_clicked();
  void on_pushButtonGerarExcel_clicked();
  void on_pushButtonGerarVenda_clicked();
  void on_pushButtonImprimir_clicked();
  void on_pushButtonLimparSelecao_clicked();
  void on_pushButtonRemoverItem_clicked();
  void on_pushButtonReplicar_clicked();
  void on_tableProdutos_clicked(const QModelIndex &index);

signals:
  void finished();

private:
  // attributes
  bool isBlockedDesconto = false;
  bool isBlockedGlobal = false;
  bool isBlockedReais = false;
  bool isBlockedTotal = false;
  bool isBlockedTotalItem = false;
  bool isReadOnly = false;
  double minimoFrete = 0;
  double porcFrete = 0;
  QDataWidgetMapper mapperItem;
  SearchDialogProxy *proxy;
  SqlTableModel modelItem;
  Ui::Orcamento *ui;
  // methods
  bool atualizaReplica();
  bool cadastrar() override;
  bool calcPrecoGlobalTotal();
  bool generateId();
  bool verificaCadastroCliente();
  virtual bool newRegister() override;
  virtual bool save() override;
  virtual bool savingProcedures() override;
  virtual bool verifyFields() override;
  virtual bool viewRegister() override;
  virtual void clearFields() override;
  virtual void registerMode() override;
  virtual void setupMapper() override;
  virtual void successMessage() override;
  virtual void updateMode() override;
  void adicionarItem(const bool isUpdate = false);
  void atualizarItem();
  void calcPrecoItemTotal();
  void novoItem();
  void removeItem();
  void setupTables();
};

#endif // ORCAMENTO_H
