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
  QString buscaUF(QString cep);
  bool inRange(QString cep, int st, int end);
  void clearFields();

  QString cidade;
  QString endereco;
  QString bairro;
  QString uf;

};

#endif // CEPCOMPLETER_H
