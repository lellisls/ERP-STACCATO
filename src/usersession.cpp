#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QSqlError>
#include <QVariant>

#include "usersession.h"

QSqlQuery *UserSession::query = nullptr;

QSettings settings("Staccato", "ERP");

int UserSession::getLoja() { return (query->value("idLoja").toInt()); }

int UserSession::getIdUsuario() { return (query->value("idUsuario").toInt()); }

QString UserSession::getNome() { return (query->value("nome").toString()); }

bool UserSession::login(const QString &user, const QString &password) {
  initialize();

  query->prepare("SELECT * FROM usuario WHERE user = :user AND passwd = PASSWORD(:password) AND desativado = FALSE");
  query->bindValue(":user", user);
  query->bindValue(":password", password);

  if (not query->exec()) {
    QMessageBox::critical(0, "Erro!", "Erro no login: " + query->lastError().text());
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

QString UserSession::getFromLoja(const QString &parameter, const QString &user) {
  QSqlQuery queryLoja;
  queryLoja.prepare("SELECT " + parameter +
                    " FROM loja LEFT JOIN usuario ON loja.idLoja = usuario.idLoja WHERE usuario.nome = :nome");
  queryLoja.bindValue(":nome", user);

  if (not queryLoja.exec() or not queryLoja.first()) {
    QMessageBox::critical(0, "Erro!", "Erro na query loja: " + queryLoja.lastError().text());
    return QString();
  }

  return queryLoja.value(0).toString();
}

QVariant UserSession::getSettings(const QString &key) { return settings.value(key); }

void UserSession::setSettings(const QString &key, const QVariant &value) { settings.setValue(key, value); }

bool UserSession::settingsContains(const QString &key) { return settings.contains(key); }

QSqlQuery *UserSession::initialize() {
  if (query) {
    query->finish();
    delete query;
  }

  query = new QSqlQuery();

  return query;
}
