#include <QDebug>
#include <QSqlError>
#include <QVariant>

#include "usersession.h"

QSqlQuery *UserSession::query = nullptr;

int UserSession::getLoja() { return (query->value("idLoja").toInt()); }

int UserSession::getIdUsuario() { return (query->value("idUsuario").toInt()); }

QString UserSession::getNome() { return (query->value("nome").toString()); }

bool UserSession::login(const QString user, const QString password) {
  initialize();

  query->prepare("SELECT * FROM usuario WHERE user = :user AND passwd = PASSWORD(:password) AND desativado = FALSE");
  query->bindValue(":user", user);
  query->bindValue(":password", password);

  if (not query->exec()) {
    qDebug() << "Login query error: " << query->lastError();
  }

  return query->first();
}

void UserSession::logout() { query->clear(); }

void UserSession::free() {
  if (query) {
    delete query;
  }

  query = nullptr;
}

QString UserSession::getTipoUsuario() { return (query->value("tipo").toString()); }

QString UserSession::getSigla() { return (query->value("sigla").toString()); }

QString UserSession::getSiglaLoja() {
  QSqlQuery queryLoja;
  queryLoja.prepare("SELECT sigla FROM loja WHERE idLoja = :idLoja");
  queryLoja.bindValue(":idLoja", getLoja());

  if (not queryLoja.exec() or not queryLoja.first()) {
    qDebug() << __FILE__ << ": ERROR IN QUERY: " << query->lastError();
  } else {
    return queryLoja.value("sigla").toString();
  }

  return QString();
}

QString UserSession::getFromLoja(QString parameter) {
  QSqlQuery queryLoja;
  queryLoja.prepare("SELECT " + parameter + " FROM loja WHERE idLoja = :idLoja");
  queryLoja.bindValue(":idLoja", getLoja());

  if (not queryLoja.exec() or not queryLoja.first()) {
    qDebug() << __FILE__ << ": ERROR IN QUERY: " << query->lastError();
  } else {
    return queryLoja.value(parameter).toString();
  }

  return QString();
}

QStringList UserSession::getTodosCNPJ() {
  QSqlQuery queryLoja;
  QStringList list;

  if (not queryLoja.exec("SELECT cnpj FROM loja")) {
    qDebug() << __FILE__ << ": ERROR IN QUERY: " << query->lastError();
  }

  if (queryLoja.first()) {
    for (int i = 0; i < queryLoja.size(); ++i) {
      list.append(queryLoja.value("cnpj").toString().remove(".").remove("/").remove("-"));
      queryLoja.next();
    }
  } else{
    qDebug() << "Não encontrou lojas";
  }

  return list;
}

QSqlQuery *UserSession::initialize() {
  if (query) {
    query->finish();
    delete query;
  }

  query = new QSqlQuery();

  return query;
}
