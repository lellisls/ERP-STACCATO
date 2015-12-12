#ifndef VENDA_H
#define VENDA_H

#include <QSqlQuery>

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
    void on_checkBoxFreteManual_clicked(const bool &checked);
    void on_comboBoxPgt1_currentTextChanged(const QString &text);
    void on_comboBoxPgt1Parc_currentTextChanged(const QString &);
    void on_comboBoxPgt2_currentTextChanged(const QString &text);
    void on_comboBoxPgt2Parc_currentTextChanged(const QString &);
    void on_comboBoxPgt3_currentTextChanged(const QString &text);
    void on_comboBoxPgt3Parc_currentTextChanged(const QString &);
    void on_dateEditPgt1_dateChanged(const QDate &date);
    void on_dateEditPgt2_dateChanged(const QDate &date);
    void on_dateEditPgt3_dateChanged(const QDate &date);
    void on_doubleSpinBoxDescontoGlobal_valueChanged(const double &);
    void on_doubleSpinBoxDescontoGlobalReais_valueChanged(const double &);
    void on_doubleSpinBoxFrete_valueChanged(const double &);
    void on_doubleSpinBoxPgt1_editingFinished();
    void on_doubleSpinBoxPgt2_editingFinished();
    void on_doubleSpinBoxPgt3_editingFinished();
    void on_doubleSpinBoxTotal_valueChanged(const double &);
    void on_lineEditPgt1_textChanged(const QString &);
    void on_lineEditPgt2_textChanged(const QString &);
    void on_lineEditPgt3_textChanged(const QString &);
    void on_pushButtonCadastrarPedido_clicked();
    void on_pushButtonCancelar_clicked();
    void on_pushButtonGerarExcel_clicked();
    void on_pushButtonImprimir_clicked();
    void on_pushButtonLimparPag_clicked();
    void on_pushButtonVoltar_clicked();
    void setValue(const int &recNo, const QString &paramName, QVariant &paramValue, const int &reportPage);

  signals:
    void finished();

  private:
    // attributes
    Ui::Venda *ui;
    double minimoFrete;
    double porcFrete;
    QSqlQuery queryCliente;
    QSqlQuery queryEndEnt;
    QSqlQuery queryEndFat;
    QSqlQuery queryLoja;
    QSqlQuery queryLojaEnd;
    QSqlQuery queryProduto;
    QSqlQuery queryProfissional;
    QSqlQuery queryVenda;
    QSqlQuery queryVendedor;
    SqlTableModel modelFluxoCaixa;
    SqlTableModel modelItem;
    // methods
    bool verifyFields();
    QString requiredStyle() const;
    QVariant settings(const QString &key) const;
    virtual bool savingProcedures();
    virtual bool viewRegister(const QModelIndex &index);
    virtual void registerMode();
    virtual void successMessage();
    virtual void updateMode();
    void calcPrecoGlobalTotal();
    void calculoSpinBox1() const;
    void calculoSpinBox2() const;
    void clearFields();
    void fillTotals();
    void montarFluxoCaixa();
    void resetarPagamentos();
    void setPrintExcelQuerys();
    void setSettings(const QString &key, const QVariant &value) const;
    void setupMapper();
    void setupTables();
};

#endif // VENDA_H
