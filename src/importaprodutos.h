#ifndef IMPORTAPRODUTOS_H
#define IMPORTAPRODUTOS_H

#include <QDialog>
#include <QProgressDialog>

#include "sqltablemodel.h"

namespace Ui {
  class ImportaProdutos;
}

class ImportaProdutos : public QDialog {
    Q_OBJECT

  public:
    explicit ImportaProdutos(QWidget *parent = 0);
    ~ImportaProdutos();
    void importar();

  private slots:
    void on_pushButtonCancelar_clicked();
    void on_pushButtonSalvar_clicked();
    void on_checkBoxRepresentacao_clicked(const bool checked);
    void on_tableProdutos_entered(const QModelIndex &index);
    void on_tableErro_entered(const QModelIndex &index);
    void on_tabWidget_currentChanged(int index);

  private:
    // attributes
    Ui::ImportaProdutos *ui;
    SqlTableModel model, modelErro;
    QProgressDialog *progressDialog;
    QString file;
    QString ids;
    QString fornecedor;
    int validade;
    QMap<QString, int> fornecedores;
    QSqlDatabase db;
    QVariantMap variantMap;
    bool hasError = false;
    int itensImported = 0;
    int itensUpdated = 0;
    int itensNotChanged = 0;
    int itensExpired = 0;
    int itensError = 0;
    // methods
    void importarTabela();
    void consistenciaDados();
    bool readFile();
    bool readValidade();
    bool verificaTabela(const QSqlRecord &record);
    int buscarCadastrarFornecedor(const QString fornecedor);
    void atualizaCamposProduto(const QSqlQuery &produto, const QString idProduto);
    void cadastraFornecedores();
    void cadastraProduto();
    void contaProdutos();
    void expiraPrecosAntigos(QSqlQuery &produto, const QString idProduto);
    void guardaNovoPrecoValidade(QSqlQuery &produto, const QString idProduto);
    void leituraProduto(const QSqlQuery &query, const QSqlRecord &record);
    void marcaProdutoNaoDescontinuado(QSqlQuery &produto, const QString idProduto);
    void marcaTodosProdutosDescontinuados();
    void mostraApenasEstesFornecedores();
    void setupTables();
    void setProgressDialog();
    void verificaSeProdutoJaCadastradoNoBD(QSqlQuery &produto);
    void verificaSeProdutoJaCadastradoNoModel();
    void pintarCamposForaDoPadrao(const int row);
    void setVariantMap();
    void salvar();
    bool camposForaDoPadrao();
    void insereEmErro();
    void insereEmOk();
    void atualizaProduto(QSqlQuery produto);
    void verificaSeRepresentacao();

    enum FieldColors {
      White = 0,  // no change
      Green = 1,  // new value
      Yellow = 2, // value changed
      Gray = 3,   // wrong value but accepted
      Red = 4     // wrong value, must be fixed
    };
};

#endif // IMPORTAPRODUTOS_H
