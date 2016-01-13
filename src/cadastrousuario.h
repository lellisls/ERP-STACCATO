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
    void modificarUsuario();

  public slots:
    void show();

  private slots:
    void fillCombobox();
    void on_lineEditUser_textEdited(const QString &text);
    void on_pushButtonAtualizar_clicked();
    void on_pushButtonBuscar_clicked();
    void on_pushButtonCadastrar_clicked();
    void on_pushButtonNovoCad_clicked();
    void on_pushButtonRemover_clicked();
    void setupTablePermissoes();

  private:
    // attributes
    Ui::CadastroUsuario *ui;
    // methods
    bool viewRegister(const QModelIndex &index) override;
    virtual bool save(const bool &isUpdate = false) override;
    virtual bool savingProcedures() override;
    virtual bool verifyFields() override;
    virtual void clearFields() override;
    virtual void registerMode() override;
    virtual void setupMapper() override;
    virtual void successMessage() override;
    virtual void updateMode() override;
};

#endif // CADASTRARUSUARIO_H
