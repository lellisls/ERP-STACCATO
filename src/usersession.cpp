#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QSqlError>
#include <QVariant>

#include "usersession.h"

QSqlQuery *UserSession::query = nullptr;

QSettings m_settings("Staccato", "ERP");

int UserSession::idLoja() { return (query->value("idLoja").toInt()); }

int UserSession::idUsuario() { return (query->value("idUsuario").toInt()); }

QString UserSession::nome() { return (query->value("nome").toString()); }

bool UserSession::login(const QString &user, const QString &password, Tipo tipo) {
  if (tipo == Tipo::Autorizacao) {
    QSqlQuery query;
    query.prepare("SELECT idLoja, idUsuario, nome, tipo FROM usuario WHERE user = :user AND passwd = PASSWORD(:password) AND desativado = FALSE AND tipo LIKE '%GERENTE%'");
    query.bindValue(":user", user);
    query.bindValue(":password", password);

    if (not query.exec()) {
      QMessageBox::critical(nullptr, "Erro!", "Erro no login: " + query.lastError().text());
      return false;
    }

    return query.first();
  }

  initialize();

  query->prepare("SELECT idLoja, idUsuario, nome, tipo FROM usuario WHERE user = :user AND passwd = PASSWORD(:password) AND desativado = FALSE");
  query->bindValue(":user", user);
  query->bindValue(":password", password);

  if (not query->exec()) {
    QMessageBox::critical(nullptr, "Erro!", "Erro no login: " + query->lastError().text());
    return false;
  }

  return query->first();
}

void UserSession::logout() { query->clear(); }

void UserSession::free() {
  delete query;

  query = nullptr;
}

QString UserSession::tipoUsuario() { return (query->value("tipo").toString()); }

QString UserSession::fromLoja(const QString &parameter, const QString &user) {
  QSqlQuery queryLoja;
  queryLoja.prepare("SELECT " + parameter + " FROM loja LEFT JOIN usuario ON loja.idLoja = usuario.idLoja WHERE usuario.nome = :nome");
  queryLoja.bindValue(":nome", user);

  if (not queryLoja.exec() or not queryLoja.first()) {
    QMessageBox::critical(nullptr, "Erro!", "Erro na query loja: " + queryLoja.lastError().text());
    return QString();
  }

  return queryLoja.value(0).toString();
}

QVariant UserSession::settings(const QString &key) { return m_settings.value(key); }

void UserSession::setSettings(const QString &key, const QVariant &value) { m_settings.setValue(key, value); }

bool UserSession::settingsContains(const QString &key) { return m_settings.contains(key); }

void UserSession::initialize() {
  if (query) {
    query->finish();
    delete query;
  }

  query = new QSqlQuery();
}
