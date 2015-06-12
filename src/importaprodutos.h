#ifndef IMPORTAPRODUTOS_H
#define IMPORTAPRODUTOS_H

#include <QDialog>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QSqlRelationalTableModel>

#include "importaprodutosproxy.h"
#include "editablesqlmodel.h"

namespace Ui {
  class ImportaProdutos;
}

class ImportaProdutos : public QDialog {
    Q_OBJECT

  public:
    explicit ImportaProdutos(QWidget *parent = 0);
    ~ImportaProdutos();
    bool consistenciaDados();
    bool readFile();
    bool verificaTabela();
    int buscarCadastrarFornecedor(QString fornecedor);
    void atualizaCamposProduto(QSqlQuery &produto, QString idProduto);
    void cadastraFornecedores(QSqlQuery &query);
    void cadastraProduto();
    void contaProdutos();
    void guardaNovoPrecoValidade(QSqlQuery &produto, QString idProduto);
    void importar();
    void leituraProduto(QSqlQuery &queryProd);
    void marcaProdutoNaoDescontinuado(QSqlQuery &produto, QString idProduto);
    void marcaTodosProdutosDescontinuados();
    void mostraApenasEstesFornecedores();
    bool readValidade();
    void setModelAndTable();
    void setProgressDialog();
    void verificaSeProdutoJaCadastrado(QSqlQuery &produto);
    void expiraPrecosAntigos(QSqlQuery produto, QString idProduto);
    void TestImportacao();

  private slots:
    void on_pushButtonCancelar_clicked();
    void on_pushButtonSalvar_clicked();

  private:
    Ui::ImportaProdutos *ui;
    EditableSqlModel model;
    QProgressDialog *progressDialog;
    QString file, ids;
    int validade;
    QMap<QString, int> fornecedores;
    QSqlDatabase db;
    QStringList fields = {"fornecedor", "descricao", "estoque", "un", "colecao", "m2cx", "pccx", "kgcx", "formComercial",
                          "codComercial", "codBarras", "ncm", "icms", "situacaoTributaria", "qtdPallet", "custo", "ipi",
                          "st", "precoVenda", "comissao", "observacoes", "origem", "descontinuado", "temLote", "ui"};
    QStringList values;
    ImportaProdutosProxy *proxyModel;

    void importarTabela();
};

#endif // IMPORTAPRODUTOS_H
