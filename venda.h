#ifndef VENDA_H
#define VENDA_H

#include <QDialog>
#include <QLineEdit>
#include <QSqlRelationalTableModel>
#include <QSqlTableModel>

#include "registerdialog.h"

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
//    void on_doubleSpinBoxRestante_valueChanged(double value);
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

  signals:
    void finished();

  private:
    // attributes
    Ui::Venda *ui;
    QSqlTableModel modelVenda;
    QSqlRelationalTableModel modelItem, modelFluxoCaixa;
    QString idOrcamento;
    // methods
    bool cadastrar();
    bool verifyFields();
    bool verifyRequiredField(QLineEdit *line);
    QString requiredStyle();
    void calcPrecoGlobalTotal();
    void calcPrecoItemTotal();
    void clearFields();
    void fillComboBoxCliente();
    void setupMapper();
    void updateId();

    // RegisterDialog interface
  protected:
    virtual bool savingProcedures(int row);
    virtual void registerMode();
    virtual void updateMode();

    // RegisterDialog interface
  public:
    virtual bool viewRegister(QModelIndex index);
};

#endif // VENDA_H
