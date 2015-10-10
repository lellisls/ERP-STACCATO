#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "sqltablemodel.h"

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setHostname(const QString &value);
    void setUsername(const QString &value);
    void setPassword(const QString &value);
    void setPort(const QString &value);
    bool dbConnect();
    bool TestCadastroClienteIncompleto();
    bool TestInitDB();
    bool TestCadastroClienteEndereco();
    bool TestCadastroClienteCompleto();
    void TestImportacao();

    bool getHomologacao() const;
    void setHomologacao(bool value);

  public slots:
    void showError(const QSqlError &err);
    void updateTables();

  private slots:
    void on_actionCadastrarCliente_triggered();
    void on_actionCadastrarFornecedor_triggered();
    void on_actionCadastrarProdutos_triggered();
    void on_actionCadastrarProfissional_triggered();
    void on_actionCadastrarUsuario_triggered();
    void on_actionCriarOrcamento_triggered();
    void on_actionGerenciar_Lojas_triggered();
    void on_actionGerenciar_Transportadoras_triggered();
    void on_actionImportaProdutos_triggered();
    void on_lineEditBuscaContasPagar_textChanged(const QString &text);
    void on_lineEditBuscaContasReceber_textChanged(const QString &text);
    void on_lineEditBuscaEntregas_textChanged(const QString &arg1);
    void on_lineEditBuscaNFe_textChanged(const QString &text);
    void on_lineEditBuscaOrcamentos_textChanged(const QString &text);
    void on_lineEditBuscaProdutosPend_textChanged(const QString &text);
    void on_lineEditBuscaVendas_textChanged(const QString &text);
    void on_pushButtonConfirmarCompra_clicked();
    void on_pushButtonCriarOrc_clicked();
    void on_pushButtonEntradaEstoque_clicked();
    void on_pushButtonGerarCompra_clicked();
    void on_pushButtonMarcarColetado_clicked();
    void on_pushButtonMarcarFaturado_clicked();
    void on_pushButtonMarcarRecebido_clicked();
    void on_radioButtonContaPagarLimpar_clicked();
    void on_radioButtonContaPagarPago_clicked();
    void on_radioButtonContaPagarPendente_clicked();
    void on_radioButtonContaReceberLimpar_clicked();
    void on_radioButtonContaReceberPendente_clicked();
    void on_radioButtonContaReceberRecebido_clicked();
    void on_radioButtonEntregaEnviado_clicked();
    void on_radioButtonEntregaLimpar_clicked();
    void on_radioButtonEntregaPendente_clicked();
    void on_radioButtonNFeAutorizado_clicked();
    void on_radioButtonNFeEnviado_clicked();
    void on_radioButtonNFeLimpar_clicked();
    void on_radioButtonOrcExpirado_clicked();
    void on_radioButtonOrcLimpar_clicked();
    void on_radioButtonOrcProprios_clicked();
    void on_radioButtonOrcValido_clicked();
    void on_radioButtonProdPendEmCompra_clicked();
    void on_radioButtonProdPendPend_clicked();
    void on_radioButtonProdPendTodos_clicked();
    void on_radioButtonVendAberto_clicked();
    void on_radioButtonVendFechado_clicked();
    void on_radioButtonVendLimpar_clicked();
    void on_tableContasPagar_activated(const QModelIndex &index);
    void on_tableContasReceber_activated(const QModelIndex &index);
    void on_tableEntregasCliente_activated(const QModelIndex &index);
    void on_tableEstoque_activated(const QModelIndex &index);
    void on_tableNfeSaida_activated(const QModelIndex &index);
    void on_tableOrcamentos_activated(const QModelIndex &index);
    void on_tableFornCompras_activated(const QModelIndex &index);
    void on_tableProdutosPend_activated(const QModelIndex &index);
    void on_tableVendas_activated(const QModelIndex &index);
    void on_pushButtonTesteFaturamento_clicked();
    void on_tableFornLogistica_activated(const QModelIndex &index);
    void on_pushButtonTesteEmail_clicked();
    void on_pushButtonComprar_clicked();
    void on_pushButtonTodosFornCompras_clicked();
    void on_pushButtonExibirXML_clicked();
    void on_tableNfeEntrada_activated(const QModelIndex &index);

  private:
    // attributes
    Ui::MainWindow *ui;
    SqlTableModel *modelOrcamento;
    SqlTableModel *modelVendas;
    SqlTableModel *modelCAPagar;
    SqlTableModel *modelCAReceber;
    SqlTableModel *modelEntregasCliente;
    SqlTableModel *modelNfeEntrada;
    SqlTableModel *modelNfeSaida;
    SqlTableModel *modelEstoque;
    SqlTableModel *modelColeta;
    SqlTableModel *modelReceb;
    SqlTableModel *modelItemPedidosPend;
    // TODO: transformar em views
    QSqlQueryModel *modelPedForn;
    QSqlQueryModel *modelPedForn2;
    QSqlQueryModel *modelProdPend;
    QSqlQueryModel *modelItemPedidosComp;
    QSqlQueryModel *modelFat;
    QString hostname;
    QString username;
    QString password;
    QString port;
    bool homologacao;
    // methods
    void setupTables();
    bool event(QEvent *e);
    void darkTheme();
    QString getHostname() const;
    QString getUsername() const;
    QString getPassword() const;
    QString getPort() const;
    void readSettings();
};

#endif // MAINWINDOW_H
