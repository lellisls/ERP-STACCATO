#include <QDebug>
#include <QSqlError>
#include <QVariant>

#include "usersession.h"

QSqlQuery *UserSession::query = nullptr;

int UserSession::getLoja() { return (query->value("idLoja").toInt()); }

int UserSession::getId() { return (query->value("idUsuario").toInt()); }

QString UserSession::getNome() { return (query->value("nome").toString()); }

bool UserSession::login(const QString user, const QString password) {
  initialize();

  query->prepare("SELECT * FROM Usuario WHERE user = :user AND passwd = PASSWORD(:password) AND desativado = FALSE");
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
  queryLoja.prepare("SELECT sigla FROM Loja WHERE idLoja = :idLoja");
  queryLoja.bindValue(":idLoja", getLoja());

  if (not queryLoja.exec() or not queryLoja.first()) {
    qDebug() << __FILE__ << ": ERROR IN QUERY: " << query->lastError();
  } else {
    return queryLoja.value("sigla").toString();
  }

  return QString();
}

QSqlQuery *UserSession::initialize() {
  if (query) {
    query->finish();
    delete query;
  }

  query = new QSqlQuery();

  return query;
}
