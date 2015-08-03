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
    virtual bool viewRegister(const QModelIndex index);
    void updateValues();

  private slots:
    void on_checkBoxFreteManual_clicked(const bool checked);
    void on_comboBoxPgt1_currentTextChanged(const QString &text);
    void on_comboBoxPgt1Parc_currentTextChanged(const QString &text);
    void on_comboBoxPgt2_currentTextChanged(const QString &text);
    void on_comboBoxPgt2Parc_currentTextChanged(const QString &text);
    void on_comboBoxPgt3_currentTextChanged(const QString &text);
    void on_comboBoxPgt3Parc_currentTextChanged(const QString &text);
    void on_dateEditPgt1_dateChanged(const QDate &date);
    void on_dateEditPgt2_dateChanged(const QDate &date);
    void on_dateEditPgt3_dateChanged(const QDate &date);
    void on_doubleSpinBoxDescontoGlobal_valueChanged(const double);
    void on_doubleSpinBoxTotal_editingFinished();
    void on_doubleSpinBoxFrete_editingFinished();
    void on_doubleSpinBoxPgt1_editingFinished();
    void on_doubleSpinBoxPgt2_editingFinished();
    void on_doubleSpinBoxPgt3_editingFinished();
    void on_pushButtonLimparPag_clicked();
    void on_pushButtonCancelar_clicked();
    void on_pushButtonCadastrarPedido_clicked();
    void on_pushButtonNFe_clicked();
    void on_pushButtonVoltar_clicked();
    void on_pushButtonImprimir_clicked();
    void setValue(const int recNo, const QString paramName, QVariant &paramValue, const int reportPage);

  signals:
    void finished();

  protected:
    virtual bool savingProcedures(int row);
    virtual void registerMode();
    virtual void updateMode();
    virtual void successMessage();

  private:
    // attributes
    Ui::Venda *ui;
    QSqlTableModel modelItem, modelFluxoCaixa;
    // methods
    bool verifyFields();
    bool verifyRequiredField(QLineEdit *line);
    QString requiredStyle();
    void calcPrecoGlobalTotal(const bool ajusteTotal = false);
    void calculoSpinBox1();
    void calculoSpinBox2();
    void clearFields();
    void fillComboBoxCliente();
    void fillTotals();
    void montarFluxoCaixa();
    void resetarPagamentos();
    void setupMapper();
    void setupTables();
};

#endif // VENDA_H
