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
    void on_checkBoxRepresentacao_clicked(const bool &checked);
    void on_pushButtonCancelar_clicked();
    void on_pushButtonSalvar_clicked();
    void on_tableErro_entered(const QModelIndex &);
    void on_tableProdutos_entered(const QModelIndex &);
    void on_tabWidget_currentChanged(const int &index);

  private:
    // attributes
    Ui::ImportaProdutos *ui;
    bool hasError = false;
    int itensError = 0;
    int itensExpired = 0;
    int itensImported = 0;
    int itensNotChanged = 0;
    int itensUpdated = 0;
    int row = 0;
    int validade;
    QHash<int, bool> hashAtualizado;
    QHash<QString, int> hash;
    QMap<QString, int> fornecedores;
    QProgressDialog *progressDialog;
    QSqlDatabase db;
    QString file;
    QString fornecedor;
    QString ids;
    QVariantMap variantMap;
    SqlTableModel model;
    SqlTableModel modelErro;
    // methods
    bool camposForaDoPadrao();
    bool readFile();
    bool readValidade();
    bool verificaSeProdutoJaCadastrado();
    bool verificaTabela(const QSqlRecord &record);
    int buscarCadastrarFornecedor(const QString &fornecedor);
    void atualizaCamposProduto();
    void atualizaProduto();
    void cadastraFornecedores();
    void cadastraProduto();
    void consistenciaDados();
    void contaProdutos();
    void expiraPrecosAntigos();
    void guardaNovoPrecoValidade();
    void importarTabela();
    void insereEmErro();
    void insereEmOk();
    void leituraProduto(const QSqlQuery &query, const QSqlRecord &record);
    void marcaProdutoNaoDescontinuado();
    void marcaTodosProdutosDescontinuados();
    void mostraApenasEstesFornecedores();
    void pintarCamposForaDoPadrao(const int &row);
    void salvar();
    void setProgressDialog();
    void setupTables();
    void setVariantMap();
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
