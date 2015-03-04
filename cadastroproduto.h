#ifndef CADASTROPRODUTO_H
#define CADASTROPRODUTO_H

#include <QDialog>

#include "registerdialog.h"

namespace Ui {
  class CadastroProduto;
}

class CadastroProduto : public RegisterDialog  {
  Q_OBJECT

public:
  explicit CadastroProduto(QWidget *parent = 0);
  ~CadastroProduto();

private slots:
  void changeItem(QVariant value, QString text);
  void on_pushButtonAtualizar_clicked();
  void on_pushButtonCadastrar_clicked();
  void on_pushButtonCancelar_clicked();
  void on_pushButtonNovoCad_clicked();
  void on_pushButtonRemover_clicked();
  //  void updateComboboxFornecedor();

  // methods derived from RegisterDialog
  void on_pushButtonBuscar_clicked();

private:
  /*!
  * \brief Função padrão para verificar campos obrigatórios
  * \return
  */
  virtual bool verifyFields(int row);
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


private:
  //attributes
  Ui::CadastroProduto *ui;
  int ProdutosRow = 0;
};

#endif // CADASTROPRODUTO_H
