#ifndef CADASTRARNFE_H
#define CADASTRARNFE_H

#include <QDialog>
#include <QTextStream>

#include "sqltablemodel.h"

namespace Ui {
  class CadastrarNFe;
}

class CadastrarNFe : public QDialog {
    Q_OBJECT

  public:
    explicit CadastrarNFe(QString idVenda, QWidget *parent = 0);
    ~CadastrarNFe();
    void prepararNFe(const QList<int> items);

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
    SqlTableModel modelLoja;
    SqlTableModel modelVenda;
    SqlTableModel modelProd;
    const QString idVenda;
    QString arquivo;
    QString chaveNum;
    QString chaveAcesso;
    // methods
    QString criarChaveAcesso();
    QString clearStr(QString str) const;
    QString calculaDigitoVerificador(const QString chave) const;
    bool writeTXT();
    void writeIdentificacao(QTextStream &stream) const;
    bool writeEmitente(QTextStream &stream) const;
    bool writeDestinatario(QTextStream &stream) const;
    bool writeProduto(QTextStream &stream, double &total, double &icmsTotal) const;
    void writeTotal(QTextStream &stream, double &total, double &icmsTotal, double &frete) const;
    QString removeDiacritics(QString str) const;
    void guardarNotaBD();
};

#endif // CADASTRARNFE_H
