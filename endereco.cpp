#include <QSqlQuery>
#include <QVariant>
#include <QDebug>

#include "endereco.h"

Endereco::Endereco(int idEndereco, QString table) : m_idEndereco(-1) {
  QSqlQuery qry("SELECT * FROM " + table + " WHERE idEndereco = " + QString::number(idEndereco) + "");

  if (not qry.exec() or not qry.first()) {
    return;
  }

  m_idEndereco = idEndereco;
  m_descricao = qry.value("descricao").toString();
  m_cep = qry.value("cep").toString();
  m_logradouro = qry.value("logradouro").toString();
  m_numero = qry.value("numero").toString();
  m_complemento = qry.value("complemento").toString();
  m_bairro = qry.value("bairro").toString();
  m_cidade = qry.value("cidade").toString();
  m_uf = qry.value("uf").toString();
}

QString Endereco::linhaUm() {
  return (m_logradouro + ", " + m_numero + " " + m_complemento + " - " + m_bairro);
}

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
