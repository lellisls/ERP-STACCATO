#ifndef IMPORTAPRODUTOS_H
#define IMPORTAPRODUTOS_H

#include <QDialog>
#include <QProgressDialog>
#include <QSqlRelationalTableModel>

#include "editablesqlmodel.h"

namespace Ui {
  class ImportaProdutos;
}

class ImportaProdutos : public QDialog {
    Q_OBJECT

  public:
    explicit ImportaProdutos(QWidget *parent = 0);
    ~ImportaProdutos();
    void consistenciaDados();
    bool readFile();
    bool readValidade();
    bool verificaTabela(const QSqlRecord &record);
    int buscarCadastrarFornecedor(const QString fornecedor);
    void atualizaCamposProduto(const QSqlQuery &produto, const QString idProduto);
    void cadastraFornecedores(QSqlQuery &query);
    void cadastraProduto();
    void contaProdutos();
    void expiraPrecosAntigos(QSqlQuery &produto, const QString idProduto);
    void guardaNovoPrecoValidade(QSqlQuery &produto, const QString idProduto);
    void importar();
    void leituraProduto(const QSqlQuery &query, const QSqlRecord &record);
    void marcaProdutoNaoDescontinuado(QSqlQuery &produto, const QString idProduto);
    void marcaTodosProdutosDescontinuados();
    void mostraApenasEstesFornecedores();
    void setModelAndTable();
    void setProgressDialog();
    void TestImportacao();
    void verificaSeProdutoJaCadastrado(QSqlQuery &produto);
    void pintarCamposForaDoPadrao(const int row);
    void setVariantMap();
    void salvar();

  private slots:
    void on_pushButtonCancelar_clicked();
    void on_pushButtonSalvar_clicked();
    void on_checkBoxRepresentacao_clicked(const bool checked);

  private:
    // attributes
    Ui::ImportaProdutos *ui;
    EditableSqlModel model;
    QProgressDialog *progressDialog;
    QString file, ids;
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

    enum FieldColors {
      White = 0,  // no change
      Green = 1,  // new value
      Yellow = 2, // value changed
      Gray = 3,   // wrong value but accepted
      Red = 4     // wrong value, must be fixed
    };
};

#endif // IMPORTAPRODUTOS_H
