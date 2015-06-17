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
    bool viewRegister(QModelIndex index);

  public slots:
    void changeItem(QVariant value);
    void show();

  private slots:
    void on_pushButtonAtualizar_clicked();
    void on_pushButtonCadastrar_clicked();
    void on_pushButtonCancelar_clicked();
    void on_pushButtonNovoCad_clicked();
    void on_pushButtonRemover_clicked();
    void on_pushButtonBuscar_clicked();
    void on_lineEditCPF_textEdited(const QString &text);
    void on_lineEditCNPJ_textEdited(const QString &text);
    void on_pushButtonAdicionarEnd_clicked();
    void on_pushButtonAtualizarEnd_clicked();
    void on_lineEditCEP_textChanged(const QString &cep);
    void on_pushButtonEndLimpar_clicked();
    void on_tableEndereco_clicked(const QModelIndex &index);
    void on_pushButtonRemoverEnd_clicked();
    void on_lineEditContatoCPF_textEdited(const QString &text);
    void on_checkBoxMostrarInativos_clicked(bool checked);
    void on_radioButtonPF_toggled(bool checked);
    void on_lineEditCPFBancario_textEdited(const QString &text);

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
    QString tipoPFPJ;
    QSqlTableModel modelEnd;
    QDataWidgetMapper mapperEnd;
    // methods
    void setupUi();
    bool verifyRequiredField(QLineEdit *line, bool silent);
    void novoEnd();
    void clearEnd();
    bool cadastrarEndereco(bool isUpdate);
};

#endif // CADASTROPROFISSIONAL_H
