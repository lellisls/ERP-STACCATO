#ifndef NFE_H
#define NFE_H

#include <QObject>
#include <QSqlTableModel>

class NFe : public QObject {
    Q_OBJECT

  public:
    explicit NFe(QString idVenda, QObject *parent = 0);
    ~NFe();
    bool TXT();
    QString calculaDigitoVerificador(QString chave);
    QString getArquivo() const;
    QString getChaveAcesso() const;
    QString criarChaveAcesso();

  private:
    // attributes
    QSqlTableModel modelVenda, modelLoja, modelItem;
    QString idVenda;
    QString chaveAcesso;
    QString arquivo;
    // methods
    QVariant getFromVenda(QString column);
    QVariant getFromLoja(QString column);
    QVariant getFromItemModel(int row, QString column);
    void writeTXT(QString chave);
};

#endif // NFE_H
