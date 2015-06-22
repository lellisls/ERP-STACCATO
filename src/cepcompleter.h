#ifndef CEPCOMPLETER_H
#define CEPCOMPLETER_H

#include <QSqlQuery>
#include <QString>

class CepCompleter {

  public:
    CepCompleter();
    ~CepCompleter();
    bool buscaCEP(const QString cpf);
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
    QString buscaUF(const QString cep);
    bool inRange(const QString cep, const int st, const int end);
    void clearFields();
};

#endif // CEPCOMPLETER_H
