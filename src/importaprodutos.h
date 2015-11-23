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
    void on_checkBoxRepresentacao_clicked(const bool &checked);
    void on_tableProdutos_entered(const QModelIndex &);
    void on_tableErro_entered(const QModelIndex &);
    void on_tabWidget_currentChanged(const int &index);

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
    QHash<QString, int> hash;
    int row = 0;
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
    int buscarCadastrarFornecedor(const QString &fornecedor);
    void atualizaCamposProduto();
    void cadastraFornecedores();
    void cadastraProduto();
    void contaProdutos();
    void expiraPrecosAntigos();
    void guardaNovoPrecoValidade();
    void leituraProduto(const QSqlQuery &query, const QSqlRecord &record);
    void marcaProdutoNaoDescontinuado();
    void marcaTodosProdutosDescontinuados();
    void mostraApenasEstesFornecedores();
    void setupTables();
    void setProgressDialog();
    bool verificaSeProdutoJaCadastrado();
    void verificaSeProdutoJaCadastradoNoModel();
    void pintarCamposForaDoPadrao(const int &row);
    void setVariantMap();
    void salvar();
    bool camposForaDoPadrao();
    void insereEmErro();
    void insereEmOk();
    void atualizaProduto();
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
