#ifndef CADASTROLOJA_H
#define CADASTROLOJA_H

#include "registerdialog.h"

namespace Ui {
  class CadastroLoja;
}

class CadastroLoja : public RegisterDialog {
  Q_OBJECT

public:
  explicit CadastroLoja(QWidget *parent = 0);
  ~CadastroLoja();

private slots:
  void on_pushButtonAtualizar_clicked();
  void on_pushButtonCadastrar_clicked();
  void on_pushButtonCancelar_clicked();
  void on_pushButtonNovoCad_clicked();
  void on_pushButtonRemover_clicked();

  // methods derived from RegisterDialog
  void on_pushButtonBuscar_clicked();
  void on_lineEditCNPJ_textEdited(const QString &);
  void changeItem(QVariant value, QString text);

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

private:
  // attributes
  Ui::CadastroLoja *ui;
  QSqlRelationalTableModel modelAlcadas;
  void validaCNPJ(QString text);
};

#endif // CADASTROLOJA_H
