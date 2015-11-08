#ifndef CADASTRARUSUARIO_H
#define CADASTRARUSUARIO_H

#include "registerdialog.h"

namespace Ui {
  class CadastroUsuario;
}

class CadastroUsuario : public RegisterDialog {
    Q_OBJECT

  public:
    explicit CadastroUsuario(QWidget *parent = 0);
    ~CadastroUsuario();

  public slots:
    void show();

  private slots:
    void on_pushButtonAtualizar_clicked();
    void on_pushButtonCadastrar_clicked();
    void on_pushButtonCancelar_clicked();
    void on_pushButtonNovoCad_clicked();
    void on_pushButtonRemover_clicked();
    void on_pushButtonBuscar_clicked();
    void setupTablePermissoes();
    void fillCombobox();

    // methods derived from RegisterDialog

  private:
    /*!
*\brief Utilizada para selecionar um item a partir de um QModelIndex
*\param index Índice do Model relacionado ao item, normalmente obtido ao clicar na tabela
*\return
*/
    bool viewRegister(const QModelIndex index);

    /*!
*\brief Função padrão para verificar campos obrigatórios
*\return
*/
    virtual bool verifyFields();
    /*!
*\brief Onde ocorre o model.setData(), baseada nas informações da view.
*/
    virtual bool savingProcedures();
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

  private:
    // attributes
    Ui::CadastroUsuario *ui;
};

#endif // CADASTRARUSUARIO_H
