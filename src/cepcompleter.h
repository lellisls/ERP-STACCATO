#ifndef CEPCOMPLETER_H
#define CEPCOMPLETER_H

class CepCompleter {

  public:
    CepCompleter();
    ~CepCompleter();
    bool buscaCEP(const QString &cpf);
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
};

#endif // CEPCOMPLETER_H
