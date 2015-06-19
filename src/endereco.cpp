#include <QSqlQuery>
#include <QVariant>
#include <QDebug>

#include "endereco.h"

Endereco::Endereco(int idEndereco, QString table) : m_idEndereco(-1) {
  QSqlQuery query;
  query.prepare("SELECT * FROM :table WHERE idEndereco = :idEndereco");
  query.bindValue(":table", table);
  query.bindValue(":idEndereco", idEndereco);

  if (not query.exec() or not query.first()) {
    return;
  }

  m_idEndereco = idEndereco;
  m_descricao = query.value("descricao").toString();
  m_cep = query.value("cep").toString();
  m_logradouro = query.value("logradouro").toString();
  m_numero = query.value("numero").toString();
  m_complemento = query.value("complemento").toString();
  m_bairro = query.value("bairro").toString();
  m_cidade = query.value("cidade").toString();
  m_uf = query.value("uf").toString();
}

QString Endereco::linhaUm() { return (m_logradouro + ", " + m_numero + " " + m_complemento + " - " + m_bairro); }

QString Endereco::linhaDois() { return (m_cidade + " - " + m_uf + " - CEP: " + m_cep); }

QString Endereco::umaLinha() { return (linhaUm() + " - " + m_cidade + " - " + m_uf); }

QString Endereco::uf() const { return m_uf; }

QString Endereco::complemento() const { return m_complemento; }

QString Endereco::numero() const { return m_numero; }

QString Endereco::cep() const { return m_cep; }

QString Endereco::descricao() const { return m_descricao; }

QString Endereco::cidade() const { return m_cidade; }

QString Endereco::bairro() const { return m_bairro; }

QString Endereco::logradouro() const { return m_logradouro; }

int Endereco::idEndereco() const { return m_idEndereco; }
