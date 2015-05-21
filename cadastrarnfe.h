#ifndef CADASTRARNFE_H
#define CADASTRARNFE_H

#include "editablesqlmodel.h"

#include <QDialog>
#include <QDataWidgetMapper>

namespace Ui {
  class CadastrarNFE;
}

class CadastrarNFE : public QDialog {
    Q_OBJECT

  public:
    explicit CadastrarNFE(QString idOrcamento, QWidget *parent = 0);
    ~CadastrarNFE();
    void gerarNFe(QList<int> items);
    void setItemData(int row, const QString &key, const QVariant &value);
    QVariant getItemData(int row, const QString &key);

  public slots:
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

  private slots:
    void updateImpostos();
    void on_pushButtonGerarNFE_clicked();
    void on_pushButtonCancelar_clicked();
    void on_tableView_activated(const QModelIndex &index);
    void on_tableView_pressed(const QModelIndex &index);

  private:
    // attributes
    Ui::CadastrarNFE *ui;
    EditableSqlModel modelNFe, modelItem, modelLoja, modelVenda, modelProd;
    QString idOrcamento;
    QString arquivo;
    QString chaveAcesso;
    QDataWidgetMapper mapper;
    // methods
    void writeTXT();
    bool generateNFE();
    QString criarChaveAcesso();
    QString clearStr(QString str);
    QVariant getFromItemModel(int row, QString column);
    QVariant getFromLoja(QString column);
    QVariant getFromVenda(QString column);
    QString calculaDigitoVerificador(QString chave);
    bool writeTXT(QString chave);
    QVariant getFromProdModel(int row, QString column);
};

#endif // CADASTRARNFE_H
