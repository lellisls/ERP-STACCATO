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
    void prepararNFe(const QList<int> &items);

  private slots:
    void updateImpostos();
    void on_pushButtonGerarNFE_clicked();
    void on_pushButtonCancelar_clicked();
    void on_tableItens_activated(const QModelIndex &);
    void on_tableItens_pressed(const QModelIndex &);

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
    QString clearStr(const QString &str) const;
    QString calculaDigitoVerificador(const QString &chave);
    bool writeTXT();
    void writeIdentificacao(QTextStream &stream) const;
    bool writeEmitente(QTextStream &stream);
    bool writeDestinatario(QTextStream &stream);
    bool writeProduto(QTextStream &stream, double &total, double &icmsTotal);
    void writeTotal(QTextStream &stream, double &total, double &icmsTotal, double &frete) const;
    QString removeDiacritics(const QString &str) const;
    void guardarNotaBD();
    void setSettings(const QString &key, const QVariant &value) const;
    QVariant settings(const QString &key) const;
};

#endif // CADASTRARNFE_H
