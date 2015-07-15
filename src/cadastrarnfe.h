#ifndef CADASTRARNFE_H
#define CADASTRARNFE_H

#include "editablesqlmodel.h"

#include <QDialog>
#include <QDataWidgetMapper>
#include <QTextStream>

namespace Ui {
  class CadastrarNFe;
}

class CadastrarNFe : public QDialog {
    Q_OBJECT

  public:
    explicit CadastrarNFe(QString idVenda, QWidget *parent = 0);
    ~CadastrarNFe();
    void prepararNFe(const QList<int> items);
    void setItemData(const int row, const QString &key, const QVariant &value);
    QVariant getItemData(const int row, const QString &key) const;
    void guardarNotaBD();

  public slots:
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

  private slots:
    void updateImpostos();
    void on_pushButtonGerarNFE_clicked();
    void on_pushButtonCancelar_clicked();
    void on_tableItens_activated(const QModelIndex &index);
    void on_tableItens_pressed(const QModelIndex &index);

  private:
    // attributes
    Ui::CadastrarNFe *ui;
    EditableSqlModel modelNFe, modelNFeItem, modelLoja, modelVenda, modelProd;
    const QString idVenda;
    QString arquivo;
    QString chaveNum;
    QString chaveAcesso;
    QDataWidgetMapper mapperNFe;
    // methods
    QString criarChaveAcesso();
    QString clearStr(QString str);
    QVariant getFromItemModel(const int row, const QString column) const;
    QVariant getFromLoja(const QString column) const;
    QVariant getFromVenda(const QString column) const;
    QVariant getFromProdModel(const int row, const QString column) const;
    QString calculaDigitoVerificador(const QString chave);
    bool writeTXT();
    void writeIdentificacao(QTextStream &stream);
    bool writeEmitente(QTextStream &stream);
    bool writeDestinatario(QTextStream &stream);
    bool writeProduto(QTextStream &stream, double &total, double &icmsTotal);
    void writeTotal(QTextStream &stream, double &total, double &icmsTotal, double &frete);
    QString removeDiacritics(QString str);
};

#endif // CADASTRARNFE_H
