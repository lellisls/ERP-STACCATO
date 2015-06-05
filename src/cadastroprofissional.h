#ifndef CADASTROPROFISSIONAL_H
#define CADASTROPROFISSIONAL_H

#include "registerdialog.h"

namespace Ui {
  class CadastroProfissional;
}

class CadastroProfissional : public RegisterDialog {
    Q_OBJECT

  public:
    explicit CadastroProfissional(QWidget *parent = 0);
    ~CadastroProfissional();

  private slots:
    void on_pushButtonAtualizar_clicked();
    void on_pushButtonCadastrar_clicked();
    void on_pushButtonCancelar_clicked();
    void on_pushButtonNovoCad_clicked();
    void on_pushButtonRemover_clicked();

    // methods derived from RegisterDialog
    void on_pushButtonBuscar_clicked();

  private:
    /*!
*\brief Função padrão para verificar campos obrigatórios
*\return
*/
    virtual bool verifyFields(int row);
    /*!
* \brief Onde ocorre o model.setData(), baseada nas informações da view.
*/
    virtual bool savingProcedures(int row);
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
    Ui::CadastroProfissional *ui;

    // RegisterDialog interface
  public:
    bool viewRegister(QModelIndex idx);

  public slots:
    void changeItem(QVariant value, QString text);
    void show();
};

#endif // CADASTROPROFISSIONAL_H
