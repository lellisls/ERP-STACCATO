#ifndef CEPCOMPLETER_H
#define CEPCOMPLETER_H

#include <QSqlQuery>
#include <QString>

class CepCompleter {

  public:
    CepCompleter();
    ~CepCompleter();
    bool buscaCEP(QString cpf);
    QString getCidade() const;
    QString getEndereco() const;
    QString getBairro() const;
    QString getUf() const;

  private:
    // attributes
    QString cidade;
    QString endereco;
    QString bairro;
    QString uf;
    // methods
    QString buscaUF(QString cep);
    bool inRange(QString cep, int st, int end);
    void clearFields();
};

#endif // CEPCOMPLETER_H
