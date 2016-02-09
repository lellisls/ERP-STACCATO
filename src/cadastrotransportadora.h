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

  private slots:
    void on_checkBoxMostrarInativos_clicked(const bool &checked);
    void on_lineEditCEP_textChanged(const QString &cep);
    void on_lineEditCNPJ_textEdited(const QString &text);
    void on_pushButtonAdicionarEnd_clicked();
    void on_pushButtonAtualizar_clicked();
    void on_pushButtonAtualizarEnd_clicked();
    void on_pushButtonBuscar_clicked();
    void on_pushButtonCadastrar_clicked();
    void on_pushButtonEndLimpar_clicked();
    void on_pushButtonNovoCad_clicked();
    void on_pushButtonRemover_clicked();
    void on_pushButtonRemoverEnd_clicked();
    void on_tableEndereco_clicked(const QModelIndex &index);

  private:
    // attributes
    Ui::CadastroTransportadora *ui;
    // methods
    bool cadastrarEndereco(const bool &isUpdate);
    virtual bool savingProcedures() override;
    virtual bool verifyFields() override;
    virtual bool viewRegister(const QModelIndex &index) override;
    virtual void clearFields() override;
    virtual void registerMode() override;
    virtual void setupMapper() override;
    virtual void successMessage() override;
    virtual void updateMode() override;
    void clearEndereco();
    void novoEndereco();
    void setupTables();
    void setupUi();
};

#endif // CADASTROTRANSPORTADORA_H
