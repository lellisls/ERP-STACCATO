#ifndef CEPCOMPLETER_H
#define CEPCOMPLETER_H

class CepCompleter {

public:
  CepCompleter();
  ~CepCompleter();
  bool buscaCEP(const QString &cpf);
  QString getBairro() const;
  QString getCidade() const;
  QString getEndereco() const;
  QString getUf() const;

private:
  QString cidade;
  QString endereco;
  QString bairro;
  QString uf;
};

#endif // CEPCOMPLETER_H
