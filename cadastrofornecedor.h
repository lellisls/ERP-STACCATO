#ifndef CADASTROFORNECEDOR_H
#define CADASTROFORNECEDOR_H

#include "registerdialog.h"

namespace Ui {
  class CadastroFornecedor;
}

class CadastroFornecedor : public RegisterDialog
{
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

  public:
    virtual bool viewRegister(QModelIndex idx);
    void clearEnd();

  private:
    bool atualizarEnd();
    void novoEnd();
    virtual bool verifyFields(int row);
    virtual bool savingProcedures(int row);
    virtual void clearFields();
    virtual void setupMapper();
    virtual void registerMode();
    virtual void updateMode();

  private:
    Ui::CadastroFornecedor *ui;
    bool closeBeforeUpdate;

    void validaCNPJ(QString text);
    void validaCPF(QString text);
    bool verifyRequiredField(QLineEdit *line, bool silent = false);

    QSqlTableModel modelEnd;
    QDataWidgetMapper mapperEnd;
};

#endif // CADASTROFORNECEDOR_H
