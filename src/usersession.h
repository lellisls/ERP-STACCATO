#ifndef USERSESSION_H
#define USERSESSION_H

#include <QSettings>
#include <QSqlQuery>

class UserSession {

public:
  enum class Tipo { Padrao, Autorizacao };
  static bool login(const QString &user, const QString &password, Tipo tipo = Tipo::Padrao);
  static int idUsuario();
  static int idLoja();
  static QString nome();
  static QString tipoUsuario();
  static QString fromLoja(const QString &parameter, const QString &user = nome());
  static QVariant settings(const QString &key);
  static void setSettings(const QString &key, const QVariant &value);
  static bool settingsContains(const QString &key);

private:
  // attributes
  static QSqlQuery *query;
  // methods
  UserSession();
  static void free();
  static void initialize();
  static void logout();
};

#endif // USERSESSION_H
