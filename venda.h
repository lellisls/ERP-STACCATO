#ifndef VENDA_H
#define VENDA_H

#include <QDialog>
#include <QLineEdit>
#include <QSqlRelationalTableModel>
#include <QSqlTableModel>

#include "registerdialog.h"
#include "searchdialog.h"

namespace Ui {
  class Venda;
}

class Venda : public RegisterDialog {
    Q_OBJECT

  public:
    explicit Venda(QWidget *parent = 0);
    ~Venda();
    void fecharOrcamento(const QString &idOrcamento);
    void viewVenda(QString idVenda);
    void updateValues();

  private slots:
    void on_doubleSpinBoxPgt1_valueChanged(double);
    void on_doubleSpinBoxPgt2_valueChanged(double);
    void on_doubleSpinBoxPgt3_valueChanged(double);
    void on_pushButtonCancelar_clicked();
    void on_pushButtonFecharPedido_clicked();
    void on_pushButtonNFe_clicked();
    void on_doubleSpinBoxPgt1_editingFinished();
    void on_doubleSpinBoxPgt2_editingFinished();
    void on_doubleSpinBoxPgt3_editingFinished();
    void on_comboBoxPgt1_currentTextChanged(const QString &text);
    void on_comboBoxPgt2_currentTextChanged(const QString &text);
    void on_comboBoxPgt3_currentTextChanged(const QString &text);
    void on_pushButtonVoltar_clicked();
    void on_comboBoxPgt1Parc_currentTextChanged(const QString &text);
    void on_comboBoxPgt2Parc_currentTextChanged(const QString &text);
    void on_comboBoxPgt3Parc_currentTextChanged(const QString &text);
    void on_dateEditPgt1_dateChanged(const QDate &date);
    void on_dateEditPgt2_dateChanged(const QDate &date);
    void on_dateEditPgt3_dateChanged(const QDate &date);
    void on_doubleSpinBoxFinal_editingFinished();
    void on_doubleSpinBoxDescontoGlobal_valueChanged(double);

    void on_pushButton_clicked();

    void on_checkBoxFreteManual_clicked(bool checked);

    void on_doubleSpinBoxFrete_editingFinished();

  signals:
    void finished();

  private:
    // attributes
    Ui::Venda *ui;
    QSqlTableModel modelVenda;
    QSqlRelationalTableModel modelItem, modelFluxoCaixa;
    QString idOrcamento;
    double subTotal;
    SearchDialog *sdEndereco;
    // methods
    bool cadastrar();
    bool verifyFields(int row);
    bool verifyRequiredField(QLineEdit *line);
    QString requiredStyle();
    void calcPrecoGlobalTotal(bool ajusteTotal = false);
    void fillTotals();
//    void calcPrecoItemTotal();
    void clearFields();
    void fillComboBoxCliente();
    void setupMapper();
    void updateId();
    void montarFluxoCaixa();

    // RegisterDialog interface
  public:
    virtual bool viewRegister(QModelIndex index);

    void calculoSpinBox1();
    void calculoSpinBox2();
    void calculoSpinBox3();

    void resetarPagamentos();
  protected:
    virtual bool savingProcedures(int row);
    virtual void registerMode();
    virtual void updateMode();
};

#endif // VENDA_H
