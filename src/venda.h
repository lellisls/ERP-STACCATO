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
    void fecharOrcamento(const QString &idVenda);

  private slots:
    void montarFluxoCaixa();
    void on_checkBoxFreteManual_clicked(const bool &checked);
    void on_comboBoxPgt1_currentTextChanged(const QString &text);
    void on_comboBoxPgt2_currentTextChanged(const QString &text);
    void on_comboBoxPgt3_currentTextChanged(const QString &text);
    void on_doubleSpinBoxDescontoGlobal_valueChanged(const double &);
    void on_doubleSpinBoxDescontoGlobalReais_valueChanged(const double &);
    void on_doubleSpinBoxFrete_valueChanged(const double &);
    void on_doubleSpinBoxPgt1_editingFinished();
    void on_doubleSpinBoxPgt2_editingFinished();
    void on_doubleSpinBoxPgt3_editingFinished();
    void on_doubleSpinBoxTotal_valueChanged(const double &);
    void on_pushButtonCadastrarPedido_clicked();
    void on_pushButtonGerarExcel_clicked();
    void on_pushButtonImprimir_clicked();
    void on_pushButtonLimparPag_clicked();
    void on_pushButtonVoltar_clicked();

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
    QString id;
    SqlTableModel modelFluxoCaixa;
    SqlTableModel modelItem;
    // methods
    virtual bool save(const bool &isUpdate) override;
    virtual bool savingProcedures() override;
    virtual bool verifyFields() override;
    virtual bool viewRegister(const QModelIndex &index) override;
    virtual void clearFields() override;
    virtual void registerMode() override;
    virtual void setupMapper() override;
    virtual void successMessage() override;
    virtual void updateMode() override;
    void calcPrecoGlobalTotal();
    void calculoSpinBox1() const;
    void calculoSpinBox2() const;
    void fillTotals();
    void resetarPagamentos();
    void setupTables();
};

#endif // VENDA_H
