#ifndef CADASTROTRANSPORTADORA_H
#define CADASTROTRANSPORTADORA_H

#include "registeraddressdialog.h"

namespace Ui {
  class CadastroTransportadora;
}

class CadastroTransportadora : public RegisterAddressDialog {
    Q_OBJECT

  public:
    explicit CadastroTransportadora(QWidget *parent = 0);
    ~CadastroTransportadora();

  public slots:
    void show();

  private slots:
    void on_pushButtonAtualizar_clicked();
    void on_pushButtonCadastrar_clicked();
    void on_pushButtonCancelar_clicked();
    void on_pushButtonNovoCad_clicked();
    void on_pushButtonRemover_clicked();
    void on_pushButtonBuscar_clicked();
    void on_lineEditCNPJ_textEdited(const QString &text);
    void on_pushButtonAdicionarEnd_clicked();
    void on_pushButtonAtualizarEnd_clicked();
    void on_pushButtonEndLimpar_clicked();
    void on_pushButtonRemoverEnd_clicked();
    void on_checkBoxMostrarInativos_clicked(const bool checked);
    void on_lineEditCEP_textChanged(const QString &cep);
    void on_tableEndereco_clicked(const QModelIndex &index);

  private:
    /*!
*\brief Função padrão para verificar campos obrigatórios
*\return
*/
    virtual bool verifyFields();
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

  private:
    // attributes
    Ui::CadastroTransportadora *ui;
    // methods
    virtual bool newRegister();
    void novoItem();
    bool cadastrarEndereco(const bool isUpdate);
    void novoEndereco();
    void setupTables();
    void clearEndereco();
    void setupUi();
    bool viewRegister(const QModelIndex index);
};

#endif // CADASTROTRANSPORTADORA_H
