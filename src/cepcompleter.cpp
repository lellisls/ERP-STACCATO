#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "cepcompleter.h"

bool CepCompleter::buscaCEP(const QString &cep) {
  QSqlQuery query;
  query.prepare("SELECT log_logradouro.log_tipo_logradouro, log_logradouro.log_no AS logradouro, log_bairro.bai_no AS "
                "bairro, log_localidade.loc_no AS cidade, log_localidade.ufe_sg AS uf, log_logradouro.cep FROM "
                "cep.`log_logradouro`, cep.`log_localidade`, cep.`log_bairro` WHERE log_logradouro.loc_nu_sequencial = "
                "log_localidade.loc_nu_sequencial AND log_logradouro.bai_nu_sequencial_ini = "
                "log_bairro.bai_nu_sequencial AND log_logradouro.cep = :cep");
  query.bindValue(":cep", QString(cep).remove("-"));

  if (not query.exec()) {
    QMessageBox::critical(nullptr, "Erro!", "Erro na busca pelo cep: " + query.lastError().text());
    return false;
  }

  if (not query.first()) return false;

  cidade = query.value("cidade").toString();
  endereco = query.value("log_tipo_logradouro").toString() + " " + query.value("logradouro").toString();
  bairro = query.value("bairro").toString();
  uf = query.value("uf").toString();

  return true;
}

QString CepCompleter::getUf() const { return uf; }

QString CepCompleter::getBairro() const { return bairro; }

QString CepCompleter::getEndereco() const { return endereco; }

QString CepCompleter::getCidade() const { return cidade; }
