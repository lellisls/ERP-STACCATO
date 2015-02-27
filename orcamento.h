#ifndef ORCAMENTO_H
#define ORCAMENTO_H

#include <QDataWidgetMapper>
#include <QDialog>
#include <QPrinter>
#include <QSqlRelationalTableModel>
#include <QStandardItemModel>
#include <QTableWidgetItem>

#include "registerdialog.h"

namespace Ui {
  class Orcamento;
}

class Orcamento : public RegisterDialog {
  Q_OBJECT

public:
  explicit Orcamento(QWidget *parent = 0);
  ~Orcamento();

private slots:
  void on_checkBoxCalculaFrete_clicked();
  //    void on_comboBoxCliente_currentIndexChanged(int);
  //    void on_comboBoxProduto_currentIndexChanged(int);
  void on_comboBoxVendedor_currentIndexChanged(int);
  void on_doubleSpinBoxDesconto_valueChanged(double);
  void on_doubleSpinBoxDescontoGlobal_valueChanged(double);
  void on_doubleSpinBoxFrete_editingFinished();
  void on_doubleSpinBoxQte_editingFinished();
  void on_doubleSpinBoxQte_valueChanged(double);
  void on_doubleSpinBoxTotal_valueChanged(double);
  void on_doubleSpinBoxFinal_editingFinished();
  void on_pushButtonAdicionarItem_clicked();
  void on_pushButtonAtualizarItem_clicked();
  void on_pushButtonAtualizarOrcamento_clicked();
  //    void on_pushButtonBuscar_clicked();
  //  void on_pushButtonCadastrarCliente_clicked();
  void on_pushButtonCadastrarOrcamento_clicked();
  void on_pushButtonCancelar_clicked();
  void on_pushButtonCancelarItem_clicked();
  void on_pushButtonFecharPedido_clicked();
  void on_pushButtonImprimir_clicked();
  void on_pushButtonRemoverItem_clicked();
  void on_tableProdutos_clicked(const QModelIndex &index);
  void print(QPrinter *printer);
  void on_doubleSpinBoxCaixas_valueChanged(double arg1);
  //    void on_pushButtonBuscarCliente_clicked();

  void on_pushButtonApagarOrc_clicked();

  void on_itemBoxProduto_textChanged(const QString &text);

  void on_itemBoxCliente_textChanged(const QString &text);

signals:
  void finished();

  // methods derived from RegisterDialog
public:
  /*!
   * \brief Utilizada para selecionar um item a partir de um QModelIndex
   * \param idx Índice do Model relacionado ao item, normalmente obtido ao clicar na tabela
   * \return
   */
  virtual bool viewRegister(QModelIndex idx);

private:

  /*!
  * \brief Função padrão para verificar campos obrigatórios
  * \return
  */
  virtual bool verifyFields();
  /*!
  * \brief Onde ocorre o model.setData(), baseada nas informações da view.
  */
  virtual bool savingProcedures(int row);
  /*!
   * \brief Limpar os campos da tela
   */
  virtual void clearFields();
  /*!
   * \brief Função onde os mapeamentos são configurados
   */
  virtual void setupMapper();
  /*!
   * \brief Função chamada para atualizar a view, escondendo botão atualizar, por exemplo
   */
  virtual void registerMode();
  /*!
   * \brief Função chamada para atualizar a view, escondendo botão cadastrarm, por exemplo
   */
  virtual void updateMode();
  /*!
   * \brief newRegister
   * \return
   */
  virtual bool newRegister();
  /*!
  * \brief Mensagem de confirmação padrão para quando cadastro é realizado com sucesso.
  */
  void successMessage();

private:
  // attributes
  QString getItensHtml();
  Ui::Orcamento *ui;
  QSqlRelationalTableModel modelItem;
  QDataWidgetMapper mapperItem;
  double subTotal, totalGlobal, subTotalItens;
  int currentRow;
  double minimoFrete, porcFrete;

  void removeItem();
  void fillComboBoxes();
  void adicionarItem();
  void atualizarItem();
  void calcPrecoGlobalTotal(bool ajusteTotal = false);
  void calcPrecoItemTotal();
  void novoItem();
  void updateId();
};

#endif // ORCAMENTO_H
