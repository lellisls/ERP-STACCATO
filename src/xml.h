#ifndef XML_H
#define XML_H

#include <QDialog>
#include <QStandardItemModel>
#include <QDomElement>
#include <QSqlTableModel>

namespace Ui {
  class XML;
}

class XML : public QDialog
{
    Q_OBJECT

  public:
    explicit XML(QWidget *parent = 0);
    ~XML();
    void importarXML();
    void exibirXML(QString file);

  private:
    // attributes
    Ui::XML *ui;
    QString fileName;
    QStandardItemModel model;
    QSqlTableModel modelProduto;

    QString idNFe;
    //produto
    int idProduto = 0;
    int itemNumero = 0;
    QString codProd;
    QString codBarras;
    QString descricao;
    QString ncm;
    QString cfop;
    QString un;
    double quant = 0;
    double valorUnid = 0;
    double valor = 0;
    QString codBarrasTrib;
    QString unTrib;
    double quantTrib = 0;
    double valorTrib = 0;
    double desconto = 0;
    bool compoeTotal;
    QString numeroPedido;
    int itemPedido = 0;
    //icms
    QString tipoICMS;
    int orig = 0;
    int cstICMS = 0;
    int modBC = 0;
    double vBC = 0;
    double pICMS = 0;
    double vICMS = 0;
    int modBCST = 0;
    double pMVAST = 0;
    double vBCST = 0;
    double pICMSST = 0;
    double vICMSST = 0;
    //ipi
    int cEnq = 0;
    int cstIPI = 0;
    //pis
    int cstPIS = 0;
    double vBCPIS = 0;
    double pPIS = 0;
    double vPIS = 0;
    //cofins
    int cstCOFINS = 0;
    double vBCCOFINS = 0;
    double pCOFINS = 0;
    double vCOFINS = 0;
    //total
    double vBC_Total = 0;
    double vICMS_Total = 0;
    double vICMSDeson_Total = 0;
    double vBCST_Total = 0;
    double vST_Total = 0;
    double vProd_Total = 0;
    double vFrete_Total = 0;
    double vSeg_Total = 0;
    double vDesc_Total = 0;
    double vII_Total = 0;
    double vIPI_Total = 0;
    double vPIS_Total = 0;
    double vCOFINS_Total = 0;
    double vOutro_Total = 0;
    double vNF_Total = 0;

    // methods
    void darkTheme();
    void readFile();
    void readChild(QDomElement element, QStandardItem *elementItem);
    void readTree(QStandardItem *item);
    void saveXML();
    void lerTotais(QStandardItem *child);
    void lerPISProduto(QStandardItem *child);
    void lerIPIProduto(QStandardItem *child);
    void lerICMSProduto(QStandardItem *child);
    void lerDadosProduto(QStandardItem *child);
    void lerCOFINSProduto(QStandardItem *child);
    bool insertEstoque();
    bool insertProdutoEstoque();
};

#endif // XML_H
