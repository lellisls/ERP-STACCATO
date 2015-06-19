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
    void changeItem(QVariant value);
    void on_pushButtonAtualizar_clicked();
    void on_pushButtonCadastrar_clicked();
    void on_pushButtonCancelar_clicked();
    void on_pushButtonEntradaNFe_clicked();
    void on_pushButtonNovoCad_clicked();
    void on_pushButtonRemover_clicked();
    void on_pushButtonSaidaNFe_clicked();
    void on_pushButtonBuscar_clicked();
    void on_lineEditCNPJ_textEdited(const QString &text);
    void on_pushButtonAdicionarEnd_clicked();
    void on_pushButtonAtualizarEnd_clicked();
    void on_pushButtonEndLimpar_clicked();
    void on_pushButtonRemoverEnd_clicked();
    void on_checkBoxMostrarInativos_clicked(bool checked);
    void on_lineEditCEP_textChanged(const QString &cep);
    void on_tableEndereco_clicked(const QModelIndex &index);

  public:
    /*!
*\brief Utilizada para selecionar um item a partir de um QModelIndex
*\param index Índice do Model relacionado ao item, normalmente obtido ao clicar na tabela
*\return
*/
    virtual bool viewRegister(QModelIndex index);
    void clearEndereco();

  private:
    /*!
*\brief Função padrão para verificar campos obrigatórios
*\return
*/
    virtual bool verifyFields(int row);
    /*!
*\brief Onde ocorre o model.setData(), baseada nas informações da view.
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
    Ui::CadastroLoja *ui;
    QSqlRelationalTableModel modelAlcadas;
    QSqlTableModel modelEnd;
    QDataWidgetMapper mapperEnd;
    // methods
    void novoEndereco();
    bool cadastrarEndereco(bool isUpdate);
  public slots:
    void show();
    int getCodigoUF();
};

#endif // CADASTROLOJA_H
