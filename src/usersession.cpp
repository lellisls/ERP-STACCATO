#include <QDebug>
#include <QSqlError>
#include <QVariant>

#include "usersession.h"

QSqlQuery *UserSession::query = nullptr;

int UserSession::getLoja() { return (query->value("idLoja").toInt()); }

int UserSession::getId() { return (query->value("idUsuario").toInt()); }

QString UserSession::getNome() { return (query->value("nome").toString()); }

bool UserSession::login(QString user, QString password) {
  initialize();

  query->prepare("SELECT * FROM Usuario WHERE user = :user AND passwd = PASSWORD(:password) AND desativado = false");
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
  QString str = "SELECT sigla FROM Loja WHERE idLoja = " + QString::number(getLoja());
  QSqlQuery queryLoja(str);

  if (not queryLoja.exec(str)) {
    qDebug() << __FILE__ << ": ERROR IN QUERY: " << query->lastError();
  }

  if (queryLoja.first()) {
    return (queryLoja.value("sigla").toString());
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
