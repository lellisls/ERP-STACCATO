#include "cepcompleter.h"
#include <QVariant>
#include <QDebug>
#include <QSqlError>
CepCompleter::CepCompleter() {}

CepCompleter::~CepCompleter() {}

void CepCompleter::clearFields() {
  uf.clear();
  cidade.clear();
  endereco.clear();
  bairro.clear();
}

bool CepCompleter::buscaCEP(QString cep) {
  clearFields();
  uf = buscaUF(cep).toUpper();

  if (uf.isEmpty()) {
    return false;
  }

  QSqlQuery query;

  if (not query.exec("SELECT * FROM cep." + uf.toLower() + " WHERE cep = '" +
                     cep + "' LIMIT 1")) {
    qDebug() << __FILE__ " : " << __LINE__
             << " : Erro ao buscar cep: " << query.lastError();
    qDebug() << "Query: " << query.lastQuery();
    return false;
  }

  if (not query.first()) {
    return false;
  }

  cidade = query.value("cidade").toString();
  endereco = query.value("tp_logradouro").toString() + " " +
             query.value("logradouro").toString();
  bairro = query.value("bairro").toString();

  return true;
}

QString CepCompleter::buscaUF(QString cep) {
  if (inRange(cep, 01000, 19999)) return "sp";
  if (inRange(cep, 69900, 69999)) return "ac";
  if (inRange(cep, 57000, 57999)) return "al";
  if (inRange(cep, 69000, 69299)) return "am";
  if (inRange(cep, 69400, 69899)) return "am";
  if (inRange(cep, 68900, 68999)) return "ap";
  if (inRange(cep, 40000, 48999)) return "ba";
  if (inRange(cep, 60000, 63999)) return "ce";
  if (inRange(cep, 70000, 72799)) return "df";
  if (inRange(cep, 73000, 73699)) return "df";
  if (inRange(cep, 29000, 29999)) return "es";
  if (inRange(cep, 72800, 72999)) return "go";
  if (inRange(cep, 73700, 76799)) return "go";
  if (inRange(cep, 65000, 65999)) return "ma";
  if (inRange(cep, 30000, 39999)) return "mg";
  if (inRange(cep, 79000, 79999)) return "ms";
  if (inRange(cep, 78000, 78899)) return "mt";
  if (inRange(cep, 66000, 68899)) return "pa";
  if (inRange(cep, 58000, 58999)) return "pb";
  if (inRange(cep, 50000, 56999)) return "pe";
  if (inRange(cep, 64000, 64999)) return "pi";
  if (inRange(cep, 80000, 87999)) return "pr";
  if (inRange(cep, 20000, 28999)) return "rj";
  if (inRange(cep, 59000, 59999)) return "rn";
  if (inRange(cep, 76800, 76999)) return "ro";
  if (inRange(cep, 69300, 69399)) return "rr";
  if (inRange(cep, 90000, 99999)) return "rs";
  if (inRange(cep, 88000, 89999)) return "sc";
  if (inRange(cep, 49000, 49999)) return "se";
  if (inRange(cep, 77000, 77999)) return "to";

  return QString();
}

bool CepCompleter::inRange(QString cep, int st, int end) {
  QStringList valueList = cep.split(QChar('-'));

  if (valueList.size() != 2) {
    return false;
  }

  int vl = valueList.at(0).toInt();

  return (vl >= st and vl <= end);
}

QString CepCompleter::getUf() const { return uf; }

QString CepCompleter::getBairro() const { return bairro; }

QString CepCompleter::getEndereco() const { return endereco; }

QString CepCompleter::getCidade() const { return cidade; }
