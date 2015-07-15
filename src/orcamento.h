#ifndef ORCAMENTO_H
#define ORCAMENTO_H

#include <QDataWidgetMapper>
#include <QDialog>
#include <QPrinter>
#include <QSqlRelationalTableModel>
#include <QStandardItemModel>
#include <QTableWidgetItem>

#include "registerdialog.h"
#include "searchdialog.h"

namespace Ui {
  class Orcamento;
}

class Orcamento : public RegisterDialog {
    Q_OBJECT

  public:
    explicit Orcamento(QWidget *parent = 0);
    ~Orcamento();
    void show();
    void testaOrcamento();

  private slots:
    void on_checkBoxFreteManual_clicked(const bool checked);
    void on_spinBoxCaixas_valueChanged(const int caixas);
    void on_doubleSpinBoxDesconto_valueChanged(const double);
    void on_doubleSpinBoxDescontoGlobal_valueChanged(const double);
    void on_doubleSpinBoxFinal_editingFinished();
    void on_doubleSpinBoxFrete_editingFinished();
    void on_doubleSpinBoxQte_valueChanged(const double);
    void on_doubleSpinBoxTotal_valueChanged(const double);
    void on_doubleSpinBoxPrecoTotal_editingFinished();
    void on_itemBoxCliente_textChanged(const QString &text);
    void on_itemBoxProduto_textChanged(const QString &text);
    void on_pushButtonAdicionarItem_clicked();
    void on_pushButtonApagarOrc_clicked();
    void on_pushButtonAtualizarItem_clicked();
    void on_pushButtonAtualizarOrcamento_clicked();
    void on_pushButtonCadastrarOrcamento_clicked();
    void on_pushButtonCancelar_clicked();
    void on_pushButtonCancelarItem_clicked();
    void on_pushButtonGerarVenda_clicked();
    void on_pushButtonImprimir_clicked();
    void on_pushButtonLimparSelecao_clicked();
    void on_pushButtonRemoverItem_clicked();
    void on_tableProdutos_clicked(const QModelIndex &index);
    void print(const QPrinter *printer);
    void on_pushButtonReplicar_clicked();
    void setValue(const int recNo, const QString paramName, QVariant &paramValue, const int reportPage);

  signals:
    void finished();

    // methods derived from RegisterDialog
  public:
    /*!
*\brief Utilizada para selecionar um item a partir de um QModelIndex
*\param index Índice do Model relacionado ao item, normalmente obtido ao clicar na tabela
*\return
*/
    virtual bool viewRegister(const QModelIndex index);

  private:
    /*!
* \brief Função padrão para verificar campos obrigatórios
* \return
*/
    virtual bool verifyFields(const int row);
    /*!
* \brief Onde ocorre o model.setData(), baseada nas informações da view.
*/
    virtual bool savingProcedures(const int row);
    /*!
*\brief Limpar os campos da tela
*/
    virtual void clearFields();
    /*!
*\brief Função onde os mapeamentos são configurados
*/
    virtual void setupMapper();
    /*!
*\brief Função chamada para atualizar a view, escondendo botão atualizar, por exemplo
*/
    virtual void registerMode();
    /*!
*\brief Função chamada para atualizar a view, escondendo botão cadastrarm, por exemplo
*/
    virtual void updateMode();
    /*!
*\brief newRegister
*\return
*/
    virtual bool newRegister();
    /*!
* \brief Mensagem de confirmação padrão para quando cadastro é realizado com sucesso.
*/
    void successMessage();

  private:
    // attributes
    Ui::Orcamento *ui;
    QSqlRelationalTableModel modelItem;
    QDataWidgetMapper mapperItem;
    double subTotal, subTotalItens;
    // methods
    void removeItem();
    void adicionarItem(const bool isUpdate = false);
    void calcPrecoGlobalTotal(const bool ajusteTotal = false);
    void calcPrecoItemTotal();
    void novoItem();
    void updateId();
    QString itemData(const int row, const QString key);
    void setupTables();
    bool verificaCampos();

    // RegisterDialog interface
  protected:
    bool save(const bool isUpdate = false);
};

#endif // ORCAMENTO_H
