#ifndef ENDERECO_HPP
#define ENDERECO_HPP

#include <QString>

class Endereco {
  public:
    explicit Endereco(int idEndereco);
    int idEndereco() const;
    QString logradouro() const;
    QString bairro() const;
    QString cidade() const;
    QString descricao() const;
    void setDescricao(const QString &descricao);
    QString cep() const;
    void setCep(const QString &cep);
    QString numero() const;
    void setNumero(const QString &numero);
    QString complemento() const;
    void setComplemento(const QString &complemento);
    QString uf() const;
    void setUf(const QString &uf);
    bool ativo() const;
    void setAtivo(bool ativo);
    QString umaLinha();
    QString linhaUm();
    QString linhaDois();

  private:
    QString m_descricao;
    QString m_cep;
    QString m_logradouro;
    QString m_numero;
    QString m_complemento;
    QString m_bairro;
    QString m_cidade;
    QString m_uf;
    bool m_ativo;
    int m_idEndereco;
};

#endif // ENDERECO_HPP
