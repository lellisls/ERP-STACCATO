#ifndef IMPORTATESTE_H
#define IMPORTATESTE_H

#include <QDialog>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QSqlRelationalTableModel>

#include "importaexportproxy.h"
#include "editablesqlmodel.h"

namespace Ui {
  class ImportaTeste;
}

class ImportaTeste : public QDialog {
    Q_OBJECT

  public:
    explicit ImportaTeste(QWidget *parent = 0);
    ~ImportaTeste();
    int buscarCadastrarFornecedor(QString fornecedor);
    bool consistenciaDados();
    bool readFile();
    bool verificaTabela();
    void atualizaCamposProduto(QSqlQuery &produto, QString idProduto);
    void cadastraFornecedores(QSqlQuery &query);
    void cadastraProduto();
    void contaProdutos();
    void guardaNovoPrecoValidade(QSqlQuery &produto, QString idProduto);
    void importar();
    void leituraProduto(QSqlQuery &queryProd);
    void marcaProdutoNaoExpirado(QSqlQuery &produto, QString idProduto);
    void marcaTodosProdutosExpirado();
    void mostraApenasEstesFornecedores();
    void readValidade();
    void setModelAndTable();
    void setProgressDialog();
    void verificaSeProdutoJaCadastrado(QSqlQuery &produto);

  private slots:
    void on_pushButtonCancelar_clicked();
    void on_pushButtonSalvar_clicked();

  private:
    Ui::ImportaTeste *ui;
    EditableSqlModel model;
    QProgressDialog *progressDialog;
    QString file, ids;
    int validade;
    QMap<QString, int> fornecedores;
    QSqlDatabase db;
    QStringList fields = {"fornecedor", "descricao", "estoque", "un", "colecao", "m2cx", "pccx", "kgcx",
                          "formComercial", "codComercial", "codBarras", "ncm", "icms", "situacaoTributaria",
                          "qtdPallet", "custo", "ipi", "st", "precoVenda", "comissao", "observacoes", "origem",
                          "descontinuado", "temLote", "ui"};
    QStringList values;
    ImportaExportProxy *proxyModel;
};

#endif // IMPORTATESTE_H
