#ifndef CADASTROFORNECEDOR_H
#define CADASTROFORNECEDOR_H

#include "registerdialog.h"

namespace Ui {
  class CadastroFornecedor;
}

class CadastroFornecedor : public RegisterDialog {
    Q_OBJECT

  public:
    explicit CadastroFornecedor(bool closeBeforeUpdate = false, QWidget *parent = 0);
    ~CadastroFornecedor();
    void setupUi();

  public slots:
    void enableEditor();
    void disableEditor();
    void show();

  private slots:
    void on_pushButtonCadastrar_clicked();
    void on_pushButtonAtualizar_clicked();
    void on_pushButtonBuscar_clicked();
    void on_pushButtonNovoCad_clicked();
    void on_pushButtonRemover_clicked();
    void on_pushButtonCancelar_clicked();
    void on_lineEditCNPJ_textEdited(const QString &);
    void on_lineEditContatoCPF_textEdited(const QString &);

  public:
    virtual bool viewRegister(QModelIndex idx);
    void clearEnd();

  private:
    // attributes
    Ui::CadastroFornecedor *ui;
    bool closeBeforeUpdate;
    QSqlTableModel modelEnd;
    QDataWidgetMapper mapperEnd;
    // methods
    void validaCNPJ(QString text);
    void validaCPF(QString text);
    bool verifyRequiredField(QLineEdit *line, bool silent = false);
    bool atualizarEnd();
    void novoEnd();
    virtual bool verifyFields(int row);
    virtual bool savingProcedures(int row);
    virtual void clearFields();
    virtual void setupMapper();
    virtual void registerMode();
    virtual void updateMode();
};

#endif // CADASTROFORNECEDOR_H
