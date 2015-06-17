#ifndef CADASTROFORNECEDOR_H
#define CADASTROFORNECEDOR_H

#include "registerdialog.h"

namespace Ui {
  class CadastroFornecedor;
}

class CadastroFornecedor : public RegisterDialog {
    Q_OBJECT

  public:
    explicit CadastroFornecedor(QWidget *parent = 0);
    ~CadastroFornecedor();
    void setupUi();

  public slots:
    void enableEditor();
    void disableEditor();
    void show();

  private slots:
    void on_lineEditCEP_textChanged(const QString &cep);
    void on_lineEditCNPJ_textEdited(const QString &text);
    void on_lineEditContatoCPF_textEdited(const QString &text);
    void on_pushButtonAdicionarEnd_clicked();
    void on_pushButtonAtualizar_clicked();
    void on_pushButtonBuscar_clicked();
    void on_pushButtonCadastrar_clicked();
    void on_pushButtonCancelar_clicked();
    void on_pushButtonNovoCad_clicked();
    void on_pushButtonRemover_clicked();
    void on_pushButtonAtualizarEnd_clicked();

  public:
    virtual bool viewRegister(QModelIndex index);
    void clearEnd();

  private:
    // attributes
    Ui::CadastroFornecedor *ui;
    QSqlTableModel modelEnd;
    QDataWidgetMapper mapperEnd;
    // methods
    bool verifyRequiredField(QLineEdit *line, bool silent = false);
    bool atualizarEndereco();
    void novoEnd();
    virtual bool verifyFields(int row);
    virtual bool savingProcedures(int row);
    virtual void clearFields();
    virtual void setupMapper();
    virtual void registerMode();
    virtual void updateMode();
    bool adicionarEndereco();
};

#endif // CADASTROFORNECEDOR_H
